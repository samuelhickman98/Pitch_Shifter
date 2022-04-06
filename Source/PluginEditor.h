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
class PitchShifterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::ComboBox::Listener
{
public:
    PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor&);
    ~PitchShifterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PitchShifterAudioProcessor& audioProcessor;
    
    juce::Slider mTranspoOne;
    juce::Slider mTranspoTwo;
    juce::Slider mWindowSizeMs;
    juce::Label mTranspoOneLabel;
    juce::Label mTranspoTwoLabel;
    juce::Label mWindowSizeLabel;
    juce::ComboBox mPreset;
    juce::Label mPresetLabel;
    
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessorEditor)
};
