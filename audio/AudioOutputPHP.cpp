//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//


#include "AudioOutputPHP.h"
#include <stdio.h>
#include "../libtgvoip/logging.h"

using namespace tgvoip;
using namespace tgvoip::audio;

AudioOutputPHP::AudioOutputPHP(void* controller){
	wrapper = (VoIP *)((VoIPController*)controller)->implData;
}
AudioOutputPHP::~AudioOutputPHP(){
}


void AudioOutputPHP::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels){
	wrapper->configureAudioOutput((int32_t)sampleRate, (int32_t)bitsPerSample, (int32_t)channels);
}

void AudioOutputPHP::Start(){
	if(running)
		return;
	running = true;
	wrapper->startOutput();
}

void AudioOutputPHP::Stop(){
	if(!running)
		return;
	wrapper->stopOutput();
	running = false;
}

bool AudioOutputPHP::IsPlaying(){
	return running;
}

float AudioOutputPHP::GetLevel(){
	return wrapper->getOutputLevel();
}

unsigned char* AudioOutputPHP::readFrames() {
	unsigned char* buf = (unsigned char *) malloc(960*2);
	InvokeCallback(buf, 960*2);
	return buf;
}
