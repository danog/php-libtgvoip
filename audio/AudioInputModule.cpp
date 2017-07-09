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
	wrapper->inputCreated = true;
}

AudioInputModule::~AudioInputModule()
{
	wrapper->inputConfigured = false;
	wrapper->inputCreated = false;
}

void AudioInputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels)
{
	wrapper->inputSampleNumber = 960;
	wrapper->inputSampleRate = sampleRate;
	wrapper->inputBitsPerSample = bitsPerSample;
	wrapper->inputChannels = channels;
	wrapper->inputSamplePeriod = 1 / sampleRate * 1000000;
	wrapper->inputWritePeriod = 1 / sampleRate * wrapper->inputSampleNumber * 1000000;
	wrapper->inputSampleSize = wrapper->inputSampleNumber * wrapper->inputChannels * wrapper->inputBitsPerSample / 8;

	wrapper->inputConfigured = true;
}

void AudioInputModule::Start()
{
	if (wrapper->inputRunning)
		return;
	wrapper->inputRunning = true;
}

void AudioInputModule::Stop()
{
	wrapper->inputRunning = false;
}
bool AudioInputModule::writeSamples(unsigned char *data)
{
	if (wrapper->inputRunning)
	{
		LOGE("STARTED");
		InvokeCallback(data, wrapper->inputSampleSize);
		return true;
	}
	else
	{
		LOGE("NOT STARTED");
		return false;
	}
}

void AudioInputModule::EnumerateDevices(std::vector<AudioInputDevice> &devs)
{
}
