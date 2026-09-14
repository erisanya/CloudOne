#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
class CloudOneLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CloudOneLookAndFeel()
    {
        setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff6fb0));
        setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3a0a22));
        setColour (juce::Slider::thumbColourId, juce::Colour (0xffffe6f2));
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffd98cb8));
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        juce::ignoreUnused (slider);

        // Leave room outside the dial body for tick marks + labels.
        auto fullBounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        auto bounds = fullBounds.reduced (fullBounds.getWidth() * 0.26f);
        const auto centre = bounds.getCentre();
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

        // Soft drop shadow.
        g.setColour (juce::Colour (0x40000000));
        g.fillEllipse (bounds.translated (0.0f, 5.0f));

        // Glossy dial body (light-from-top-left, tinted pink).
        juce::ColourGradient knobGradient (juce::Colour (0xffffe6f2), centre.x - radius * 0.5f, centre.y - radius * 0.6f,
                                            juce::Colour (0xff8a1054), centre.x + radius * 0.6f, centre.y + radius * 0.8f, true);
        knobGradient.addColour (0.55, juce::Colour (0xffe087bd));
        g.setGradientFill (knobGradient);
        g.fillEllipse (bounds);

        g.setColour (juce::Colour (0xff3a0a22));
        g.drawEllipse (bounds, 1.5f);

        // Tick marks around the dial, evenly spaced from 0 to +24 dB.
        constexpr int numTicks = 21;
        for (int i = 0; i < numTicks; ++i)
        {
            const float t = static_cast<float> (i) / static_cast<float> (numTicks - 1);
            const float tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            const bool isMajor = (i == 0 || i == numTicks / 2 || i == numTicks - 1);

            const float inner = radius + 5.0f;
            const float outer = radius + (isMajor ? 13.0f : 9.0f);

            juce::Point<float> p1 (centre.x + std::cos (tickAngle - juce::MathConstants<float>::halfPi) * inner,
                                   centre.y + std::sin (tickAngle - juce::MathConstants<float>::halfPi) * inner);
            juce::Point<float> p2 (centre.x + std::cos (tickAngle - juce::MathConstants<float>::halfPi) * outer,
                                   centre.y + std::sin (tickAngle - juce::MathConstants<float>::halfPi) * outer);

            g.setColour (isMajor ? juce::Colour (0xffffe6f2) : juce::Colour (0x99ffe6f2));
            g.drawLine (p1.x, p1.y, p2.x, p2.y, isMajor ? 1.8f : 1.2f);
        }

        // Progress arc showing the current amount.
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, radius - 4.0f, radius - 4.0f,
                           0.0f, rotaryStartAngle, angle, true);
        g.setColour (juce::Colour (0xffff4fa0));
        g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // Pointer (no centre dot - just the line).
        const float pointerLength = radius * 0.64f;
        juce::Point<float> p (centre.x + std::cos (angle - juce::MathConstants<float>::halfPi) * pointerLength,
                              centre.y + std::sin (angle - juce::MathConstants<float>::halfPi) * pointerLength);

        g.setColour (juce::Colour (0xff3a0a22));
        g.drawLine (centre.x, centre.y, p.x, p.y, 2.6f);

        // 0 / +12 / +24 labels (this knob is a real dB gain, not a %),
        // positioned around the dial like a gauge - same treatment/spot
        // as the 0%/50%/100% labels on the other two plugins.
        g.setColour (juce::Colour (0xffffe6f2));
        g.setFont (juce::Font (juce::FontOptions (12.0f)).withTypefaceStyle ("Bold"));

        auto drawLabelAt = [&] (float labelAngle, const juce::String& text)
        {
            const float labelRadius = radius + 22.0f;
            juce::Point<float> lp (centre.x + std::cos (labelAngle - juce::MathConstants<float>::halfPi) * labelRadius,
                                   centre.y + std::sin (labelAngle - juce::MathConstants<float>::halfPi) * labelRadius);
            juce::Rectangle<float> r (lp.x - 24.0f, lp.y - 9.0f, 48.0f, 18.0f);
            g.drawText (text, r, juce::Justification::centred);
        };

        drawLabelAt (rotaryStartAngle, "0");
        drawLabelAt ((rotaryStartAngle + rotaryEndAngle) * 0.5f, "+12");
        drawLabelAt (rotaryEndAngle, "+24");
    }
};

CloudOneLookAndFeel cloudOneLaf;

constexpr int kWindowWidth = 250;
constexpr int kWindowHeight = 340;

void drawScrew (juce::Graphics& g, juce::Point<float> centre)
{
    constexpr float r = 5.5f;
    g.setColour (juce::Colour (0xff0c0608));
    g.fillEllipse (centre.x - r, centre.y - r, r * 2.0f, r * 2.0f);
    g.setColour (juce::Colour (0xff4a1030));
    g.drawEllipse (centre.x - r, centre.y - r, r * 2.0f, r * 2.0f, 1.0f);
    g.setColour (juce::Colour (0xff8a3060));
    g.drawLine (centre.x - r * 0.6f, centre.y, centre.x + r * 0.6f, centre.y, 1.2f);
}

void styleCaption (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setFont (juce::Font (juce::FontOptions (15.0f)).withTypefaceStyle ("Bold"));
    l.setColour (juce::Label::textColourId, juce::Colour (0xfffbe6f2));
    l.setJustificationType (juce::Justification::centred);
}
}

