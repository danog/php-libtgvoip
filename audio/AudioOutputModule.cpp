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

AudioOutputModule::AudioOutputModule(std::string deviceID, void* controller){
	wrapper = (VoIP *)((VoIPController *)controller)->implData;
}
AudioOutputModule::~AudioOutputModule(){
}


void AudioOutputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels){
	wrapper->configureAudioOutput(sampleRate, bitsPerSample, channels);
}

void AudioOutputModule::Start(){
	if(running)
		return;
	running = true;
	wrapper->startOutput();
}

void AudioOutputModule::Stop(){
	if(!running)
		return;
	wrapper->stopOutput();
	running = false;
}

bool AudioOutputModule::IsPlaying(){
	return running;
}

float AudioOutputModule::GetLevel(){
	return wrapper->getOutputLevel();
}

unsigned char* AudioOutputModule::readFrames() {
	unsigned char* buf = (unsigned char *) malloc(960*2);
	InvokeCallback(buf, 960*2);
	return buf;
}

void AudioOutputModule::EnumerateDevices(std::vector<AudioOutputDevice>& devs) {
}
