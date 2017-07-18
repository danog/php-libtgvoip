//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#include "AudioInputModule.h"
#include <stdio.h>
#include "../libtgvoip/logging.h"
#include <queue>
#include <unistd.h>
#include <phpcpp.h>

using namespace tgvoip;
using namespace tgvoip::audio;

AudioInputModule::AudioInputModule(std::string deviceID, VoIPController *controller)
{
	wrapper = (VoIP *)(controller->implData);
	wrapper->in = this;
	wrapper->inputState = AUDIO_STATE_CREATED;
    init_mutex(inputMutex);
    configuringInput = false;
}

AudioInputModule::~AudioInputModule()
{
	wrapper->inputState = AUDIO_STATE_NONE;
	wrapper->in = NULL;

	LOGD("before join senderThread");
	join_thread(senderThread);

	while (holdFiles.size()) {
        fclose(holdFiles.front());
        holdFiles.pop();
    }
	while (inputFiles.size()) {
        fclose(inputFiles.front());
        inputFiles.pop();
    }

    free_mutex(inputMutex);
}

void AudioInputModule::Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels)
{
	inputSampleNumber = 960;
	inputSampleRate = sampleRate;
	inputBitsPerSample = bitsPerSample;
	inputChannels = channels;
	inputSamplePeriod = 1.0 / sampleRate * 1000000;
	inputWritePeriod = 1.0 / sampleRate * inputSampleNumber * 1000000;
	inputSamplePeriodSec = 1.0 / sampleRate;
	inputWritePeriodSec = 1.0 / sampleRate * inputSampleNumber;
	inputSamplesSize = inputSampleNumber * inputChannels * inputBitsPerSample / 8;
	inputCSamplesSize = inputSampleNumber * inputChannels * inputBitsPerSample / 8 * sizeof(unsigned char);

	wrapper->inputState = AUDIO_STATE_CONFIGURED;
}

bool AudioInputModule::play(const char *file) {
    FILE *tmp = fopen(file, "rb");

    if (tmp == NULL) {
        throw Php::Exception("Could not open file!");
        return false;
    }

    configuringInput = true;
    lock_mutex(inputMutex);
    inputFiles.push(tmp);
    configuringInput = false;
    unlock_mutex(inputMutex);

    return true;
}

bool AudioInputModule::playOnHold(Php::Parameters &params) {
    configuringInput = true;
    FILE *tmp = NULL;

    lock_mutex(inputMutex);
    while (holdFiles.size()) {
        fclose(holdFiles.front());
        holdFiles.pop();
    }
    for (int i = 0; i < params[0].size(); i++) {
        tmp = fopen(params[0][i], "rb");
        if (tmp == NULL) {
            throw Php::Exception("Could not open file!");
            configuringInput = false;
            unlock_mutex(inputMutex);
            return false;
        }
        holdFiles.push(tmp);
    }
    configuringInput = false;
    unlock_mutex(inputMutex);
    return true;
}


void AudioInputModule::Start()
{
	if (wrapper->inputState == AUDIO_STATE_RUNNING)
		return;
	wrapper->inputState = AUDIO_STATE_RUNNING;
	
	LOGE("STARTING SENDER THREAD");
	start_thread(senderThread, StartSenderThread, this);
	set_thread_priority(senderThread, get_thread_max_priority());
	set_thread_name(senderThread, "voip-sender");

}

void AudioInputModule::Stop()
{
	if (wrapper->inputState != AUDIO_STATE_RUNNING)
		return;
	wrapper->inputState = AUDIO_STATE_CONFIGURED;
}


void* AudioInputModule::StartSenderThread(void* input){
	((AudioInputModule*)input)->RunSenderThread();
	return NULL;
}

void AudioInputModule::RunSenderThread() {
	unsigned char *data = (unsigned char *) malloc(inputCSamplesSize);
	size_t read;
	double time = VoIPController::GetCurrentTime();
	double sleeptime;
	while (wrapper->inputState == AUDIO_STATE_RUNNING) {
		lock_mutex(inputMutex);
		while (!configuringInput && wrapper->inputState == AUDIO_STATE_RUNNING) {
			if ((sleeptime = (inputWritePeriodSec - (VoIPController::GetCurrentTime() - time))*1000000.0) < 0) {
				LOGE("Sender: I'm late!");
			} else {
				usleep(sleeptime);
			}
			time = VoIPController::GetCurrentTime();

			if (!inputFiles.empty()) {
				if ((read = fread(data, sizeof(unsigned char), inputSamplesSize, inputFiles.front())) != inputCSamplesSize) {
					fclose(inputFiles.front());
					inputFiles.pop();
					memset(data + (read % inputCSamplesSize), 0, inputCSamplesSize - (read % inputCSamplesSize));
				}
				wrapper->playing = true;
			} else {
				wrapper->playing = false;
				if (holdFiles.empty()) {
					memset(data, 0, inputCSamplesSize);
				} else {
					if ((read = fread(data, sizeof(unsigned char), inputSamplesSize, holdFiles.front())) != inputCSamplesSize) {
						fseek(holdFiles.front(), 0, SEEK_SET);
						holdFiles.push(holdFiles.front());
						holdFiles.pop();
						memset(data + (read % inputCSamplesSize), 0, inputCSamplesSize - (read % inputCSamplesSize));
					}
				}
			}
			InvokeCallback(data, inputCSamplesSize);		
		}
		unlock_mutex(inputMutex);
	}
	free(data);
}

void AudioInputModule::EnumerateDevices(std::vector<AudioInputDevice> &devs)
{
}
