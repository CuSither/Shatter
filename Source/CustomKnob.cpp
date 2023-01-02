#include "CustomKnob.h"

void CustomKnob::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &)
{
    auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // fill
    g.setColour (juce::Colours::snow);
    g.fillEllipse (rx, ry, rw, rw);

    // outline
    g.setColour (juce::Colours::darkred);
    g.drawEllipse (rx, ry, rw, rw, 2.0f);
    
    juce::Path p;
    auto pointerLength = radius * 0.5f;
    auto pointerThickness = 3.0f;
    p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    
    // pointer
    g.setColour (juce::Colours::darkred);
    g.fillPath (p);
}

