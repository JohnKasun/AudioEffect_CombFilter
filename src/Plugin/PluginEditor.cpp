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
    mDelayLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(mGainSlider);
    mGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    mGainAttachment.reset(new SliderAttachment(vts, "gain", mGainSlider));
    addAndMakeVisible(mGainLabel);
    mGainLabel.setText("Gain", juce::dontSendNotification);
    mGainLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(mTypeSelector);
    mTypeSelector.addItemList({
        "FIR",
        "IIR"
        }, 1);
    mTypeAttachment.reset(new ComboBoxAttachment(vts, "type", mTypeSelector));
    addAndMakeVisible(mTypeLabel);
    mTypeLabel.setText("Type", juce::dontSendNotification);
    mTypeLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(mInvertGainButton);
    mInvertGainAttachment.reset(new ButtonAttachment(vts, "invert gain", mInvertGainButton));
    addAndMakeVisible(mInvertGainLabel);
    mInvertGainLabel.setText("Invert Gain: ", juce::dontSendNotification);
    mInvertGainLabel.setJustificationType(juce::Justification::centredRight);

    setSize(paramSliderWidth + paramLabelWidth, paramSliderHeight * 2 + paramComboBoxHeight);
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
    auto typeArea = area.removeFromTop(paramComboBoxHeight);
    auto gainArea = area.removeFromTop(paramSliderHeight);
    auto delayArea = area;

    mTypeLabel.setBounds(typeArea.removeFromLeft(paramLabelWidth));
    mTypeSelector.setBounds(typeArea.removeFromLeft(paramComboBoxWidth));

    mInvertGainButton.setBounds(typeArea.removeFromRight(paramToggleButtonOffset));
    mInvertGainLabel.setBounds(typeArea);

    mDelaySlider.setBounds(delayArea.removeFromRight(paramSliderWidth));
    mDelayLabel.setBounds(delayArea);

    mGainSlider.setBounds(gainArea.removeFromRight(paramSliderWidth));
    mGainLabel.setBounds(gainArea);
}