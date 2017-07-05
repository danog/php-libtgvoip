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


AudioInputModule::AudioInputModule(std::string deviceID, void* controller){
	wrapper = (VoIP *)((VoIPController *)controller)->implData;
}


AudioInputModule::~AudioInputModule(){

}


void AudioInputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels) {
	wrapper->configureAudioInput(sampleRate, bitsPerSample, channels);
}

void AudioInputModule::Start(){
	if(running)
		return;
	running = true;
	wrapper->startInput();
}

void AudioInputModule::Stop(){
	wrapper->stopInput();
	running = false;
}
bool AudioInputModule::writeFrames(unsigned char* data){
	if (running) {
		LOGE("STARTED");
		InvokeCallback(data, (size_t)960*2);
		return true;
	} else {
		LOGE("NOT STARTED");
		return false;
	}/*
	if (!running) {
		return false;
	}*/
}

void AudioInputModule::EnumerateDevices(std::vector<AudioInputDevice>& devs) {
}
