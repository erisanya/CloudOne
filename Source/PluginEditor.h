#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class AnalogKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;
};

class CloudOneAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CloudOneAudioProcessorEditor (CloudOneAudioProcessor&);
    ~CloudOneAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CloudOneAudioProcessor& processorRef;

    AnalogKnobLookAndFeel knobLookAndFeel;

    juce::Slider brightnessKnob;
    juce::Rectangle<int> titleBounds;
    juce::Rectangle<int> gainLabelBounds;
    juce::Image noiseTexture;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CloudOneAudioProcessorEditor)
};
