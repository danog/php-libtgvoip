//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef LIBTGVOIP_AUDIOOUTPUTPHP_H
#define LIBTGVOIP_AUDIOOUTPUTPHP_H

#include "../libtgvoip/audio/AudioOutput.h"
#include "../libtgvoip/VoIPController.h"
#include "../main.h"
#include "../libtgvoip/threading.h"

namespace tgvoip{ namespace audio{
class AudioOutputModule : public AudioOutput{

public:

	AudioOutputModule(std::string deviceID, VoIPController* controller);
	virtual ~AudioOutputModule();
	virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
	virtual void Start();
	virtual void Stop();
	virtual bool IsPlaying() override;
	virtual float GetLevel() override;
	unsigned char * readSamples();
	static void EnumerateDevices(std::vector<AudioOutputDevice>& devs);

	static void* StartReceiverThread(void* output);
	void RunReceiverThread();

	bool unsetOutputFile();
	bool setOutputFile(const char *file);

    int outputBitsPerSample;
    int outputSampleRate;
    int outputChannels;
    int outputSamplePeriod;
    int outputWritePeriod;
    double outputSamplePeriodSec;
    double outputWritePeriodSec;
    int outputSampleNumber;
    int outputSamplesSize;
    size_t outputCSamplesSize;
    float outputLevel = 0.0;

private:
	tgvoip_thread_t receiverThread;
	VoIP *wrapper;

    FILE *outputFile;
    tgvoip_mutex_t outputMutex;
	
    bool configuringOutput;

};
}}

#endif //LIBTGVOIP_AUDIOOUTPUTPHP_H
