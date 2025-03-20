#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "shift", shiftDial));

    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "peak", myPeakThreshold));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "reset", myPhaseReset));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "lerp", myPhaseLerp));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "blend", mySpectrumBlend));

    addAndMakeVisible(shiftDial);

    addAndMakeVisible(myPeakThreshold);
    addAndMakeVisible(myPhaseReset);
    addAndMakeVisible(myPhaseLerp);
    addAndMakeVisible(mySpectrumBlend);


    background = ImageCache::getFromMemory (BinaryData::background_png, BinaryData::background_pngSize);

    // addAndMakeVisible(openButton);

    // chooser = std::make_unique<juce::FileChooser>(
    //     "Select a file to open...",
    //     juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
    //     "*.wav;*.mp3;*.aif;*.flac");
    // openButton.setButtonText("select audio file");
    // openButton.onClick = [this] {
    //   auto fn = [this](const juce::FileChooser& c) {
    //     juce::File file = c.getResult();
    //     if (!file.exists()) return;
  
    //     juce::AudioFormatManager formatManager;
    //     formatManager.registerBasicFormats();
    //     std::unique_ptr<juce::AudioFormatReader> reader(
    //         formatManager.createReaderFor(file));
    //     if (reader == nullptr) return;
  
    //     auto buffer = std::make_unique<juce::AudioBuffer<float>>(
    //         static_cast<int>(reader->numChannels),
    //         static_cast<int>(reader->lengthInSamples));
  
    //     reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples),
    //                  0, true, true);
  
    //     // auto old = processorRef.buffer.exchange(buffer.release(),
    //     // std::memory_order_acq_rel); if (old) delete old;
    //     processorRef.setBuffer(std::move(buffer));
    //   };
  
    //   chooser->launchAsync(juce::FileBrowserComponent::canSelectFiles, fn);
    // };
  
    // addAndMakeVisible(openButton);
  }

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    // g.drawFittedText ("Hello World", getLocalBounds(), juce::Justification::centred, 1);
    g.drawImage (background, getLocalBounds ().toFloat ());
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();

    auto height = area.getHeight();
    auto width = area.getWidth();
    // openButton.setBounds(area.removeFromTop(height));
    shiftDial.setBounds((width / 4) + 35, (height / 4) - 30, 128, 150 +20);
    shiftDial.setRange(-12, 12, 1);
    shiftDial.setNumDecimalPlacesToDisplay(0);
    shiftDial.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    shiftDial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    // shiftDial.setTextBoxStyle(juce::Slider::textBoxOutlineColourId );
    shiftDial.setTextValueSuffix (" Semitones");
    // shiftDial.setSize(200, 200);
    // frequencySlider.setBounds(area.removeFromTop(height));
    // rateSlider.setBounds(area.removeFromTop(height));
    // distortionSlider.setBounds(area.removeFromTop(height));

    myPeakThreshold.setBounds((width / 4) - 45, (height / 4) - 50, 128 / 2, (150 / 2) + 20);
    myPeakThreshold.setRange(1.0, 0.1);
    myPeakThreshold.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    myPeakThreshold.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 150, 20);
    myPeakThreshold.setTextValueSuffix(" Threshold");
    
    mySpectrumBlend.setBounds((width / 4) - 45, (height / 4) + 100, 128 / 2, (150 / 2) + 20);
    mySpectrumBlend.setRange(1.0, 0.1);
    mySpectrumBlend.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mySpectrumBlend.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 150, 20);
    mySpectrumBlend.setTextValueSuffix(" Blend");

    myPhaseLerp.setBounds((width / 2) + 80, (height / 4) - 50, 128 / 2, (150 / 2) + 20);
    myPhaseLerp.setRange(1.0, 0.1);
    myPhaseLerp.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    myPhaseLerp.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 150, 20);
    myPhaseLerp.setTextValueSuffix(" Phase Lerp");
    
    myPhaseReset.setBounds((width / 2) + 80, (height / 4) + 100, 128 / 2, (150 / 2) + 20);
    myPhaseReset.setRange(1.0, 0.1);
    myPhaseReset.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    myPhaseReset.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 150, 20);
    myPhaseReset.setTextValueSuffix(" Phase Reset");

}

