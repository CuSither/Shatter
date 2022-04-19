#include "GrainProcessor.h"


GrainProcessor::GrainProcessor()
{
}

void GrainProcessor::initialize(double sr)
{
    sampleRate = sr;
    
    delayBufferWriteIndex = 0;
    delayBufferNumChannels = 2;
    delayBufferSize = (int)(sampleRate * 5);
    
    delayBuffer = std::make_unique<juce::AudioBuffer<float>>(delayBufferNumChannels, delayBufferSize);
    delayBuffer->clear();
    
    grainFrequency = 30;
    globalGrainSize = (int)(sampleRate / grainFrequency * 2);
    samplesToNextGrain = 0;
    globalGrainPanning = 0;
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
//    std::cout<<"spawn"<<std::endl;
    int bufferSize = audioBuffer.getNumSamples();
    int bufferIndex = 0;
    
    while (bufferSize > bufferIndex + samplesToNextGrain)
    {
        bufferIndex += samplesToNextGrain;
        int startPosition = (delayBufferWriteIndex + bufferIndex) % delayBufferSize;
        
//        std::cout<<"initializing grain with size "<<globalGrainSize<<" and position "<<startPosition<<std::endl;
        Grain newGrain(globalGrainSize, globalGrainPanning, startPosition);
        grains.push_back(newGrain);
//        std::cout<<"grain start position: "<<startPosition<<std::endl;
//        std::cout<<grains.size()<<" grains"<<std::endl;
        
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
        
//        applyPanning(tempBuffer, *grain);
        applyWindow(tempBuffer, *grain, numSamplesRead);
        
//        addFromTempBuffer(audioBuffer, tempBuffer);
        
        for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
        {
            audioBuffer.addFrom(channel, 0, tempBuffer, channel, 0, audioBuffer.getNumSamples());
        }

        updateGrain(*grain, numSamplesRead);
        
        // check if grain is eaten
        if (grain->writeIndex >= grain->size)
        {
//            std::cout<<"deleting grain"<<std::endl;
            grain = grains.erase(grain);
        }
        else
        {
            ++grain;
        }
    }
}
//
//void GrainProcessor::fillWindow(Grain &grain)
//{
//    for (int i = 0; i < grain.size; ++i)
//    {
//        grain.window[i] = sin((float)i / grain.size * M_PI);
//    }
//}

//Grain GrainProcessor::initGrain(int size, double panning, int startPosition)
//{
//    Grain newGrain;
//
//    newGrain.size = grainSize;
//    newGrain.readIndex = startPosition;
//    newGrain.writeIndex = 0;
//    newGrain.panning = grainPanning;
//
//
//    newGrain.window(grainSize, juce::dsp::WindowingFunction<float>::WindowingMethod::hann);
////    windowingFunction.fillWindowingTables(newGrain.window, grainSize, windowingMethod);
//
//    return newGrain;
//}

//void GrainProcessor::addFromTempBuffer(juce::AudioBuffer<float>& audioBuffer, juce::AudioBuffer<float>& tempBuffer)
//{
//    for (int channel = 0; channel < audioBuffer.getNumChannels(); channel++)
//    {
//        audioBuffer.addFrom(channel, 0, tempBuffer, channel, 0, audioBuffer.getNumSamples());
//    }
//}

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
//            if (windowReadIndex > grain.size)
//            {
//                std::cout<<"windowReadIndex: "<<windowReadIndex<<std::endl;
//            }
            
            auto before = tempBuffer.getSample(channel, i + grainRelativeStartIndex);
            auto after = tempBuffer.getSample(channel, i + grainRelativeStartIndex) * grain.window[windowReadIndex];
            if (after > 1)
            {
                std::cout<<"sample before: "<<before<<std::endl;
                std::cout<<"windowReadIndex: "<<windowReadIndex<<std::endl;
                std::cout<<"Window: "<<grain.window[windowReadIndex]<<std::endl;
                std::cout<<"sample after: "<<after<<std::endl;
            }
            *tempBuffer.getWritePointer(channel, i + grainRelativeStartIndex) = tempBuffer.getSample(channel, i + grainRelativeStartIndex) * grain.window[windowReadIndex];

        }
    }
}

void GrainProcessor::updateGrain(Grain& grain, int numSamplesWritten)
{
//    std::cout<<"Number of samples written: "<<numSamplesWritten<<std::endl;
    grain.readIndex = (grain.readIndex + numSamplesWritten) % delayBufferSize;
    grain.writeIndex += numSamplesWritten;
//    std::cout<<"grain writeIndex is now "<<grain.writeIndex<<std::endl;
}



//int GrainProcessor::getDelayBufferIndexForGrain(int readIndex)
//{
//    return (delayBufferWriteIndex + readIndex) % delayBufferSize;
//}


void GrainProcessor::incrementBufferWriteIndex(int numSamples)
{
    delayBufferWriteIndex = (delayBufferWriteIndex + numSamples) % delayBufferSize;
}

int GrainProcessor::getRelativeStartIndex(Grain grain)
{
    // If the grain is new, this returns where in the current buffer it starts
    if (grain.writeIndex == 0)
    {
        int grainRelativeStartIndex = grain.readIndex - delayBufferWriteIndex;
        return (grainRelativeStartIndex >= 0) ? grainRelativeStartIndex : grainRelativeStartIndex + delayBufferSize;
    }
    else
    {
        return 0;
    }
    
}

void GrainProcessor::setSampleRate(double sr)
{
    sampleRate = sr;
}


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
