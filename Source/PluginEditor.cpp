#include "PluginProcessor.h"
#include "PluginEditor.h"

// ---------------------------------------------------------------------
// AnalogKnobLookAndFeel — back to the thin, simple ring style from the
// first pink version: a slim track, a pink value arc, a small pointer.
// Tick marks and their labels now sit OUTSIDE the ring on the panel,
// like markings printed around a real hardware knob rather than on it.
// ---------------------------------------------------------------------
void AnalogKnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                               juce::Slider&)
{
    // The full allocated square includes room for the ring AND the tick
    // marks/labels around it — that's why the component's bounds are
    // sized generously by the editor, not just tight to the ring.
    auto fullArea = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.0f);
    auto diameter = juce::jmin (fullArea.getWidth(), fullArea.getHeight());
    auto bounds   = juce::Rectangle<float> (diameter, diameter).withCentre (fullArea.getCentre());
    auto radius   = diameter / 2.0f;
    auto centre   = bounds.getCentre();
    auto angle    = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    auto ringRadius = radius * 0.50f;

    // Knob body — brighter grey than before, with a subtle gradient so
    // it reads as a rounded surface rather than a flat disc.
    auto knobBounds = juce::Rectangle<float> (ringRadius * 2.0f, ringRadius * 2.0f).withCentre (centre);
    juce::ColourGradient shading (juce::Colour (0xffc4c4c4), knobBounds.getX(), knobBounds.getY(),
                                   juce::Colour (0xff9a9a9a), knobBounds.getRight(), knobBounds.getBottom(),
                                   false);
    g.setGradientFill (shading);
    g.fillEllipse (knobBounds);

    // Thin outline ring around the edge — thickness unchanged.
    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawEllipse (knobBounds, 2.0f);

    // Pointer line — black now, slightly smaller, pointing at the value.
    juce::Path pointer;
    auto pointerLength    = ringRadius * 0.72f;
    auto pointerThickness = 1.8f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength,
                                  pointerThickness, pointerLength * 0.9f,
                                  pointerThickness * 0.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (juce::Colours::black);
    g.fillPath (pointer);

    // Tick marks — OUTSIDE the ring, on the panel, not on the knob.
    const int numTicks = 21;
    for (int i = 0; i < numTicks; ++i)
    {
        auto t         = (float) i / (float) (numTicks - 1);
        auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        bool isMajor   = (i == 0 || i == numTicks / 2 || i == numTicks - 1);

        auto inner = centre.getPointOnCircumference (radius * 0.60f, tickAngle);
        auto outer = centre.getPointOnCircumference (isMajor ? radius * 0.80f : radius * 0.70f, tickAngle);

        g.setColour (juce::Colours::white.withAlpha (isMajor ? 0.95f : 0.5f));
        g.drawLine ({ inner, outer }, isMajor ? 2.0f : 1.0f);
    }

    // Labels — "0" at the start, "+12" at the top (halfway point of our
    // 0-24 range), "+24" at the end.
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.setColour (juce::Colours::white.withAlpha (0.9f));

    auto labelPoint = [&] (float a) { return centre.getPointOnCircumference (radius * 0.90f, a); };

    auto zeroPos = labelPoint (rotaryStartAngle);
    g.drawFittedText ("0", juce::Rectangle<int> (20, 16).withCentre (zeroPos.toInt()),
                       juce::Justification::centred, 1);

    auto midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
    auto midPos   = labelPoint (midAngle);
    g.drawFittedText ("+12", juce::Rectangle<int> (30, 16).withCentre (midPos.toInt()),
                       juce::Justification::centred, 1);

    auto maxPos = labelPoint (rotaryEndAngle);
    g.drawFittedText ("+24", juce::Rectangle<int> (30, 16).withCentre (maxPos.toInt()),
                       juce::Justification::centred, 1);
}

