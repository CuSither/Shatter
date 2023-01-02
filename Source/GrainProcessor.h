#pragma once
#include <JuceHeader.h>

struct Grain
{
    Grain(int grainSize, double grainPanning, int startPosition, int startIndex) : size(grainSize), readIndex(startPosition),
        writeIndex(0), relativeStartIndex(startIndex), panning(grainPanning)
    {
        //create hanning window
        window.resize(size);
        for (int i = 0; i < size; ++i)
        {
            window[i] = pow(sin(((float)i / size) * M_PI), 2);
        }
    }
    
    int size;
    int readIndex;  // position in delayBuffer
    int writeIndex; // position in grain
    int relativeStartIndex;
    double panning;

    std::vector<float> window;
};

class GrainProcessor
{
public:
    GrainProcessor();
    
    void grainify(juce::AudioBuffer<float>& audioBuffer);
    void setSampleRate(double sr);
    
    void setGrainSize(double grainSize);
    void setGrainRandomSize(double randomAmount);
    void setGrainFrequency(double grainFrequency);
    void setGrainRandomFreq(double randomAmount);
    void setGrainWidth(double width);
    void setGrainSpread(double spread);
    
    double getGrainSize();
    double getGrainFrequency();
    double getGrainRandomSize();
    double getGrainRandomFreq();
    double getGrainWidth();
    double getGrainSpread();

private:
    void spawnGrains(juce::AudioBuffer<float>& audioBuffer);
    void readFromGrains(juce::AudioBuffer<float>& audioBuffer);
    void writeToDelayBuffer(juce::AudioBuffer<float>& audioBuffer);
    
    int copyFromBufferWithWraparound(juce::AudioBuffer<float>& tempBuffer, Grain& grain);
    int getRelativeStartIndex(Grain grain);
    
    void applyPanning(juce::AudioBuffer<float>& tempBuffer, Grain& grain);
    void applyWindow(juce::AudioBuffer<float>& tempBuffer, Grain& grain, int numSamplesRead);
    void updateGrain(Grain& grain, int numSamplesWritten);

    void testDelayBuffer(juce::AudioBuffer<float>& audioBuffer);
    
    double sampleRate;
    std::vector<Grain> grains;

    std::unique_ptr<juce::AudioBuffer<float>> delayBuffer;
    
    int delayBufferSize;
    int delayBufferNumChannels;
    int delayBufferWriteIndex;
    int samplesToNextGrain;
    
    double globalGrainSize;         // grain size in terms of seconds
    double grainSizeRandom;
    double globalGrainFrequency;    // grain frequency in terms of hz
    double grainFrequencyRandom;
    double grainWidth;
    double grainSpread;
    
    juce::Random randomizer;
};
