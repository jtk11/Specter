/*
  ==============================================================================

    LadderFilter.h
    Created: 8 Nov 2023 11:25:58am
    Author:  MacBook Pro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LadderFilterEffect
{
public:
    LadderFilterEffect() {}
    ~LadderFilterEffect() {}

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        ladderFilter.prepare(spec);
    }

    void reset()
    {
        ladderFilter.reset();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        ladderFilter.process(context);
    }

    void updateParameters(float cutoffFrequency, float resonance, float drive, juce::dsp::LadderFilter<float>::Mode mode)
    {
        ladderFilter.setCutoffFrequencyHz(cutoffFrequency);
        ladderFilter.setResonance(resonance);
        ladderFilter.setDrive(drive);
        ladderFilter.setMode(mode);
    }

private:
    juce::dsp::LadderFilter<float> ladderFilter;
};

