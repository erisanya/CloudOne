#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CloudOneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit CloudOneAudioProcessorEditor (CloudOneAudioProcessor&);
    ~CloudOneAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    CloudOneAudioProcessor& audioProcessor;

    juce::Slider brightnessKnob;
    juce::Label title;
    juce::Label subtitle;
    juce::Label mixCaption;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttachment;

    // Small "alive" LED that pulses with the plugin's own output level -
    // same mechanism/look as HYPER SCAPE and Chocola, just recoloured.
    juce::Point<float> ledCentre;
    float ledLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CloudOneAudioProcessorEditor)
};
