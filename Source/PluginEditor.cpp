/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ShatterAudioProcessorEditor::ShatterAudioProcessorEditor (ShatterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), sizeKnobs("Size", 1.0, "SIZE", "Random", 1.0, "SIZERANDOM", p.apvts, this), densityKnobs("Density", 1.0, "DENSITY", "Random", 1.0, "DENSITYRANDOM", p.apvts, this), widthAndSpreadKnobs("Width", 1.0, "WIDTH", "Spread", 0.3, "SPREAD", p.apvts, this)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, true);
    setSize (800, 450);
    
    addAndMakeVisible(sizeKnobs);
    addAndMakeVisible(densityKnobs);
    addAndMakeVisible(widthAndSpreadKnobs);
        
}

ShatterAudioProcessorEditor::~ShatterAudioProcessorEditor()
{
}

//==============================================================================

void ShatterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.drawImage(background, getLocalBounds().toFloat());

}

void ShatterAudioProcessorEditor::resized()
{
    juce::Rectangle<int> localBounds = getLocalBounds();
    int height = localBounds.getHeight();
    int width = localBounds.getWidth();
    
    juce::Rectangle<int> top(localBounds.removeFromTop(height / 2));
    juce::Rectangle<int> topLeft(top.removeFromLeft(width / 2));

    sizeKnobs.setBounds(topLeft.reduced(top.getHeight() / 8));
    densityKnobs.setBounds(top.reduced(top.getHeight() / 8));
    widthAndSpreadKnobs.setBounds(localBounds.reduced((width - sizeKnobs.getWidth()) / 2, (height / 2 - sizeKnobs.getHeight()) / 2));
}

void ShatterAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
}
