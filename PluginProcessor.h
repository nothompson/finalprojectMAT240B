#pragma once

#include <JuceHeader.h>
#include "FFTProcessor.h"
#include "Library.h"
#include "LookAndFeel.h"
#include "extraLookAndFeel.h"
#include "myDial.h"
#include "extraDial.h"




//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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

    void setBuffer(std::unique_ptr<juce::AudioBuffer<float>> buffer);

    juce::AudioProcessorValueTreeState apvts;


    // juce::AudioProcessorParameter* getBypassParameter() const override;
    // juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout()};
    // juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    // two FFT processors for stereo 
    std::unique_ptr<ky::ClipPlayer> player;

    ky::Ramp ramp;

    FFTProcessor fft[2];

    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};

juce::AudioProcessorValueTreeState::ParameterLayout parameters();