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


namespace tgvoip{ namespace audio{
class AudioInputModule : public AudioInput{

public:
	AudioInputModule(std::string deviceID, void *controller);
	virtual ~AudioInputModule();

	virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
	virtual void Start();
	virtual void Stop();
	bool writeSamples(unsigned char *data);
	static void EnumerateDevices(std::vector<AudioInputDevice>& devs);
/*
	static void* StartSenderThread(void* input);
	void RunSenderThread();
*/
    int inputBitsPerSample;
    int inputSampleRate;
    int inputChannels;
    int inputSamplePeriod;
    int inputWritePeriod;
    int inputSamplePeriodSec;
    int inputWritePeriodSec;
    int inputSampleNumber;
    int inputSamplesSize;
    size_t inputCSamplesSize;
    int inputState = AUDIO_STATE_NONE;

private:


//	tgvoip_thread_t senderThread;
	VoIP *wrapper;
};
}}

#endif //LIBTGVOIP_AUDIOINPUTPHP_H
