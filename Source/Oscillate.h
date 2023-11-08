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

    // Method to oscillate a single buffer
    std::vector<short> oscillateBuffer(const std::vector<short>& buffer) {
        std::vector<short> outputSamples(buffer.size(), 0);
        // ... (processing as before) ...
        return outputSamples;
    }

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
};