CloudOneAudioProcessorEditor::CloudOneAudioProcessorEditor (CloudOneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setResizable (false, false);
    setSize (kWindowWidth, kWindowHeight);

    brightnessKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    brightnessKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    brightnessKnob.setRange (0.0, 24.0, 0.1);
    brightnessKnob.setLookAndFeel (&cloudOneLaf);
    brightnessKnob.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (brightnessKnob);

    title.setText ("CloudOne!", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (24.0f)).withTypefaceStyle ("Bold"));
    title.setColour (juce::Label::textColourId, juce::Colour (0xfffbe6f2));
    title.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title);

    subtitle.setText ("by erisa", juce::dontSendNotification);
    subtitle.setFont (juce::Font (juce::FontOptions (12.0f)).withTypefaceStyle ("Regular"));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xfffbe0ee));
    subtitle.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitle);

    styleCaption (mixCaption, "GAIN");
    addAndMakeVisible (mixCaption);

    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.parameters, CloudOneAudioProcessor::brightnessParamID, brightnessKnob);

    startTimerHz (30);
}

void CloudOneAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Outer black bezel - same bezel as HYPER SCAPE / Chocola, just a
    // different panel colour inside it.
    g.setColour (juce::Colour (0xff0a0a0c));
    g.fillRoundedRectangle (b, 14.0f);

    constexpr float bezelThickness = 12.0f;
    auto panel = b.reduced (bezelThickness);

    juce::ColourGradient bg (juce::Colour (0xffff6fb0), panel.getX(), panel.getY(),
                             juce::Colour (0xffb0146a), panel.getX(), panel.getBottom(), false);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (panel, 8.0f);

    g.setColour (juce::Colour (0xff7a0f47));
    g.drawRoundedRectangle (panel.reduced (1.0f), 8.0f, 1.2f);

    // Bolts sit inside the panel with a visible gap from the true corner,
    // like a hardware faceplate's corner mounting screws.
    constexpr float inset = 26.0f;
    drawScrew (g, { b.getX() + inset, b.getY() + inset });
    drawScrew (g, { b.getRight() - inset, b.getY() + inset });
    drawScrew (g, { b.getX() + inset, b.getBottom() - inset });
    drawScrew (g, { b.getRight() - inset, b.getBottom() - inset });

    // Small "alive" LED - brightens with the plugin's own output level,
    // like an activity/VU light on a hardware unit.
    const float glow = juce::jlimit (0.0f, 1.0f, ledLevel);
    const float baseR = 3.2f;
    const float glowR = baseR + glow * 5.0f;

    if (glow > 0.02f)
    {
        juce::ColourGradient haze (juce::Colour::fromFloatRGBA (1.0f, 0.4f, 0.7f, glow * 0.55f),
                                   ledCentre.x, ledCentre.y,
                                   juce::Colour::fromFloatRGBA (1.0f, 0.4f, 0.7f, 0.0f),
                                   ledCentre.x, ledCentre.y - glowR, true);
        haze.addColour (1.0, juce::Colour::fromFloatRGBA (1.0f, 0.4f, 0.7f, 0.0f));
        g.setGradientFill (haze);
        g.fillEllipse (ledCentre.x - glowR, ledCentre.y - glowR, glowR * 2.0f, glowR * 2.0f);
    }

    const juce::Colour ledColour = juce::Colour (0xff4a1030).interpolatedWith (juce::Colour (0xffffb8dd), glow);
    g.setColour (ledColour);
    g.fillEllipse (ledCentre.x - baseR, ledCentre.y - baseR, baseR * 2.0f, baseR * 2.0f);
}

void CloudOneAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();

    // Keep clear of the black bezel/outline on every side - nothing should
    // visually touch or cross that outer line.
    constexpr int topMargin = 30;
    constexpr int bottomMargin = 20;
    b.removeFromTop (topMargin);
    b.removeFromBottom (bottomMargin);

    auto top = b.removeFromTop (46);
    title.setBounds (top.removeFromTop (30));
    subtitle.setBounds (top.removeFromTop (14));

    ledCentre = { static_cast<float> (b.getCentreX()), static_cast<float> (b.getY()) + 16.0f };
    b.removeFromTop (10);

    constexpr int knobSize = 170;
    constexpr int captionHeight = 20;

    // Centre the knob+caption block in the remaining space rather than the
    // knob alone, so the pair reads as one unit.
    const int knobY = b.getY() + (b.getHeight() - knobSize - captionHeight) / 2;

    juce::Rectangle<int> knobArea (b.getCentreX() - knobSize / 2, knobY, knobSize, knobSize);

    brightnessKnob.setBounds (knobArea);

    // Tucked just under the knob (slight overlap into its own bounding
    // box) so the label reads as part of the knob.
    constexpr int captionOverlap = 8;
    mixCaption.setBounds (knobArea.getX(), knobArea.getBottom() - captionOverlap, knobSize, captionHeight);
}

void CloudOneAudioProcessorEditor::timerCallback()
{
    const float target = audioProcessor.getOutputLevel();
    // Fast-ish attack, slower release - reads as a natural pulse rather
    // than a flicker.
    const float coeff = target > ledLevel ? 0.55f : 0.12f;
    ledLevel += (target - ledLevel) * coeff;
    repaint();
}
