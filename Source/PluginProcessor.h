/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum presetType
{
    nice = 1,
    weird,
    scary
};

//==============================================================================
/**
*/
class PitchShifterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PitchShifterAudioProcessor();
    ~PitchShifterAudioProcessor() override;

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
    
    int mNumInputChannels;
    double mSampleRate;
    double mBlockSize;
    
    double mWindowSizeSamps;
    double mWindowSizeMs;
    double mTranspoOne;
    double mTranspoTwo;
    int mPresetFlag;
    
    void setPhasorFreqOne(double f);
    void setPhasorFreqTwo(double f);
    
private:
    
    atec::RingBuffer mRingBuf;
    atec::LFO mPhasorOne[2];
    atec::LFO mPhasorTwo[2];
    void setPhasorDebug(bool d);
    void setPhasorType(atec::LFO::LfoType d);
    void initPhasor();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessor)
};
