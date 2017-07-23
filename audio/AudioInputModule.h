//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef LIBTGVOIP_AUDIOINPUTPHP_H
#define LIBTGVOIP_AUDIOINPUTPHP_H

#include "../libtgvoip/audio/AudioInput.h"
#include "../libtgvoip/threading.h"
#include "../libtgvoip/VoIPController.h"
#include "../main.h"

using namespace tgvoip;
using namespace tgvoip::audio;


namespace tgvoip {
namespace audio {
class AudioInputModule : public AudioInput {

public:
    AudioInputModule(std::string deviceID, VoIPController *controller);
    virtual ~AudioInputModule();

    virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
    virtual void Start();
    virtual void Stop();
    static void EnumerateDevices(std::vector<AudioInputDevice>& devs);

    static void* StartSenderThread(void* input);
    void RunSenderThread();

    int inputBitsPerSample;
    int inputSampleRate;
    int inputChannels;
    int inputSamplePeriod;
    int inputWritePeriod;
    double inputSamplePeriodSec;
    double inputWritePeriodSec;
    int inputSampleNumber;
    int inputSamplesSize;
    size_t inputCSamplesSize;



private:


    tgvoip_thread_t senderThread;
    VoIP *wrapper;
};
}
}

#endif //LIBTGVOIP_AUDIOINPUTPHP_H
