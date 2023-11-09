#ifndef OSCILLATE_H  // If OSCILLATE_H is not defined
#define OSCILLATE_H  // Define OSCILLATE_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <iostream>

class SampleOscillator {
private:
    static const int sampleRate = 44100;
    int snippetDuration;
    int zoneDuration;
    int overlapRatio;
    int overlapSamples;
    void updateOverlapSamples() {
        overlapSamples = overlapRatio * zoneDuration / 100;
    }
    

public:
    SampleOscillator()
    : snippetDuration(sampleRate / 220), // Initialize tuned for 220Hz
      zoneDuration(sampleRate / 20), // Initialize for 0.05 seconds
      overlapRatio(68) // Initialize to 68% overlap
    {
        updateOverlapSamples();
    }

    // Method to update snippet duration, zone duration, and overlap ratio
    void updateParameters(int frequencyHz, double durationInSeconds, int newOverlapRatio) {
        snippetDuration = sampleRate / frequencyHz;
        zoneDuration = static_cast<int>(durationInSeconds * sampleRate);
        overlapRatio = newOverlapRatio;
        updateOverlapSamples();
    }

    // Method to process and mix four buffers
    void processAndMixBuffers(const std::vector<short>& buffer1,
                              const std::vector<short>& buffer2,
                              const std::vector<short>& buffer3,
                              const std::vector<short>& buffer4,
                              std::vector<short>& mixBuffer) {
        
        // Process each buffer
        std::vector<short> oscBuffer1 = oscillateBuffer(buffer1);
        std::vector<short> oscBuffer2 = oscillateBuffer(buffer2);
        std::vector<short> oscBuffer3 = oscillateBuffer(buffer3);
        std::vector<short> oscBuffer4 = oscillateBuffer(buffer4);

        // Ensure the mixBuffer is clear and has the proper size
        mixBuffer.clear();
        mixBuffer.resize(oscBuffer1.size(), 0);

        // Sum the processed buffers into mixBuffer
        for (size_t i = 0; i < mixBuffer.size(); ++i) {
            int mixedSample = 0;
            if (i < oscBuffer1.size()) mixedSample += oscBuffer1[i];
            if (i < oscBuffer2.size()) mixedSample += oscBuffer2[i];
            if (i < oscBuffer3.size()) mixedSample += oscBuffer3[i];
            if (i < oscBuffer4.size()) mixedSample += oscBuffer4[i];

            // Clipping prevention
            mixedSample = std::max(std::min(mixedSample, (int)std::numeric_limits<short>::max()), (int)std::numeric_limits<short>::min());

            mixBuffer[i] = mixedSample;
        }
    }
    /// Method to oscillate a single buffer
    std::vector<short> oscillateBuffer(const std::vector<short>& buffer) {
        std::vector<short> outputSamples(buffer.size(), 0);
        std::cout << "oscillateBuffer entered with buffer size: " << buffer.size() << std::endl;

        size_t snippetLength = snippetDuration; // Ensure this is correctly set
        size_t zoneLength = zoneDuration;       // Ensure this is correctly set

        if (zoneLength > buffer.size()) {
            std::cerr << "Error: zoneLength is greater than buffer size, adjusting zoneLength." << std::endl;
            zoneLength = buffer.size();
        }

        if (zoneLength <= overlapSamples) {
            std::cerr << "Error: overlapSamples is greater than or equal to zoneLength, adjusting overlapSamples." << std::endl;
            overlapSamples = zoneLength / 2; // Example adjustment, you may choose a different strategy
        }

        if (zoneLength - overlapSamples > buffer.size()) {
            std::cerr << "Adjusting overlapSamples to fit within buffer size." << std::endl;
            overlapSamples = buffer.size() - zoneLength; // Ensure overlapSamples is less than buffer size
        }

        for (size_t i = 0; i + zoneLength <= buffer.size(); i += zoneLength - overlapSamples) {
            std::cout << "Processing zone starting at index: " << i << std::endl;
            std::vector<short> snippet(buffer.begin() + i, buffer.begin() + std::min(i + snippetLength, buffer.size()));
            snippet.resize(zoneLength, 0); // Extend the snippet to fill the zone, be cautious with this line.

            for (size_t j = 0; j < zoneLength; ++j) {
                if (i + j < outputSamples.size()) {
                    float crossfadeFactor = 1.0;
                    if (j < overlapSamples && i != 0) {
                        crossfadeFactor = static_cast<float>(j) / overlapSamples;
                    }
                    short newVal = static_cast<short>(crossfadeFactor * snippet[j] + (1 - crossfadeFactor) * outputSamples[i + j]);
                    outputSamples[i + j] = newVal;

                    // Debug output
                    std::cout << "Processing sample " << i + j << ": crossfadeFactor = " << crossfadeFactor << ", newVal = " << newVal << std::endl;
                    std::cout << ": crossfadeFactor = " << crossfadeFactor;
                    std::cout << ", newVal = " << newVal << std::endl;
                }
            std::cout << std::flush;
            }
        }
        std::cout << "oscillateBuffer exiting." << std::endl;

        return outputSamples;
    }

};

#endif  // End of OSCILLATE_H guard
