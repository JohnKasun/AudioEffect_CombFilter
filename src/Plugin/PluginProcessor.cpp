#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    mParameters(*this, nullptr, juce::Identifier("Parameters"),
        {
            std::make_unique<juce::AudioParameterFloat>("delay", "Delay", juce::NormalisableRange<float>(0.0f, 0.1f), 0.05f),
            std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("type", "Type", 1.0f, 2.0f, 1.0f),
            std::make_unique<juce::AudioParameterBool>("invert gain", "Invert Gain", false)
        })
{
    mDelayParam = mParameters.getRawParameterValue("delay");
    mGainParam = mParameters.getRawParameterValue("gain");
    mFilterType = mParameters.getRawParameterValue("type");
    mInvertGainParam = mParameters.getRawParameterValue("invert gain");


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

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (std::unique_ptr<CombFilter>& combFilter : mCombFilter) {
        combFilter.reset(new CombFilter(sampleRate));
    }
}

void AudioPluginAudioProcessor::releaseResources()
{
    for (std::unique_ptr<CombFilter>& combFilter : mCombFilter) {
        combFilter.reset();
    }
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    float phase = static_cast<bool>(*mInvertGainParam) ? -1.0f : 1.0f;
    for (std::unique_ptr<CombFilter>& combFilter : mCombFilter) {
        combFilter->setParam(CombFilter::Param_t::delayInSec, *mDelayParam);
        combFilter->setParam(CombFilter::Param_t::gain, phase * *mGainParam);
        combFilter->setFilterType(static_cast<CombFilter::FilterType_t>(*mFilterType - 1));
    }

    if (getNumOutputChannels() <= 0)
        buffer.clear();

    switch (getNumInputChannels()) {
    case 1:
        mCombFilter.at(0)->process(buffer.getReadPointer(0), buffer.getWritePointer(0), buffer.getNumSamples());
        for (int c = 1; c < getNumOutputChannels(); c++) {
            buffer.copyFrom(0, 0, buffer.getWritePointer(c), buffer.getNumSamples());
        }
        break;
    case 2:
        for (int c = 0; c < getNumOutputChannels(); c++) {
            mCombFilter.at(c)->process(buffer.getReadPointer(c), buffer.getWritePointer(c), buffer.getNumSamples());
        }
        break;
    default:
        buffer.clear();
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this, mParameters);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = mParameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(mParameters.state.getType())) {
            mParameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}