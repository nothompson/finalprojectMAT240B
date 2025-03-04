#pragma once

#include <JuceHeader.h>

class FFTProcessor
{
    public:
        FFTProcessor();
        // typically a latency of one fft block size before output buffer gets read
        int getLatencyInSamples() const {return fftSize;}
        void reset();
        // main method to place samples into input FIFO, perform FFT every hop, reads from output FIFO and returns the value
        float processSample(float sample);
        void processBlock(float* data, int numSamples);

    private:

        void processFrame();

        void processSpectrum(float* data, int numBins);

        static constexpr int fftOrder = 10;
        // 1024 samples ( 2^10)
        static constexpr int fftSize = 1 << fftOrder; 
        // 513 bins 
        static constexpr int numBins = (fftSize / 2) + 1; 
        // 75% overlap with hann window
        static constexpr int overlap = 4;
        // 256 sample hop size
        static constexpr int hopSize = fftSize / overlap;

        static constexpr float windowCorrection = 2.0f/ 3.0f;

        // need to get built-in JUCE objects. member variables of FFTProcessor
        juce::dsp::FFT fft;
        juce::dsp::WindowingFunction<float> window;

        // variables to manage FIFO 
        // count up until next hop block
        int count = 0;
        // position within buffer
        // write into input FIFO , read into output FIFO
        int pos = 0;

        // fifo circular buffer 
        std::array<float, fftSize> inputFifo;
        std::array<float, fftSize> outputFifo;

        // where the fft takes place
        // fft uses complex numbers, double the length since each complex is made up of two floats
        std::array<float,fftSize * 2>fftData;

        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTProcessor)
};