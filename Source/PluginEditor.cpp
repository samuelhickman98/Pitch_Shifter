/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShifterAudioProcessorEditor::PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (700, 500);
    
    mTranspoOne.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mTranspoOne.setRange(-12.0, 12.0, 0.1);
    mTranspoOne.setValue(audioProcessor.mTranspoOne);
    addAndMakeVisible(&mTranspoOne);
    mTranspoOne.addListener(this);
    
    mTranspoTwo.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mTranspoTwo.setRange(-12.0, 12.0, 0.1);
    mTranspoTwo.setValue(audioProcessor.mTranspoTwo);
    addAndMakeVisible(&mTranspoTwo);
    mTranspoTwo.addListener(this);
    
    mWindowSizeMs.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mWindowSizeMs.setRange(5.0, 300.0, 0.1);
    mWindowSizeMs.setValue(audioProcessor.mWindowSizeMs);
    addAndMakeVisible(&mWindowSizeMs);
    mWindowSizeMs.addListener(this);
    
    mPreset.addItem("Perfect Fifth", 1);
    mPreset.addItem("Weird", 2);
    mPreset.addItem("Scary", 3);
    mPreset.setSelectedId(audioProcessor.mPresetFlag);
    addAndMakeVisible(&mPreset);
    mPreset.addListener(this);
    
    addAndMakeVisible(&mTranspoTwoLabel);
    mTranspoOneLabel.setText("Transposition Voice 1", juce::dontSendNotification);
    mTranspoOneLabel.attachToComponent(&mTranspoOne, true);
    mTranspoOneLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mTranspoOneLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mTranspoOneLabel);
    mTranspoTwoLabel.setText("Transposition Voice 2", juce::dontSendNotification);
    mTranspoTwoLabel.attachToComponent(&mTranspoTwo, true);
    mTranspoTwoLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mTranspoTwoLabel.setJustificationType(juce::Justification::right);
    
    addAndMakeVisible(&mWindowSizeLabel);
    mWindowSizeLabel.setText("Window Size", juce::dontSendNotification);
    mWindowSizeLabel.attachToComponent(&mWindowSizeMs, true);
    mWindowSizeLabel.setColour(juce::Label::textColourId, juce::Colours::magenta);
    mWindowSizeLabel.setJustificationType(juce::Justification::right);
}

PitchShifterAudioProcessorEditor::~PitchShifterAudioProcessorEditor()
{
    mTranspoOne.removeListener(this);
    mTranspoTwo.removeListener(this);
    mWindowSizeMs.removeListener(this);
    mPreset.removeListener(this);
}

//==============================================================================
void PitchShifterAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    double phasorFreqOne, phasorFreqTwo, windowSizeSec;
    
    audioProcessor.mWindowSizeMs = mWindowSizeMs.getValue();
    windowSizeSec = audioProcessor.mWindowSizeMs/(double)1000.0f;
    audioProcessor.mWindowSizeSamps = atec::Utilities::sec2samp(windowSizeSec, audioProcessor.mSampleRate);
    audioProcessor.mTranspoOne = mTranspoOne.getValue();
    audioProcessor.mTranspoTwo = mTranspoTwo.getValue();
    
    phasorFreqOne = atec::Utilities::transpo2freq(audioProcessor.mTranspoOne, audioProcessor.mWindowSizeMs);
    phasorFreqTwo = atec::Utilities::transpo2freq(audioProcessor.mTranspoTwo, audioProcessor.mWindowSizeMs);
    audioProcessor.setPhasorFreqOne(phasorFreqOne);
    audioProcessor.setPhasorFreqTwo(phasorFreqTwo);
    
    DBG("Window Ms: " + juce::String(audioProcessor.mWindowSizeMs));
    DBG("Window samples: " + juce::String(audioProcessor.mWindowSizeSamps));
    DBG("TranspoOne: " + juce::String(audioProcessor.mTranspoOne));
    DBG("TranspoTwo: " + juce::String(audioProcessor.mTranspoTwo));
    DBG("FreqOne" + juce::String(phasorFreqOne));
    DBG("FreqTwo" + juce::String(phasorFreqTwo));
}

void PitchShifterAudioProcessorEditor::comboBoxChanged(juce::ComboBox *comboBox)
{
    audioProcessor.mPresetFlag = mPreset.getSelectedId();
    
    switch (mPreset.getSelectedId())
    {
        case 1:
            mTranspoOne.setValue(0.0f);
            mTranspoTwo.setValue(7.5f);
            //mTranspo.setValue(4.0f);
            break;
            
        case 2:
            mTranspoOne.setValue(-12.0f);
            mTranspoTwo.setValue(12.0f);
            break;
            
        case 3:
            mTranspoOne.setValue(0.0f);
            mTranspoTwo.setValue(-6.5f);
            
        default:
            break;
    }
}

void PitchShifterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (30.0f);
    g.drawFittedText ("Pitch Shifter", getLocalBounds(), juce::Justification::centred, 1);
}

void PitchShifterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    mTranspoOne.setBounds(200, 50, 300, 50);
    
    mTranspoTwo.setBounds(200, 100, 300, 50);
    
    mWindowSizeMs.setBounds(200, 400, 300, 50);
    
    mPreset.setBounds(350, 325, 75, 50);
    
}
