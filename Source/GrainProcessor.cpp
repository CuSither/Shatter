#include "GrainProcessor.h"


GrainProcessor::GrainProcessor()
{
    delayBufferWriteIndex = 0;
    delayBufferNumChannels = 2;
    delayBufferSize = 196000;
    delayBuffer = std::make_unique<juce::AudioBuffer<float>>(delayBufferNumChannels, delayBufferSize);
    delayBuffer->clear();
    
    samplesToNextGrain = 0;
    
}

void GrainProcessor::grainify(juce::AudioBuffer<float>& audioBuffer)
{    
    writeToDelayBuffer(audioBuffer);
    spawnGrains(audioBuffer);
    readFromGrains(audioBuffer);

    delayBufferWriteIndex = (delayBufferWriteIndex + audioBuffer.getNumSamples()) % delayBufferSize;
}

void GrainProcessor::writeToDelayBuffer(juce::AudioBuffer<float>& audioBuffer)
{
    int bufferSize = audioBuffer.getNumSamples();
    
    for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
    {
        if (delayBufferWriteIndex + bufferSize < delayBufferSize)
        {
            delayBuffer->copyFrom(channel, delayBufferWriteIndex, audioBuffer, channel, 0, bufferSize);
        }
        else
        {
            int samplesRemaining = delayBufferSize - delayBufferWriteIndex;
            
            delayBuffer->copyFrom(channel, delayBufferWriteIndex, audioBuffer, channel, 0, samplesRemaining);
            delayBuffer->copyFrom(channel, 0, audioBuffer, channel, samplesRemaining, bufferSize - samplesRemaining);
        }
    }
}

void GrainProcessor::spawnGrains(juce::AudioBuffer<float>& audioBuffer)
{
    int bufferSize = audioBuffer.getNumSamples();
    int bufferIndex = 0;

    while (bufferSize > bufferIndex + samplesToNextGrain)
    {
        bufferIndex += samplesToNextGrain;
        
        double randomDelay = randomizer.nextDouble() * grainSpread;
        int startPosition = (delayBufferWriteIndex + bufferIndex) - (int)(randomDelay * sampleRate);
        if (startPosition < 0)  { startPosition += delayBufferSize; }
        startPosition %= delayBufferSize;
    
        double pan = grainWidth == 0 ? 0 : (randomizer.nextDouble() * 2 - 1) * grainWidth;
        int size = (int) (std::max(0.1, std::min(1.0, globalGrainSize + (randomizer.nextDouble() - 0.5) * grainSizeRandom)) * sampleRate);  // grain size in terms of samples
        
        Grain newGrain(size, pan, startPosition, bufferIndex);
        grains.push_back(newGrain);
        
        double grainFrequency = std::max(1.0, std::min(40.0, globalGrainFrequency + (randomizer.nextDouble() * 10 - 5) * grainFrequencyRandom));
        samplesToNextGrain = (int)(sampleRate / grainFrequency);
    }
    
    samplesToNextGrain = samplesToNextGrain - (bufferSize - bufferIndex);
}

void GrainProcessor::readFromGrains(juce::AudioBuffer<float>& audioBuffer)
{
    int audioBufferSize = audioBuffer.getNumSamples();
    audioBuffer.clear();
    juce::AudioBuffer<float> tempBuffer(delayBufferNumChannels, audioBufferSize);
    
    auto grain = grains.begin();
    
    while (grain != grains.end())
    {
        tempBuffer.clear();
        
        int numSamplesRead = copyFromBufferWithWraparound(tempBuffer, *grain);
        
        applyPanning(tempBuffer, *grain);
        applyWindow(tempBuffer, *grain, numSamplesRead);
        
        for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
        {
            audioBuffer.addFrom(channel, 0, tempBuffer, channel, 0, audioBuffer.getNumSamples());
        }

        updateGrain(*grain, numSamplesRead);
        
        // check if grain is eaten
        if (grain->writeIndex >= grain->size)
        {
            grain = grains.erase(grain);
        }
        else
        {
            ++grain;
        }
    }
}

