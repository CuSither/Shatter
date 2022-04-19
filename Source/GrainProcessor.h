#pragma once
#include <JuceHeader.h>

struct Grain
{
    Grain(int grainSize, int grainPanning, int startPosition) : size(grainSize), readIndex(startPosition),
        writeIndex(0), panning(grainPanning)
    {
        //create window
        window.resize(size);
        for (int i = 0; i < size; ++i)
        {
            window[i] = pow(sin(((float)i / size) * M_PI), 2);
        }
        
//        std::cout<<"[";
//
//        for (int i = 0; i < size; ++i)
//        {
//            std::cout<<window[i]<<", ";
//        }
//
//        std::cout<<"]"<<std::endl;
    }
    
    int size;
    int readIndex; // position/Users/alex/JuceProjects/Shatter/Source/GrainProcessor.cpp in delayBuffer
    int writeIndex;// position in grain
    double panning;
    
//    float* window;
    std::vector<float> window;
    
};

class GrainProcessor
{
public:
    GrainProcessor();
    
    void initialize(double sr);
    
    void grainify(juce::AudioBuffer<float>& audioBuffer);
    void spawnGrains(juce::AudioBuffer<float>& audioBuffer);
    void readFromGrains(juce::AudioBuffer<float>& audioBuffer);
    void writeToDelayBuffer(juce::AudioBuffer<float>& audioBuffer);
    
//    Grain initGrain(int size, double panning, int startPosition);
//    void fillWindow(Grain& grain);
    
    void incrementBufferWriteIndex(int numSamples);
    int copyFromBufferWithWraparound(juce::AudioBuffer<float>& tempBuffer, Grain& grain);
    
    void addFromTempBuffer(juce::AudioBuffer<float>& audioBuffer, juce::AudioBuffer<float>& tempBuffer);
    
    void applyPanning(juce::AudioBuffer<float>& tempBuffer, Grain& grain);
    void applyWindow(juce::AudioBuffer<float>& tempBuffer, Grain& grain, int numSamplesRead);
    void updateGrain(Grain& grain, int numSamplesWritten);
    int getRelativeStartIndex(Grain grain);
//    void setSamplesToNextGrain();
    void setSampleRate(double sr);
    
    void testDelayBuffer(juce::AudioBuffer<float>& audioBuffer);

private:
    double sampleRate;
    std::vector<Grain> grains;

    std::unique_ptr<juce::AudioBuffer<float>> delayBuffer;
    int delayBufferSize;
    int delayBufferNumChannels;
    int delayBufferWriteIndex;
    
    int samplesToNextGrain;
    
    int globalGrainSize;
    double grainFrequency;
    double globalGrainPanning;
};
