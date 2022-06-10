#pragma once

#include "PluginProcessor.h"
#include "MyLookAndFeel.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:

    enum Dimen_t {
        paramControlWidth = 250,
        paramControlHeight = 50,
        paramLabelWidth = 50
    };

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:

    MyLookAndFeel mMyLookAndFeel;
    juce::AudioProcessorValueTreeState& mValueTreeState;

    std::unique_ptr<SliderAttachment> mDelayAttachment;
    juce::Slider mDelaySlider;
    juce::Label mDelayLabel;

    std::unique_ptr<SliderAttachment> mGainAttachment;
    juce::Slider mGainSlider;
    juce::Label mGainLabel;

    juce::ComboBox mTypeSelector;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};