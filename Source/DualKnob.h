#pragma once

#include <JuceHeader.h>
#include "CustomKnob.h"
//#include "PluginEditor.h"


class DualKnob : public juce::Component
{
public:
    DualKnob(std::string firstKnobName, double firstKnobSkewFactor, std::string firstKnobID, std::string secondKnobName, double secondKnobSkewFactor, std::string secondKnobID, juce::AudioProcessorValueTreeState& valueTreeState, juce::Slider::Listener* sliderListener);
    ~DualKnob() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    juce::Slider firstKnob;
    juce::Slider secondKnob;
    
private:
    void initKnob(juce::Slider& knob, juce::Label& label, std::string name, double skewFactor, std::string ID);
    
    CustomKnob knobLookAndFeel;
    
    juce::Label firstKnobLabel;
    juce::Label secondKnobLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> firstKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> secondKnobAttachment;
    
    int outlineThickness;
    int curveAmount;
};
