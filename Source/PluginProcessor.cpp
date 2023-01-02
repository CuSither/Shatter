/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ShatterAudioProcessor::ShatterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), grainMill(std::make_unique<GrainProcessor>()), apvts(*this, nullptr, "Parameters", initParameters())
#endif
{
}

ShatterAudioProcessor::~ShatterAudioProcessor()
{
}

//==============================================================================
const juce::String ShatterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShatterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShatterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShatterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ShatterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShatterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShatterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShatterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ShatterAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShatterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ShatterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    grainMill->setSampleRate(sampleRate);
    
    
}

void ShatterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShatterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ShatterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    grainMill->setGrainSize(*apvts.getRawParameterValue("SIZE"));
    grainMill->setGrainRandomSize(*apvts.getRawParameterValue("SIZERANDOM"));
    grainMill->setGrainFrequency(*apvts.getRawParameterValue("DENSITY"));
    grainMill->setGrainRandomFreq(*apvts.getRawParameterValue("DENSITYRANDOM"));
    grainMill->setGrainWidth(*apvts.getRawParameterValue("WIDTH"));
    grainMill->setGrainSpread(*apvts.getRawParameterValue("SPREAD"));
   
    grainMill->grainify(buffer);
}

//==============================================================================
bool ShatterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ShatterAudioProcessor::createEditor()
{
    return new ShatterAudioProcessorEditor (*this);
}

//==============================================================================
void ShatterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);

}

void ShatterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//===========================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShatterAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ShatterAudioProcessor::initParameters()
{
    // initial parameter values
    float initSize = 0.4f;
    float initDensity = 5.0f;
    float initRandom = 0.0f;
    float initWidth = 0.0f;
    float initSpread = 0.0f;
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"SIZE", 1}, "Size", 0.05f, 2.0f, initSize));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"SIZERANDOM", 1}, "Size Random", 0.0f, 1.0f, initRandom));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"DENSITY", 1}, "Density", 1.0f, 30.0f, initDensity));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"DENSITYRANDOM", 1}, "Density Random", 0.0f, 1.0f, initRandom));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"WIDTH", 1}, "Width", 0.0f, 1.0f, initWidth));
    juce::NormalisableRange<float> spreadRange = juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f);
    spreadRange.setSkewForCentre(200.0);
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"SPREAD", 1}, "Spread", spreadRange, initSpread));
       
    return {parameters.begin(), parameters.end()};
}
