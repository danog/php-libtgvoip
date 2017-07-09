/*
Copyright 2016-2017 Daniil Gentili
(https://daniil.it)
This file is part of php-libtgvoip.
php-libtgvoip is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
The PWRTelegram API is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.
You should have received a copy of the GNU General Public License along with php-libtgvoip.
If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PHPLIBTGVOIP_H
#define PHPLIBTGVOIP_H

#include "libtgvoip/VoIPController.h"

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <phpcpp.h>

#define AUDIO_STATE_NONE -1
#define AUDIO_STATE_CREATED 0
#define AUDIO_STATE_CONFIGURED 1
#define AUDIO_STATE_RUNNING 2

using namespace tgvoip;
using namespace tgvoip::audio;

class VoIP : public Php::Base
{
  public:
    void __construct();

    void __destruct();

    void start();

    void connect();

    void setEncryptionKey(Php::Parameters &params);

    void setRemoteEndpoints(Php::Parameters &params);

    void release();

    Php::Value writeSamples(Php::Parameters &params);

    Php::Value readSamples();

    Php::Value getDebugString();

    void setNetworkType(Php::Parameters &params);

    void setMicMute(Php::Parameters &params);
    // jdouble recvTimeout, jdouble initTimeout, jint dataSavingMode, jboolean enableAEC, jboolean enableNS, jboolean enableAGC, jstring logFilePath
    void setConfig(Php::Parameters &params);

    void debugCtl(Php::Parameters &params);

    Php::Value getVersion();

    Php::Value getPreferredRelayID();

    Php::Value getLastError();

    Php::Value getStats();

    void setSharedConfig(Php::Parameters &params);

    void setProxy(Php::Parameters &params);

    Php::Value getDebugLog();

    void updateConnectionState(int state);

    void setOutputLevel(Php::Parameters &params);

    Php::Value getState();

    Php::Value getOutputState();
    Php::Value getInputState();

    Php::Value getOutputParams();
    Php::Value getInputParams();
    /*
    Php::Value madeline;
    Php::Value current_call;
    */

    int inputBitsPerSample;
    int inputSampleRate;
    int inputChannels;
    int inputSamplePeriod;
    int inputWritePeriod;
    int inputSampleNumber;
    int inputSamplesSize;
    int inputState = AUDIO_STATE_NONE;

    int outputBitsPerSample;
    int outputSampleRate;
    int outputChannels;
    int outputSamplePeriod;
    int outputWritePeriod;
    int outputSampleNumber;
    int outputSamplesSize;
    float outputLevel = 0.0;
    int outputState = AUDIO_STATE_NONE;

    int state = STATE_CREATED;

  private:
    VoIPController *inst;
};

#endif
