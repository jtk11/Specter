/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Reverb.h"

//==============================================================================
SpecterAudioProcessor::SpecterAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                      .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                      ),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      isLooping(true)
{
    // Constructor body remains empty as parameters are set up in the initializer list
}
SpecterAudioProcessor::~SpecterAudioProcessor() {
    
}


//==============================================================================
const juce::String SpecterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpecterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpecterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpecterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpecterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpecterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpecterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpecterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpecterAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpecterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================


#ifndef JucePlugin_PreferredChannelConfigurations
bool SpecterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//===================

void SpecterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Existing code...
    samplesPerBlockExpected = samplesPerBlock;
    this->currentSampleRate = sampleRate;
    // ...

    // Prepare transport sources with the current sample rate
    for (int i = 0; i < 4; ++i) {
        transportSource[i].prepareToPlay(samplesPerBlockExpected, currentSampleRate);
    }
    
    for (int i = 0; i < 4; ++i) {
        isTransportSourcePrepared[i] = true;
    }

    // Prepare the reverb effect
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = getTotalNumOutputChannels();

    reverbEffect.prepare(spec);
    reverbEffect.reset();
}

void SpecterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//==================

void SpecterAudioProcessor::loadFiles(const juce::Array<juce::File>& files)
{
    // Lock the critical section
    const juce::ScopedLock myScopedLock(lock);

    // Clear existing sources and transport sources to ensure we do not have dangling pointers
    for (auto& source : transportSource)
    {
        source.setSource(nullptr);  // Disconnect the source before resetting
        source.releaseResources();
    }

    for (auto& readerSrc : readerSource)
    {
        readerSrc.reset();  // Reset the reader source, which will delete the AudioFormatReader if it's the owner
    }

    formatManager.registerBasicFormats();

    for (int i = 0; i < 4; ++i)
    {
        if (i < files.size())
        {
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(files[i]));
            if (reader.get() != nullptr)
            {
                // Get the sample rate from the reader before releasing it
                auto fileSampleRate = reader->sampleRate;

                readerSource[i].reset(new juce::AudioFormatReaderSource(reader.release(), true));
                transportSource[i].setSource(readerSource[i].get(),
                                            0,                    // buffer size
                                            nullptr,              // use the default buffer
                                            fileSampleRate);      // use the file's sample rate
                transportSource[i]
                .prepareToPlay(samplesPerBlockExpected, currentSampleRate);
            }
        }
    }
}
//=================

void SpecterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    juce::ScopedLock myScopedLock(lock); // Lock the critical section
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();  // Store the result here

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);  // Use the stored result

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < numSamples; ++sample)  // And here
            channelData[sample] *= 0.5f;
    }
    juce::MidiMessage message;
        int samplePosition; // This will be used to know the position of the MIDI message in the buffer

        // Use a for loop to iterate over the midi messages in the buffer
         for (const auto metadata : midiMessages)
        {
            const auto& message = metadata.getMessage();
            if (message.isNoteOn())
            {
                int noteNumber = message.getNoteNumber();
                double speed = std::pow(2.0, (noteNumber - 60) / 12.0);

                for (int i = 0; i < 4; ++i)
                {
                    transportSource[i].setSource(readerSource[i].get(), 0, nullptr, readerSource[i]->getAudioFormatReader()->sampleRate * speed);
                    transportSource[i].setPosition(0.0);
        
                    // Assuming isTransportSourcePrepared[i] is an array of flags that indicates whether each transport source is ready
                    if (isTransportSourcePrepared[i])
                    {
                        transportSource[i].start();
                    }
                }
            }
            else if (message.isNoteOff())
            {
                // Optionally handle note off messages
            }
        }

        // Clear the MIDI buffer if you have processed the messages
        midiMessages.clear();
// Temporary buffer to hold audio data from each source
    juce::AudioBuffer<float> tempBuffer(totalNumOutputChannels, buffer.getNumSamples());
    // Retrieve the current mix levels (assumes these are stored in your processor)
    float mixLevels[4];
    getMixLevels(mixLevels[0], mixLevels[1], mixLevels[2], mixLevels[3]);

    // Mix the audio from each transport source into the output buffer
    for (int i = 0; i < 4; ++i)
    {
        if (transportSource[i].isPlaying())
        {
            tempBuffer.clear(); // Clear the temporary buffer
            juce::AudioSourceChannelInfo info(&tempBuffer, 0, buffer.getNumSamples());
            transportSource[i].getNextAudioBlock(info);
            
            if (isLooping && transportSource[i].hasStreamFinished()) {
                transportSource[i].setPosition(0); // Loop back to the start
                transportSource[i].start(); // Start playing again
            }

            // Add the contents of the temporary buffer to the main buffer
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                buffer.addFrom(channel, 0, tempBuffer, channel, 0, buffer.getNumSamples(), mixLevels[i]);
            }
        }
    }
}




//==============================================================================
bool SpecterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpecterAudioProcessor::createEditor()
{
    return new SpecterAudioProcessorEditor (*this);
}

//==============================================================================
void SpecterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpecterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpecterAudioProcessor();
}

//======
//Persistence:

void SpecterAudioProcessor::updateAudioFiles(const juce::Array<juce::File>& newFiles) {
    const juce::ScopedLock myScopedLock(lock); // Lock for thread safety if necessary
    audioFiles2 = newFiles;
    // Add any additional logic you need, such as updating the playback state.
}

//=========
//Looping:

void SpecterAudioProcessor::setLooping(bool shouldLoop)
{
    // Assuming transportSource is your audio playing source and is accessible here
    isLooping.store(shouldLoop); // Store the loop state in an atomic for thread safety
}

//================
//Ball movement:

void SpecterAudioProcessor::setMixLevels(float topLeft, float topRight, float bottomLeft, float bottomRight)
{
    // You might want to lock this operation if these values are accessed in your processBlock
    const juce::ScopedLock myScopedLock(lock);
    
    // Set the member variables that hold the mix levels
    this->topLeftLevel.store(topLeft);
    this->topRightLevel.store(topRight);
    this->bottomLeftLevel.store(bottomLeft);
    this->bottomRightLevel.store(bottomRight);

    // Now these levels will be used in the processBlock method
}

void SpecterAudioProcessor::getMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight) const
{
    // Retrieve the mix levels from where they are stored
    // This could be an atomic or some thread-safe mechanism
    topLeft = topLeftLevel.load();
    topRight = topRightLevel.load();
    bottomLeft = bottomLeftLevel.load();
    bottomRight = bottomRightLevel.load();
}
