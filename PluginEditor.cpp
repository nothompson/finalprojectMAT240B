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
        processorRef.apvts, "gain", gainSlider));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "frequency", frequencySlider));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "distortion", distortionSlider));
    attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "rate", rateSlider));

    openButton.setButtonText("select audio file");
    openButton.onClick = [this](){
        chooser = std::make_unique<juce::FileChooser>(
            "select audio file",
            juce::File::getSpecialLocation(File::userHomeDirectory), "*.wav");

            chooser ->launchAsync(
                FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& c){
                    File file(c.getResult());

                    if (!file.exists()) return;

                    juce::AudioFormatManager formatManager;
                    formatManager.registerBasicFormats();
                    std::unique_ptr<juce::AudioFormatReader> reader(
                        formatManager.createReaderFor(file));
                    if (reader == nullptr) return;

                auto buffer = std::make_unique<juce::AudioBuffer<float>>(
                    static_cast<int>(reader->numChannels),
                    static_cast<int>(reader->lengthInSamples));

                reader->read(buffer.get(), 0,
                                static_cast<int>(reader->lengthInSamples), 0, true, true);

                // processorRef.setBuffer(std::move(buffer));

                });
    };


    addAndMakeVisible(gainSlider);
    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(distortionSlider);
    addAndMakeVisible(rateSlider);
    addAndMakeVisible(openButton);



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
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();

    auto height = 35;
    openButton.setBounds(area.removeFromTop(height));
    gainSlider.setBounds(area.removeFromTop(height));
    frequencySlider.setBounds(area.removeFromTop(height));
    rateSlider.setBounds(area.removeFromTop(height));
    distortionSlider.setBounds(area.removeFromTop(height));

}
