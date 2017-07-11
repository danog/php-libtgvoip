//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#include "AudioInputModule.h"
#include <stdio.h>
#include "../libtgvoip/logging.h"

using namespace tgvoip;
using namespace tgvoip::audio;

AudioInputModule::AudioInputModule(std::string deviceID, void *controller)
{
	wrapper = (VoIP *)((VoIPController *)controller)->implData;
	wrapper->in = this;
	wrapper->inputState = AUDIO_STATE_CREATED;
}

AudioInputModule::~AudioInputModule()
{
	wrapper->inputState = AUDIO_STATE_NONE;
	wrapper->in = NULL;
}

void AudioInputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels)
{
	wrapper->inputSampleNumber = 960;
	wrapper->inputSampleRate = sampleRate;
	wrapper->inputBitsPerSample = bitsPerSample;
	wrapper->inputChannels = channels;
	wrapper->inputSamplePeriod = 1.0 / sampleRate * 1000000;
	wrapper->inputWritePeriod = 1.0 / sampleRate * wrapper->inputSampleNumber * 1000000;
	wrapper->inputSamplesSize = wrapper->inputSampleNumber * wrapper->inputChannels * wrapper->inputBitsPerSample / 8;

	wrapper->inputState = AUDIO_STATE_CONFIGURED;
}

void AudioInputModule::Start()
{
	if (wrapper->inputState == AUDIO_STATE_RUNNING)
		return;
	wrapper->inputState = AUDIO_STATE_RUNNING;
}

void AudioInputModule::Stop()
{
	if (wrapper->inputState != AUDIO_STATE_RUNNING)
		return;
	wrapper->inputState = AUDIO_STATE_CONFIGURED;
}
bool AudioInputModule::writeSamples(unsigned char *data)
{
	if (wrapper->inputState == AUDIO_STATE_RUNNING)
	{
		InvokeCallback(data, wrapper->inputSamplesSize);
		return true;
	}
	else
	{
		return false;
	}
}

void AudioInputModule::EnumerateDevices(std::vector<AudioInputDevice> &devs)
{
}
