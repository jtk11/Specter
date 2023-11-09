/*
  ==============================================================================

    LowPassFilter.h
    Created: 8 Nov 2023 11:25:58am
    Author:  MacBook Pro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LowPassFilterEffect
{
public:
    LowPassFilterEffect() {}
    ~LowPassFilterEffect() {}

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        lowPassFilter.reset(new juce::dsp::IIR::Filter<float>());

        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, 20000.0f);
        lowPassFilter->coefficients = coefficients;

        lowPassFilter->prepare(spec);

        lastSampleRate = spec.sampleRate;
    }


    void reset()
    {
        lowPassFilter->reset();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            juce::dsp::AudioBlock<float> block(buffer);
            auto singleChannelBlock = block.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> context(singleChannelBlock);
            lowPassFilter->process(context);
        }
    }
    
    void updateParameters(float frequency, float qualityFactor)
    {
        // Update the low pass filter coefficients
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, frequency, qualityFactor);
        lowPassFilter->coefficients = *coefficients; // Assign the new coefficients
    }

private:
    std::unique_ptr<juce::dsp::IIR::Filter<float>> lowPassFilter;
    double lastSampleRate = 44100.0; // Default to standard CD sample rate
};

