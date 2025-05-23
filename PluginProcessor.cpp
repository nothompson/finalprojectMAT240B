#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

    parameter_list.push_back(std::make_unique<juce::AudioParameterInt>(
        ParameterID { "shift",  1 },
        "Shift",
        -12,
        12,
        0));
    
    parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParameterID { "peak",  1 },
            "Peak",
            0.0,
            1.0,
            0.1));

     parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParameterID { "reset",  1 },
            "Reset",
            0.0,
            1.0,
            0.2));

     parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParameterID { "lerp",  1 },
            "Lerp",
            0.0,
            1.0,
            0.5));

     parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
            ParameterID { "blend",  1 },
            "Blend",
            0.0,
            1.0,
            0.2));

    return { parameter_list.begin(), parameter_list.end() };
}
//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    apvts(*this, nullptr, "Parameters", parameters())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    ky::setPlaybackRate(static_cast<float>(getSampleRate()));

    setLatencySamples(fft[0].getLatencyInSamples());

    // reset fftprocessors internal state 

    fft[0].reset();
    fft[1].reset();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
    // return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i){
        buffer.clear(i, 0, numSamples);
    }

    float semitoneShift = *apvts.getRawParameterValue("shift");

    float pPeak = *apvts.getRawParameterValue("peak");
    float pReset = *apvts.getRawParameterValue("reset");
    float pLerp = *apvts.getRawParameterValue("lerp");
    float pBlend = *apvts.getRawParameterValue("blend");
    

    // send semitones to conversion function in fft processor
    fft[0].setSemitoneShift(semitoneShift);
    fft[1].setSemitoneShift(semitoneShift);

    fft[0].setParams(pPeak, pReset, pLerp, pBlend);
    fft[1].setParams(pPeak, pReset, pLerp, pBlend);

    // parameter to toggle on and off for testing
    // bool bypassed = apvts.getRawParameterValue("Bypass")->load();

    // float* channelL = buffer.getWritePointer(0);
    // float* channelR = buffer.getWritePointer(1);
    // float r = apvts.getParameter("distortion")->getValue();

    // ramp.frequency(r);
    // for (int i = 0; i < numSamples; ++i){

    //     float sample = player ? player -> operator()(ramp()): 0; 

    //     buffer.addSample(0, i, sample);
    //     buffer.addSample(1, i, sample);
    
    // }
    
    // float v = apvts.getParameter("rate")->getValue();

    juce::dsp::AudioBlock<float>block(buffer);


    // process block all at once. processBlock() still calls processSample()

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        fft[channel].processBlock(channelData, numSamples);
    }

    



}

void AudioPluginAudioProcessor::setBuffer(
    std::unique_ptr<juce::AudioBuffer<float>>buffer){
    juce::ignoreUnused(buffer);

    auto b = std::make_unique<ky::ClipPlayer>();
    for (int i = 0; i < buffer -> getNumSamples(); ++i){
        if(buffer -> getNumChannels() == 2){
            b -> addSample(buffer -> getSample(0,i) / 2 + buffer-> getSample(1,i)/2);
        } else {
            b ->addSample(buffer->getSample(0,i));
        }
    }
    player = std::move(b);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    // juce::ignoreUnused (destData);
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml,destData);

    // copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    // juce::ignoreUnused (data, sizeInBytes);
    // std::unique_ptr<juce::XmlElement>xml(getXmlFromBinary(data,sizeInBytes));
    // if(xml.get()!=nullptr && xml->hasTagName(apvts.state.getType())){
    //     apvts.replaceState(juce::ValueTree::fromXml(*xml));
    // }
    std::unique_ptr<juce::XmlElement>xmlState(
        getXmlFromBinary(data,sizeInBytes));

    if(xmlState.get()!=nullptr)
        if(xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));

}

// juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout(){
//     juce::AudioProcessorValueTreeState::ParameterLayout layout;

//     layout.add(std::make_unique<juce::AudioParameterBool>(
//         juce::ParameterID("Bypass",1),
//         "Bypass",
//         false));

//     return layout;
// }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
