#pragma once

#include "PluginProcessor.h"
#include "MyLookAndFeel.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:

    enum Dimen_t {
        paramSliderWidth = 250,
        paramSliderHeight = 50,

        paramComboBoxWidth = 75,
        paramComboBoxHeight = 25,

        paramToggleButtonOffset = 30,

        paramLabelWidth = 50
    };

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

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

    std::unique_ptr<ComboBoxAttachment> mTypeAttachment;
    juce::ComboBox mTypeSelector;
    juce::Label mTypeLabel;

    std::unique_ptr<ButtonAttachment> mInvertGainAttachment;
    juce::ToggleButton mInvertGainButton;
    juce::Label mInvertGainLabel;

    AudioPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};