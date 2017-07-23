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

using namespace tgvoip;
using namespace tgvoip::audio;

AudioInputModule::AudioInputModule(std::string deviceID, VoIPController *controller)
{
    wrapper = (VoIP *)(controller->implData);
    wrapper->in = this;
    wrapper->inputState = AUDIO_STATE_CREATED;
}

AudioInputModule::~AudioInputModule()
{
    wrapper->inputState = AUDIO_STATE_NONE;
    wrapper->in = NULL;

    if (senderThread)
    {
        LOGD("before join senderThread");
        join_thread(senderThread);
    }
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

void *AudioInputModule::StartSenderThread(void *input)
{
    ((AudioInputModule *)input)->RunSenderThread();
    return NULL;
}

void AudioInputModule::RunSenderThread()
{
    unsigned char *data = (unsigned char *)malloc(inputCSamplesSize);
    size_t read;
    double time = VoIPController::GetCurrentTime();
    double sleeptime;
    while (wrapper->inputState == AUDIO_STATE_RUNNING)
    {
        lock_mutex(wrapper->inputMutex);
        while (!wrapper->configuringInput && wrapper->inputState == AUDIO_STATE_RUNNING)
        {
            if ((sleeptime = (inputWritePeriodSec - (VoIPController::GetCurrentTime() - time)) * 1000000.0) < 0)
            {
                LOGE("Sender: I'm late!");
            }
            else
            {
                usleep(sleeptime);
            }
            time = VoIPController::GetCurrentTime();

            if (!wrapper->inputFiles.empty())
            {
                if ((read = fread(data, sizeof(unsigned char), inputSamplesSize, wrapper->inputFiles.front())) != inputCSamplesSize)
                {
                    fclose(wrapper->inputFiles.front());
                    wrapper->inputFiles.pop();
                    memset(data + (read % inputCSamplesSize), 0, inputCSamplesSize - (read % inputCSamplesSize));
                }
                wrapper->playing = true;
            }
            else
            {
                wrapper->playing = false;
                if (wrapper->holdFiles.empty())
                {
                    memset(data, 0, inputCSamplesSize);
                }
                else
                {
                    if ((read = fread(data, sizeof(unsigned char), inputSamplesSize, wrapper->holdFiles.front())) != inputCSamplesSize)
                    {
                        fseek(wrapper->holdFiles.front(), 0, SEEK_SET);
                        wrapper->holdFiles.push(wrapper->holdFiles.front());
                        wrapper->holdFiles.pop();
                        memset(data + (read % inputCSamplesSize), 0, inputCSamplesSize - (read % inputCSamplesSize));
                    }
                }
            }
            InvokeCallback(data, inputCSamplesSize);
        }
        unlock_mutex(wrapper->inputMutex);
    }
    free(data);
}

void AudioInputModule::EnumerateDevices(std::vector<AudioInputDevice> &devs)
{
}
