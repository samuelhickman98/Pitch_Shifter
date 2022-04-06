/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShifterAudioProcessor::PitchShifterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mTranspoOne = 0.0f;
    mTranspoTwo = 0.0f;
    mWindowSizeMs = 50.0f;
    mPresetFlag = 1;
}

PitchShifterAudioProcessor::~PitchShifterAudioProcessor()
{
}

//==============================================================================
void PitchShifterAudioProcessor::setPhasorType(atec::LFO::LfoType t)
{
    mPhasorOne[0].setType(t);
    mPhasorOne[1].setType(t);
    mPhasorTwo[0].setType(t);
    mPhasorTwo[1].setType(t);
}

void PitchShifterAudioProcessor::setPhasorFreqOne(double f)
{
    mPhasorOne[0].setFreq(f);
    mPhasorOne[1].setFreq(f);
}

void PitchShifterAudioProcessor::setPhasorFreqTwo(double f)
{
    mPhasorTwo[0].setFreq(f);
    mPhasorTwo[1].setFreq(f);
}

void PitchShifterAudioProcessor::setPhasorDebug(bool d)
{
    mPhasorOne[0].debug(d);
    mPhasorOne[1].debug(d);
    mPhasorTwo[0].debug(d);
    mPhasorTwo[1].debug(d);

}

void PitchShifterAudioProcessor::initPhasor()
{
    mPhasorOne[0].init();
    mPhasorOne[1].init();
    mPhasorTwo[0].init();
    mPhasorTwo[1].init();
}


const juce::String PitchShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchShifterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PitchShifterAudioProcessor::getProgramName (int index)
{
    return {};
}

void PitchShifterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    double phasorFreqOne, phasorFreqTwo;
    
    mNumInputChannels = getTotalNumInputChannels();
    mBlockSize = samplesPerBlock;
    mSampleRate = sampleRate;
    
    //initialize mWindowSizeSamps now that we know the sample rate
    mWindowSizeSamps = atec::Utilities::sec2samp(mWindowSizeMs/1000.0f, mSampleRate);
    
    mRingBuf.debug(false);
    mRingBuf.setSize(mNumInputChannels, 1.0 * mSampleRate, mBlockSize);
    mRingBuf.init();
    
    setPhasorDebug(false);
    setPhasorType(atec::LFO::saw);
    phasorFreqOne = atec::Utilities::transpo2freq(mTranspoOne, mWindowSizeMs);
    phasorFreqTwo = atec::Utilities::transpo2freq(mTranspoTwo, mWindowSizeMs);
    setPhasorFreqOne(phasorFreqOne);
    setPhasorFreqTwo(phasorFreqTwo);

    initPhasor();
}

void PitchShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PitchShifterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto bufSize = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //use a range-based for loop to look at the incoming MIDI messages
    for (const auto metadata : midiMessages)
    {
        double phasorFreqOne, phasorFreqTwo;
        auto message = metadata.getMessage();
        
        if(message.isNoteOn())
        {
            mTranspoOne = message.getNoteNumber() - 60.0f;
            mTranspoTwo = message.getNoteNumber() - 60.0f;
            phasorFreqOne = atec::Utilities::transpo2freq(mTranspoOne, mWindowSizeMs);
            phasorFreqTwo = atec::Utilities::transpo2freq(mTranspoTwo, mWindowSizeMs);
            setPhasorFreqOne(phasorFreqOne);
            setPhasorFreqTwo(phasorFreqTwo);
        }
    }
    
    mRingBuf.write(buffer);
    
    buffer.clear();

    // pull a block of delayed interpolated audio from the RingBuffer
    // we'll read at two different positions A & B, and crossfade the results
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        for (int i = 0; i < bufSize; i++)
        {
            double phasorSampleA, phasorSampleB;
            double envSignalA, envSignalB;
            double delayTimeSignalA, delayTimeSignalB;
            double sampleA, sampleB;
            
            //Voice 1
            phasorSampleA = mPhasorOne[channel].getNextSample();
            phasorSampleB = std::fmod(phasorSampleA + 0.5, 1.0f);
            
            envSignalA = std::sin(phasorSampleA * juce::MathConstants<double>::pi);
            delayTimeSignalA = phasorSampleA * mWindowSizeSamps;
            
            envSignalB = std::sin(phasorSampleB * juce::MathConstants<double>::pi);
            delayTimeSignalB = phasorSampleB * mWindowSizeSamps;
            
            sampleA = mRingBuf.readInterpSample(channel, i - mWindowSizeSamps, delayTimeSignalA);
            sampleA *= envSignalA;
            
            sampleB = mRingBuf.readInterpSample(channel, i - mWindowSizeSamps, delayTimeSignalB);
            sampleB *= envSignalB;
            
            channelData[i] = sampleA + sampleB;
            
            //Voice 2
            phasorSampleA = mPhasorTwo[channel].getNextSample();
            phasorSampleB = std::fmod(phasorSampleA + 0.5, 1.0f);
            
            envSignalA = std::sin(phasorSampleA * juce::MathConstants<double>::pi);
            delayTimeSignalA = phasorSampleA * mWindowSizeSamps;
            
            envSignalB = std::sin(phasorSampleB * juce::MathConstants<double>::pi);
            delayTimeSignalB = phasorSampleB * mWindowSizeSamps;
            
            sampleA = mRingBuf.readInterpSample(channel, i - mWindowSizeSamps, delayTimeSignalA);
            sampleA *= envSignalA;
            
            sampleB = mRingBuf.readInterpSample(channel, i - mWindowSizeSamps, delayTimeSignalB);
            sampleB *= envSignalB;
            
            channelData[i] += sampleA + sampleB;
        }
        
    }
    
    buffer.applyGain(juce::Decibels::decibelsToGain(-3.0f));
    
}

//==============================================================================
bool PitchShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchShifterAudioProcessor::createEditor()
{
    return new PitchShifterAudioProcessorEditor (*this);
}

//==============================================================================
void PitchShifterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PitchShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchShifterAudioProcessor();
}
