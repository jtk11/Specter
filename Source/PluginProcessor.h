/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <atomic>
#include "Reverb.h"
#include "Filter.h"
#include "Oscillate.h"


//==============================================================================
/**
*/
class SpecterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SpecterAudioProcessor();
    virtual ~SpecterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void loadFiles(const juce::Array<juce::File>& files);
    void updateAudioFiles(const juce::Array<juce::File>& newFiles);
    juce::Array<juce::File> audioFiles2;
    const juce::Array<juce::File>& getAudioFiles() const { return audioFiles2; }
    void setLooping(bool shouldLoop);
    void setMixLevels(float topLeft, float topRight, float bottomLeft, float bottomRight);
    std::atomic<float> ballPosX{0.5f}; // Default x position (0.5 for center)
    std::atomic<float> ballPosY{0.5f};
    void getMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight) const;
    juce::AudioProcessorValueTreeState apvts;
    ReverbEffect reverbEffect; 
    LadderFilterEffect ladderFilterEffect;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { "reverbButton", 1 }, // Here "reverbButton" is the parameter ID, 1 is the version hint
            "Reverb On/Off",                         // human-readable name for the parameter
            false                                    // default value
        ));
        
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { "filterButton", 1 },
            "Filter On/Off",
            false
        ));
        
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { "oscillatorButton", 1 },
            "Oscillate On/Off",
            false
        ));


        return layout;
    }
    void randomizeReverbParameters();
    void randomizeLadderFilterParameters();
    //SampleOscillator sampleOscillator;
    
private:
    //==============================================================================
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource[4]; // Assuming you have 4 transport sources
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource[4];
    juce::CriticalSection lock;  // To protect the shared resources during audio processing
    int samplesPerBlockExpected;
    juce::Random random;
    double currentSampleRate;
    std::atomic<bool> isLooping;
    std::atomic<float> topLeftLevel{0.0f};
    std::atomic<float> topRightLevel{0.0f};
    std::atomic<float> bottomLeftLevel{0.0f};
    std::atomic<float> bottomRightLevel{0.0f};
    bool isTransportSourcePrepared[4] = {false, false, false, false};
    
    
   
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpecterAudioProcessor)
};
