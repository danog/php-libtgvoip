//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#include "AudioOutputModule.h"
#include <stdio.h>
#include "../libtgvoip/logging.h"

using namespace tgvoip;
using namespace tgvoip::audio;

AudioOutputModule::AudioOutputModule(std::string deviceID, void *controller)
{
	wrapper = (VoIP *)((VoIPController *)controller)->implData;
	wrapper->out = this;
	outputState = AUDIO_STATE_CREATED;
}
AudioOutputModule::~AudioOutputModule()
{
	outputState = AUDIO_STATE_NONE;
	wrapper->out = NULL;

	LOGD("before join receiverThread");
	join_thread(receiverThread);
	
}


void AudioOutputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels)
{
	outputSampleNumber = 960;
	outputSampleRate = sampleRate;
	outputBitsPerSample = bitsPerSample;
	outputChannels = channels;
	outputSamplePeriod = 1.0 / sampleRate * 1000000;
	outputWritePeriod = 1.0 / sampleRate * outputSampleNumber * 1000000;
	outputSamplePeriodSec = 1.0 / sampleRate;
	outputWritePeriodSec = 1.0 / sampleRate * outputSampleNumber;
	outputSamplesSize = outputSampleNumber * outputChannels * outputBitsPerSample / 8;
	outputCSamplesSize = outputSampleNumber * outputChannels * outputBitsPerSample / 8 * sizeof(unsigned char);

	outputState = AUDIO_STATE_CONFIGURED;
}

void AudioOutputModule::Start()
{
	if (outputState == AUDIO_STATE_RUNNING)
		return;
	outputState = AUDIO_STATE_RUNNING;
	

	LOGE("STARTING RECEIVER THREAD");
	start_thread(receiverThread, StartReceiverThread, this);
	set_thread_priority(receiverThread, get_thread_max_priority());
	set_thread_name(receiverThread, "voip-receiver");
	
}

void AudioOutputModule::Stop()
{
	if (outputState != AUDIO_STATE_RUNNING)
		return;
	outputState = AUDIO_STATE_CONFIGURED;
}

bool AudioOutputModule::IsPlaying()
{
	return outputState == AUDIO_STATE_RUNNING;
}

float AudioOutputModule::GetLevel()
{
	return outputLevel;
}


void* AudioOutputModule::StartReceiverThread(void* output){
	((AudioOutputModule*)output)->RunReceiverThread();
	return NULL;
}

void AudioOutputModule::RunReceiverThread() {
	unsigned char *data = (unsigned char *) malloc(outputCSamplesSize);
	double time = VoIPController::GetCurrentTime();
	while (outputState == AUDIO_STATE_RUNNING) {
		lock_mutex(wrapper->outputMutex);
		fputs(wrapper->configuringOutput ? "configuringOutput = true\n" : "configuringOutput = false\n", stdout);
		fputs(wrapper->configuringInput ? "configuringInput = true\n" : "configuringInput = false\n", stdout);

		if(wrapper->configuringOutput) sleep(1);
		while (!wrapper->configuringOutput && outputState == AUDIO_STATE_RUNNING) {
			usleep((outputWritePeriodSec - (VoIPController::GetCurrentTime() - time))*1000000.0);
			time = VoIPController::GetCurrentTime();

			InvokeCallback(data, outputCSamplesSize);
			if (wrapper->outputFile != NULL) {
				if (fwrite(data, sizeof(unsigned char), outputSamplesSize, wrapper->outputFile) != outputCSamplesSize) {
					LOGE("COULD NOT WRITE DATA TO FILE");
				} else LOGE("WROTE");
			}else LOGE("NO FILE");
		}
		unlock_mutex(wrapper->outputMutex);
	}
	free(data);
}

void AudioOutputModule::EnumerateDevices(std::vector<AudioOutputDevice> &devs)
{
}