int GrainProcessor::copyFromBufferWithWraparound(juce::AudioBuffer<float>& tempBuffer, Grain& grain)
{
    int grainRelativeStartIndex = getRelativeStartIndex(grain);
    
    int grainSamplesRemaining = grain.size - grain.writeIndex;
    int amountToCopy = std::min(grainSamplesRemaining, tempBuffer.getNumSamples() - grainRelativeStartIndex);
    
    for (int channel = 0; channel < delayBufferNumChannels; ++channel)
    {
        if (grain.readIndex + amountToCopy < delayBufferSize)
        {
            tempBuffer.copyFrom(channel, grainRelativeStartIndex, *delayBuffer, channel, grain.readIndex, amountToCopy);
        }
        else
        {
            int samplesRemaining = delayBufferSize - grain.readIndex;
            
            tempBuffer.copyFrom(channel, grainRelativeStartIndex, *delayBuffer, channel, grain.readIndex, samplesRemaining);
            tempBuffer.copyFrom(channel, grainRelativeStartIndex + samplesRemaining, *delayBuffer, channel, 0, amountToCopy - samplesRemaining);
        }
    }
    
    return amountToCopy;
}

void GrainProcessor::applyPanning(juce::AudioBuffer<float>& tempBuffer, Grain& grain)
{
    if (grain.panning > 0)
    {
        tempBuffer.applyGain(0, 0, tempBuffer.getNumSamples(), 1 - grain.panning);
    }
    else if (grain.panning < 0)
    {
        tempBuffer.applyGain(1, 0, tempBuffer.getNumSamples(), 1 + grain.panning);
    }
}

void GrainProcessor::applyWindow(juce::AudioBuffer<float>& tempBuffer, Grain& grain, int numSamplesRead)
{
    int grainRelativeStartIndex = getRelativeStartIndex(grain);
    
    for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
    {
        for (int i = 0; i < numSamplesRead; ++i)
        {
            int windowReadIndex = i + grain.writeIndex;

            *tempBuffer.getWritePointer(channel, i + grainRelativeStartIndex) = tempBuffer.getSample(channel, i + grainRelativeStartIndex) * grain.window[windowReadIndex];
        }
    }
}

void GrainProcessor::updateGrain(Grain& grain, int numSamplesWritten)
{
    grain.readIndex = (grain.readIndex + numSamplesWritten) % delayBufferSize;
    grain.writeIndex += numSamplesWritten;
}

int GrainProcessor::getRelativeStartIndex(Grain grain)
{
    // If the grain is new, this returns where in the current buffer it starts
    if (grain.writeIndex == 0)
    {
        return grain.relativeStartIndex;
    }
    else
    {
        return 0;
    }
}

void GrainProcessor::setSampleRate(double sr)                   { sampleRate = sr; }

void GrainProcessor::setGrainSize(double grainSize)             { globalGrainSize = grainSize; }
void GrainProcessor::setGrainFrequency(double grainFrequency)   { globalGrainFrequency = grainFrequency; }
void GrainProcessor::setGrainWidth(double width)                { grainWidth = width; }
void GrainProcessor::setGrainRandomSize(double randomAmount)    { grainSizeRandom = randomAmount; }
void GrainProcessor::setGrainRandomFreq(double randomAmount)    { grainFrequencyRandom = randomAmount; }
void GrainProcessor::setGrainSpread(double spreadMilliseconds)  { grainSpread = spreadMilliseconds / 1000; }

double GrainProcessor::getGrainSize()                           { return globalGrainSize; }
double GrainProcessor::getGrainFrequency()                      { return globalGrainFrequency; }
double GrainProcessor::getGrainRandomSize()                     { return grainSizeRandom; }
double GrainProcessor::getGrainRandomFreq()                     { return grainFrequencyRandom; }
double GrainProcessor::getGrainWidth()                          { return grainWidth; }
double GrainProcessor::getGrainSpread()                         { return grainSpread; }


void GrainProcessor::testDelayBuffer(juce::AudioBuffer<float>& audioBuffer)
{
    for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
    {
        int delayBufferReadIndex = delayBufferWriteIndex - (sampleRate * 0.5);
        if (delayBufferReadIndex < 0) {delayBufferReadIndex += delayBufferSize;}
        if (delayBufferReadIndex + audioBuffer.getNumSamples() < delayBufferSize)
        {
            audioBuffer.addFrom(channel, 0, *delayBuffer, channel, delayBufferReadIndex, audioBuffer.getNumSamples());
        }
        else
        {
            audioBuffer.addFrom(channel, 0, *delayBuffer, channel, delayBufferReadIndex, delayBufferSize - delayBufferReadIndex);
            audioBuffer.addFrom(channel, delayBufferSize - delayBufferReadIndex, *delayBuffer, channel, 0, audioBuffer.getNumSamples() - (delayBufferSize - delayBufferReadIndex));
        }
    }
}
