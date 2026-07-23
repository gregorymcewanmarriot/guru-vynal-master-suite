#include "GuruLookAndFeel.h"

GuruLookAndFeel::GuruLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe9e2d2));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff17191b));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff35393d));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff181a1d));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffe9e2d2));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4b4f52));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff181a1d));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe9e2d2));
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff2ead8));
    setColour(juce::TextButton::textColourOnId, juce::Colour(0xff101112));
}

void GuruLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                       juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                static_cast<float>(width), static_cast<float>(height)).reduced(7.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(juce::Colour(0xff0e0f10));
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xff303438));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                      rotaryStartAngle, angle, true);
    g.setColour(juce::Colour(0xffd6922e));
    g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

    juce::Path pointer;
    const float pointerLength = radius * 0.62f;
    const float pointerThickness = 2.5f;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength,
                                pointerThickness, pointerLength, 1.2f);
    g.setColour(juce::Colour(0xfff1e6cc));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

    juce::ignoreUnused(slider);
}

void GuruLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour&, bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto colour = button.getToggleState() ? juce::Colour(0xffd6922e) : juce::Colour(0xff24272a);
    if (highlighted)
        colour = colour.brighter(0.08f);
    if (down)
        colour = colour.darker(0.12f);

    g.setColour(colour);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(juce::Colour(0xff555a5e));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}
