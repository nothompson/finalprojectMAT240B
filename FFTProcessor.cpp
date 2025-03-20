#include "FFTProcessor.h"

// constructor 
FFTProcessor::FFTProcessor():
    // juce fft initialized by fft frame length
    fft(fftOrder),
    // fftsize + 1 to work around juce's symmetrical windowing. overlap add must be periodic
    // set normalization to false to avoid messing with gain
    window(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)    
    {
        previousPhases.resize(numBins, 0.0f);
        phaseAccum.resize(numBins, 0.0f);
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

void FFTProcessor::processSpectrum(float* data, int numBins)
{
    // frequency domain has complex numbers, easier than dealing with interleaved real and imaginary
    auto* cdata = reinterpret_cast<std::complex<float>*>(data);
    
    // helpful constant
    const float twoPi = juce::MathConstants<float>::twoPi;
    
    // phase reset parameters
    // constexpr float phaseResetThreshold = 0.2f;  // turned into parameter
    // constexpr float phaseResetLerpFactor = 0.5f;   // turned into parameter
    
    // make temporary buffers for analysis
    std::vector<float> magnitudes(numBins, 0.0f);
    std::vector<float> phases(numBins, 0.0f);
    
    // our "expected" phase of the next frame based on our fft and hop windows 
    float expectedPhaseAdvance = twoPi * hopSize / static_cast<float>(fftSize);
    
    // compute magnitudes, phases, and update phase accumulators.
    for (int i = 0; i < numBins; ++i)
    {
        // absolute value to get magnitudes 
        float mag = std::abs(cdata[i]);
        // arg is phase angle of complex number 
        float ph = std::arg(cdata[i]);

        // each mag and phase
        magnitudes[i] = mag;
        phases[i] = ph;
        
        // "current" (expected) frame compared to last frame
        float expectedPhase = i * expectedPhaseAdvance;
        float deltaPhase = ph - previousPhases[i] - expectedPhase;
        
        // unwrap phases from -pi - pi, get true frequency from phase shift
        while (deltaPhase > juce::MathConstants<float>::pi)
            deltaPhase -= twoPi;
        while (deltaPhase < -juce::MathConstants<float>::pi)
            deltaPhase += twoPi;
        
        // getting fundamental frequency of frame 
        float instantFreq = expectedPhase + (deltaPhase / hopSize);

        // synthesis stage phase adding phase shift to avoid discontinuties and scaling to new pitch
        float newPhase = phaseAccum[i] + instantFreq * hopSize * pitchShiftFactor;
        
        // ensure phase doesn't drift off
        float phaseDiff = std::abs(newPhase - ph);
        if (phaseDiff > phaseResetThreshold)
        {
            // gradually reinitialize the phase accumulator toward the current phase.
            newPhase = phaseAccum[i] + phaseResetLerpFactor * (ph - phaseAccum[i]);
        }
        phaseAccum[i] = newPhase;
        previousPhases[i] = ph;
    }
    
    // peak detection, integer bins 
    // each "peak" has an associated position in the bin, phase, and magnitude
    struct Peak { int index; float phase; float mag; };
    std::vector<Peak> peaks;
    // const float peakThreshold = 0.1f;  // turned into parameter
    for (int i = 1; i < numBins - 1; ++i)
    {
        if (magnitudes[i] > magnitudes[i - 1] &&
            magnitudes[i] >= magnitudes[i + 1] &&
            magnitudes[i] > peakThreshold)
        {
            // append to peak vector if magnitudes are high enough
            peaks.push_back({ i, phaseAccum[i], magnitudes[i] });
        }
    }
    
    // resynthesis 
    // create spectrum, initialize with zeroes
    std::vector<std::complex<float>> peakSpectrum(numBins, std::complex<float>(0.0f, 0.0f));
    // reference peak container 
    for (auto& peak : peaks)
    {
        // map peaks to new bin (fractional before lerp)
        float newIndexFloat = peak.index * pitchShiftFactor;
        int lowerIndex = static_cast<int>(std::floor(newIndexFloat));
        float frac = newIndexFloat - lowerIndex;
        
        // reconstruct our data including both magnitude and phase
        std::complex<float> peakValue = std::polar(peak.mag, peak.phase);
        
        // lerping to distribute energy to nearby bins. can cause inharmonics, maybe look into cubic or other interpolation methods
        if (lowerIndex < numBins)
            peakSpectrum[lowerIndex] += peakValue * (1.0f - frac);
        if (lowerIndex + 1 < numBins)
            peakSpectrum[lowerIndex + 1] += peakValue * frac;
    }
    
    // get residual spectrum, not peak based. try and maintain harmonics and noise of original spectrum
    std::vector<std::complex<float>> residualSpectrum(numBins, std::complex<float>(0.0f, 0.0f));
    for (int i = 0; i < numBins; ++i)
    {
        // use original FFT bin value.
        std::complex<float> origValue = cdata[i];
        float newIndexFloat = i * pitchShiftFactor;
        int lowerIndex = static_cast<int>(std::floor(newIndexFloat));
        float frac = newIndexFloat - lowerIndex;
        
        // lerp to to integer bins
        if (lowerIndex < numBins)
            residualSpectrum[lowerIndex] += origValue * (1.0f - frac);
        if (lowerIndex + 1 < numBins)
            residualSpectrum[lowerIndex + 1] += origValue * frac;
    }
    
    // blend together the peaks and the spectral information
    // constexpr float nonPeakBlendFactor = 0.2f; // made into parameter
    std::vector<std::complex<float>> newSpectrum(numBins, std::complex<float>(0.0f, 0.0f));
    for (int i = 0; i < numBins; ++i)
    {
        // lerp smoothing for blending two spectrums
        newSpectrum[i] = (1.0f - nonPeakBlendFactor) * peakSpectrum[i] +
                         nonPeakBlendFactor * residualSpectrum[i];
    }
    
    // re synthesize and write back to cdata
    for (int i = 0; i < numBins; ++i)
    {
        cdata[i] = newSpectrum[i];
    }
}

// resets fft, fills with zeros
void FFTProcessor::reset(){
    count = 0;
    pos = 0;
    std::fill(inputFifo.begin(),inputFifo.end(),0.0f);
    std::fill(outputFifo.begin(),outputFifo.end(),0.0f);
}

