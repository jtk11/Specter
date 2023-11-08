/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SpecterAudioProcessorEditor  : 
public juce::AudioProcessorEditor, public juce::Button::Listener, public juce::Slider::Listener, public juce::Timer
{
public:
    explicit SpecterAudioProcessorEditor (SpecterAudioProcessor&);
    ~SpecterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SpecterAudioProcessor& audioProcessor;
     juce::TextButton folderButton; // Button to select folder
     juce::TextButton diceButton;   // Button to randomly select files
     juce::Label label1, label2, label3, label4;
     juce::Array<juce::String> fileNames;
     juce::Array<juce::File> audioFiles;
     juce::Point<float> ballPosition;
     void shuffleAudioFiles();
     juce::ToggleButton loopButton;
     juce::TextButton rndMixButton;
     bool shouldMoveBall = false;
     int currentPointIndex = 0; // Declare a variable to keep track of the current point
     juce::Point<float> points[4]; // Declare an array to hold the points
     juce::Slider timerHzSlider;
     juce::TextButton stopButton;
     juce::TextButton filterButton;
     juce::TextButton reverbButton;
     // The attachment that binds the button to the APVTS
     //std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reverbButtonAttachment;

     juce::TextButton granularButton;
     juce::TextButton oscillatorButton;
     bool isDragging =false;
     void computeMixLevels(float& topLeft, float& topRight, float& bottomLeft, float& bottomRight);
     void mouseDown(const juce::MouseEvent& e) override;
     void mouseDrag(const juce::MouseEvent& e) override;
     void mouseUp(const juce::MouseEvent& e) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpecterAudioProcessorEditor)
};

