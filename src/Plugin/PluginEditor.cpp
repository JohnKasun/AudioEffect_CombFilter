#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), processorRef(p), mValueTreeState(vts)
{
    juce::ignoreUnused(processorRef);

    
    addAndMakeVisible(mDelaySlider);
    mDelaySlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    mDelayAttachment.reset(new SliderAttachment(vts, "delay", mDelaySlider));
    addAndMakeVisible(mDelayLabel);
    mDelayLabel.setText("Delay", juce::dontSendNotification);
    mDelayLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(mGainSlider);
    mGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    mGainAttachment.reset(new SliderAttachment(vts, "gain", mGainSlider));
    addAndMakeVisible(mGainLabel);
    mGainLabel.setText("Gain", juce::dontSendNotification);
    mGainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(mTypeSelector);
    mTypeSelector.addItemList({
        "FIR",
        "IIR"
        }, 1);

    setSize(paramControlWidth + paramLabelWidth, paramControlHeight * 3);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto typeArea = area.removeFromTop(paramControlHeight);
    auto delayArea = area.removeFromTop(paramControlHeight);
    auto gainArea = area;

    mTypeSelector.setBounds(typeArea);

    mDelaySlider.setBounds(delayArea.removeFromRight(paramControlWidth));
    mDelayLabel.setBounds(delayArea);

    mGainSlider.setBounds(gainArea.removeFromRight(paramControlWidth));
    mGainLabel.setBounds(gainArea);
}