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

AudioOutputPHP::AudioOutputPHP(Php::Value callbacks){
	startMethod = callbacks["start"];
	stopMethod = callbacks["stop"];
	configureMethod = callbacks["configure"];
	getLevelMethod = callbacks["get_level"];
}

void AudioOutputPHP::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels){
	configureMethod((int32_t)sampleRate, (int32_t)bitsPerSample, (int32_t)channels);
}

void AudioOutputPHP::Start(){
	if(running)
		return;
	running = true;
	startMethod();
}

void AudioOutputPHP::Stop(){
	if(!running)
		return;
	stopMethod();
	running = false;
}

bool AudioOutputPHP::IsPlaying(){
	return running;
}

float AudioOutputPHP::GetLevel(){
	return (double)getLevelMethod();
}

Php::Value AudioOutputPHP::readFrames() {
	unsigned char* buf;
	InvokeCallback(buf, 960*2);
	return buf;
}

/*
AudioOutputPHP::~AudioOutputPHP(){
	JNIEnv* env=NULL;
	bool didAttach=false;
	sharedJVM->GetEnv((void**) &env, JNI_VERSION_1_6);
	if(!env){
		sharedJVM->AttachCurrentThread(&env, NULL);
		didAttach=true;
	}

	env->CallVoidMethod(javaObject, releaseMethod);
	env->DeleteGlobalRef(javaObject);
	javaObject=NULL;

	if(didAttach){
		sharedJVM->DetachCurrentThread();
	}
}
*/
