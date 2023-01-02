#include "DualKnob.h"


DualKnob::DualKnob(std::string firstKnobName, double firstKnobSkewFactor, std::string firstKnobID, std::string secondKnobName, double secondKnobSkewFactor, std::string secondKnobID, juce::AudioProcessorValueTreeState& valueTreeState, juce::Slider::Listener* sliderListener)
{
    setLookAndFeel(&knobLookAndFeel);
    
    outlineThickness = 2;
    curveAmount = 15;
    
    initKnob(firstKnob, firstKnobLabel, firstKnobName, firstKnobSkewFactor, firstKnobID);
    initKnob(secondKnob, secondKnobLabel, secondKnobName, secondKnobSkewFactor, secondKnobID);
    
    firstKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, firstKnobID, firstKnob);
    secondKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, secondKnobID, secondKnob);
}

DualKnob::~DualKnob()
{
    setLookAndFeel (nullptr);
}

void DualKnob::paint(juce::Graphics& g)
{
    g.setColour (juce::Colours::darkslategrey);
    g.setOpacity(0.97);
    g.fillRoundedRectangle(getLocalBounds().reduced(outlineThickness).toFloat(), curveAmount);
    
    g.setColour (juce::Colours::black);
    g.setOpacity(1);
    g.drawRoundedRectangle(getLocalBounds().reduced(outlineThickness).toFloat(), curveAmount, outlineThickness);
}

void DualKnob::resized()
{
    int knobSize = getWidth() / 3.5;
    
    firstKnob.setSize(knobSize, knobSize);
    firstKnob.setCentrePosition(getWidth() / 4, getHeight() / 2);
    
    secondKnob.setSize(knobSize, knobSize);
    secondKnob.setCentrePosition(getWidth() / 4 * 3, getHeight() / 2);
    
}

void DualKnob::initKnob(juce::Slider& knob, juce::Label& label, std::string name, double skewFactor, std::string ID)
{
    addAndMakeVisible(knob);
    knob.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    knob.setComponentID(ID);
    knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    knob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    
    addAndMakeVisible(label);
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::horizontallyCentred);
    label.attachToComponent(&knob, false);
}