// ---------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------
CloudOneAudioProcessorEditor::CloudOneAudioProcessorEditor (CloudOneAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    brightnessKnob.setLookAndFeel (&knobLookAndFeel);
    brightnessKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    brightnessKnob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                         juce::MathConstants<float>::pi * 2.8f,
                                         true);
    brightnessKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (brightnessKnob);

    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.parameters, CloudOneAudioProcessor::brightnessParamID, brightnessKnob);

    setSize (260, 330);

    // Pre-generate a fixed film-grain texture sized to the actual panel
    // (bounds reduced by 10px on each side), so it never flickers and
    // always matches the current window size.
    const int texW = getWidth() - 20;
    const int texH = getHeight() - 20;
    noiseTexture = juce::Image (juce::Image::ARGB, texW, texH, true);
    juce::Random rng (12345);
    juce::Image::BitmapData bitmap (noiseTexture, juce::Image::BitmapData::writeOnly);
    for (int py = 0; py < texH; ++py)
    {
        for (int px = 0; px < texW; ++px)
        {
            auto isLight = rng.nextBool();
            auto alpha   = rng.nextFloat() * 0.05f; // almost nothing, just grain
            bitmap.setPixelColour (px, py, (isLight ? juce::Colours::white : juce::Colours::black)
                                              .withAlpha (alpha));
        }
    }
}

CloudOneAudioProcessorEditor::~CloudOneAudioProcessorEditor()
{
    brightnessKnob.setLookAndFeel (nullptr);
}

void CloudOneAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Outer dark rack bezel.
    g.fillAll (juce::Colour (0xff0c0c0c));

    // Pink analog panel, metallic gradient top-to-bottom.
    auto panel = bounds.reduced (10.0f);
    juce::ColourGradient panelGradient (juce::Colour (0xffff6fb0), panel.getX(), panel.getY(),
                                         juce::Colour (0xffb0146a), panel.getX(), panel.getBottom(),
                                         false);
    g.setGradientFill (panelGradient);
    g.fillRoundedRectangle (panel, 10.0f);

    // Subtle film-grain texture — keeps the panel from looking like a
    // flat digital gradient, gives it that analog-hardware feel.
    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path panelPath;
        panelPath.addRoundedRectangle (panel, 10.0f);
        g.reduceClipRegion (panelPath);
        g.drawImageAt (noiseTexture, (int) panel.getX(), (int) panel.getY());
    }

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle (panel, 10.0f, 1.5f);

    // Corner screws, hardware-style.
    auto drawScrew = [&] (float cx, float cy)
    {
        g.setColour (juce::Colour (0xff2a2a2a));
        g.fillEllipse (cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawLine (cx - 3.0f, cy, cx + 3.0f, cy, 1.2f);
    };

    drawScrew (panel.getX() + 16.0f, panel.getY() + 16.0f);
    drawScrew (panel.getRight() - 16.0f, panel.getY() + 16.0f);
    drawScrew (panel.getX() + 16.0f, panel.getBottom() - 16.0f);
    drawScrew (panel.getRight() - 16.0f, panel.getBottom() - 16.0f);

    // Title — taller-looking display font with a black outline.
    juce::Font titleFont (28.0f, juce::Font::bold);
    titleFont = titleFont.withHorizontalScale (0.8f);
    g.setFont (titleFont);

    const juce::String title = "CloudOne!";
    g.setColour (juce::Colours::black);
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (dx != 0 || dy != 0)
                g.drawText (title, titleBounds.translated (dx, dy), juce::Justification::centred);

    g.setColour (juce::Colours::white);
    g.drawText (title, titleBounds, juce::Justification::centred);

    // "GAIN" label under the knob — plain white, no outline.
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.setColour (juce::Colours::white);
    g.drawText ("GAIN", gainLabelBounds, juce::Justification::centred);
}

void CloudOneAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    titleBounds = area.removeFromTop (40);
    area.removeFromTop (8);

    auto knobSize = juce::jmin (area.getWidth(), 210);
    auto knobArea = area.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize);
    brightnessKnob.setBounds (knobArea);

    area.removeFromTop (4);
    gainLabelBounds = area.removeFromTop (24);
}
