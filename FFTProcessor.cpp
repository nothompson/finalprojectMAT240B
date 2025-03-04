#include "FFTProcessor.h"

// constructor 
FFTProcessor::FFTProcessor():
    // juce fft initialized by fft frame length
    fft(fftOrder),
    // fftsize + 1 to work around juce's symmetrical windowing. overlap add must be periodic
    // set normalization to false to avoid messing with gain
    window(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)    
    {
    }

void FFTProcessor::processBlock(float* data, int numSamples)
{
    for (int i = 0; i < numSamples; ++i){
        data[i] = processSample(data[i]);
    }
}

// main function
float FFTProcessor::processSample(float sample){
    // push sample value into input fifo
    inputFifo[pos] = sample;

    // read values from output fifo. once read, clear to add new IFFT values later
    float outputSample = outputFifo[pos];
    outputFifo[pos] = 0.0f;

    // incrementing through samples (FIFO index), once fft bin size is reached, reset
    pos +=1;
    if(pos == fftSize){
        pos = 0;
    }

    // process FFT for each hop block
    count +=1;
    if(count == hopSize){
        count = 0;
        processFrame();
    }

    return outputSample;
}

void FFTProcessor::processFrame(){
    // copy contents of input FIFO into fft array
    const float* inputPtr = inputFifo.data();
    float* fftPtr = fftData.data();

    std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos)* sizeof(float));
    if (pos > 0){
        std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
    }

    // pre windowing, prevent spectral leakage
    window.multiplyWithWindowingTable(fftPtr, fftSize);

    // perform fft if parameter is checked
    {
        // time to frequency, true to only check positive frequencies 
        fft.performRealOnlyForwardTransform(fftPtr, true); 
        // processing 
        processSpectrum(fftPtr, numBins);
        // inverse fft, frequency back to time
        fft.performRealOnlyInverseTransform(fftPtr);
    }

    // post window 
    window.multiplyWithWindowingTable(fftPtr, fftSize);

    for(int i = 0; i < fftSize; ++i){
        // reduce gain altering produced by 75% hann overlap add
        fftPtr[i] *=windowCorrection;
    }

    // potentially optimizable with FloatVectorOperations class, idrk

    // add in results from IFFT to output buffer
    for(int i = 0; i < pos; ++i){
        outputFifo[i] += fftData[i + fftSize - pos];
    }
    for(int i = 0; i < fftSize - pos; ++i){
        outputFifo[i + pos] += fftData[i];
    }
}

void FFTProcessor::processSpectrum(float* data, int numBins){
    // frequency domain has complex numbers, easier than dealing with interleaved real and imaginary
    auto* cdata = reinterpret_cast<std::complex<float>*>(data);

    for(int i = 0; i < numBins; ++i){
        // deal with real and imaginary as magnitute and phase
        float magnitute = std::abs(cdata[i]);
        float phase = std::arg(cdata[i]);

        // spectral processing algorithm here
        

        // convert interleaved values back to complex number
        cdata[i] = std::polar(magnitute, phase);
    }
}

// resets fft, fills with zeros
void FFTProcessor::reset(){
    count = 0;
    pos = 0;
    std::fill(inputFifo.begin(),inputFifo.end(),0.0f);
    std::fill(outputFifo.begin(),outputFifo.end(),0.0f);
}

