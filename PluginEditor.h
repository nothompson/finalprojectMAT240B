#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    Image background;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    Rectangle<int> bounds;
    // HDLSpace space;
    // juce::Slider shiftDial;

    myDial shiftDial;

    extraDial myPeakThreshold;
    extraDial myPhaseReset;
    extraDial myPhaseLerp;
    extraDial mySpectrumBlend;
    
    // juce::Slider frequencySlider;
    // juce::Slider distortionSlider;
    // juce::Slider rateSlider;
    // juce::TextButton openButton;
    
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachment;
    // std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
