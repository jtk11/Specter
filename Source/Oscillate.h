#include <vector>
#include <algorithm>
#include <numeric>

class SampleOscillator {
private:
    static const int sampleRate = 44100;
    static const int snippetDuration = sampleRate / 220; // Tuned for 220Hz
    static const int zoneDuration = sampleRate / 20; // 0.05 seconds
    static const int overlapRatio = 68; // 68% overlap
    static const int overlapSamples = overlapRatio * zoneDuration / 100;

    

public:
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
        size_t snippetLength = snippetDuration;
        size_t zoneLength = zoneDuration;

        for (size_t i = 0; i + zoneLength <= buffer.size(); i += zoneLength - overlapSamples) {
            std::vector<short> snippet(buffer.begin() + i, buffer.begin() + i + snippetLength);
            snippet.resize(zoneLength, 0); // Extend the snippet to fill the zone

            for (size_t j = 0; j < zoneLength; ++j) {
                if (i + j < outputSamples.size()) {
                    if (j < overlapSamples && i != 0) {
                        // Apply crossfade with overlap
                        float crossfadeFactor = static_cast<float>(j) / overlapSamples;
                        outputSamples[i + j] = static_cast<short>(crossfadeFactor * snippet[j] + (1 - crossfadeFactor) * outputSamples[i + j]);
                    } else {
                        // No overlap, just sum up
                        outputSamples[i + j] += snippet[j];
                    }
                }
            }
        }
        return outputSamples;
    }
};
