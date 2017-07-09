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
	wrapper->outputCreated = true;
}
AudioOutputModule::~AudioOutputModule()
{
	wrapper->outputCreated = false;
	wrapper->outputConfigured = false;
}

void AudioOutputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels)
{
	wrapper->outputSampleNumber = 960;
	wrapper->outputSampleRate = sampleRate;
	wrapper->outputBitsPerSample = bitsPerSample;
	wrapper->outputChannels = channels;
	wrapper->outputSamplePeriod = 1 / sampleRate * 1000000;
	wrapper->outputWritePeriod = 1 / sampleRate * wrapper->outputSampleNumber * 1000000;
	wrapper->outputSampleSize = wrapper->outputSampleNumber * wrapper->outputChannels * wrapper->outputBitsPerSample / 8;

	wrapper->outputConfigured = true;
}

void AudioOutputModule::Start()
{
	if (wrapper->outputRunning)
		return;
	wrapper->outputRunning = true;
}

void AudioOutputModule::Stop()
{
	if (!wrapper->outputRunning)
		return;
	wrapper->outputRunning = false;
}

bool AudioOutputModule::IsPlaying()
{
	return wrapper->outputRunning;
}

float AudioOutputModule::GetLevel()
{
	return wrapper->outputLevel;
}

unsigned char *AudioOutputModule::readSamples()
{
	unsigned char *buf = (unsigned char *)emalloc(wrapper->outputSampleSize);
	InvokeCallback(buf, wrapper->outputSampleSize);
	return buf;
}

void AudioOutputModule::EnumerateDevices(std::vector<AudioOutputDevice> &devs)
{
}
