/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DualKnob.h"

//==============================================================================
/**
*/
class ShatterAudioProcessorEditor  :  public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    ShatterAudioProcessorEditor (ShatterAudioProcessor&);
    ~ShatterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
        
    void sliderValueChanged(juce::Slider* sliderChanged) override;

private:
    ShatterAudioProcessor& audioProcessor;

    juce::Image background = juce::ImageCache::getFromMemory(BinaryData::particles_jpg, BinaryData::particles_jpgSize);
    
    DualKnob sizeKnobs;
    DualKnob densityKnobs;
    DualKnob widthAndSpreadKnobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShatterAudioProcessorEditor)
};
