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
/*
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>*/
#include <phpcpp.h>
#include <stdio.h>
#include "libtgvoip/threading.h"
#include <queue>

#define AUDIO_STATE_NONE -1
#define AUDIO_STATE_CREATED 0
#define AUDIO_STATE_CONFIGURED 1
#define AUDIO_STATE_RUNNING 2

#define CALL_STATE_NONE -1
#define CALL_STATE_REQUESTED 0
#define CALL_STATE_INCOMING 1
#define CALL_STATE_ACCEPTED 2
#define CALL_STATE_CONFIRMED 3
#define CALL_STATE_READY 4
#define CALL_STATE_ENDED 5

#ifndef TGVOIP_USE_CALLBACK_AUDIO_IO
#define TGVOIP_USE_CALLBACK_AUDIO_IO
#endif

using namespace tgvoip;
using namespace tgvoip::audio;

namespace tgvoip
{
namespace audio
{
class AudioInputModule;
class AudioOutputModule;
}
}
class VoIP : public Php::Base
{
public:
    void __construct(Php::Parameters &params);
    void initVoIPController();
    Php::Value discard(Php::Parameters &params);
    Php::Value accept();
    void deinitVoIPController();
    void __wakeup();
    Php::Value __sleep();

    Php::Value startTheMagic();

    void recvAudioFrame(int16_t* data, size_t size);
    void sendAudioFrame(int16_t* data, size_t size);

    Php::Value getVisualization();
    void setVisualization(Php::Parameters &params);

    Php::Value play(Php::Parameters &params);
    Php::Value playOnHold(Php::Parameters &params);
    Php::Value setOutputFile(Php::Parameters &params);
    Php::Value unsetOutputFile();

    Php::Value getVersion();
    Php::Value getPreferredRelayID();
    Php::Value getLastError();
    Php::Value getStats();
    Php::Value getDebugLog();
    Php::Value getDebugString();
    Php::Value getSignalBarsCount();
    Php::Value getPeerCapabilities();
    void requestCallUpgrade();
    void sendGroupCallKey(Php::Parameters &params);
    Php::Value debugCtl(Php::Parameters &params);
    Php::Value setBitrate(Php::Parameters &params);

    Php::Value setMicMute(Php::Parameters &params);
    Php::Value setMadeline(Php::Parameters &params);

    Php::Value getProtocol();
    Php::Value getState();
    Php::Value isPlaying();
    Php::Value isDestroyed();
    Php::Value whenCreated();
    Php::Value isCreator();
    Php::Value getOtherID();
    Php::Value getCallID();
    Php::Value getCallState();
    Php::Value close();


    int state = STATE_CREATED;

    bool playing = false;
    bool destroyed = false;

    void parseConfig();
    void parseProxyConfig();
    int otherID;

private:
    std::queue<FILE *> inputFiles;
    std::queue<FILE *> holdFiles;
    Mutex inputMutex;

    FILE *outputFile;
    Mutex outputMutex;

    size_t readInput;
    size_t readOutput;
    int callState = CALL_STATE_NONE;
    VoIPController *inst;
};

#endif
