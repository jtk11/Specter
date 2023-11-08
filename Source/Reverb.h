/*
  ==============================================================================

    Reverb.h
    Created: 8 Nov 2023 11:25:58am
    Author:  MacBook Pro

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ReverbEffect
{
public:
    ReverbEffect() {}
    ~ReverbEffect() {}

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        reverb.prepare(spec);
    }

    void reset()
    {
        reverb.reset();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        reverb.process(context);
    }

    void updateParameters(float roomSize, float damping, float wetLevel, float dryLevel, float width, float freezeMode)
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = roomSize;
        params.damping = damping;
        params.wetLevel = wetLevel;
        params.dryLevel = dryLevel;
        params.width = width;
        params.freezeMode = freezeMode;
        reverb.setParameters(params);
    }

private:
    juce::dsp::Reverb reverb;
};
