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
  void discard(Php::Parameters &params);
  void accept();
  void deinitVoIPController();
  void __wakeup();
  void __sleep();

  void startTheMagic();

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
  void debugCtl(Php::Parameters &params);

  void setOutputLevel(Php::Parameters &params);
  void setMicMute(Php::Parameters &params);

  Php::Value getProtocol();
  Php::Value getState();
  Php::Value getOutputState();
  Php::Value getInputState();
  Php::Value getOutputParams();
  Php::Value getInputParams();
  Php::Value isPlaying();
  Php::Value isDestroyed();
  Php::Value whenCreated();
  Php::Value isCreator();
  Php::Value getOtherID();
  Php::Value getCallID();
  Php::Value getCallState();
  void setCallState(Php::Parameters &params);

  AudioInputModule *in;
  AudioOutputModule *out;

  int state = STATE_CREATED;
  int inputState = AUDIO_STATE_NONE;
  int outputState = AUDIO_STATE_NONE;

  bool playing = false;
  bool destroyed = false;

  void parseConfig();
  void parseProxyConfig();
private:
  int callState = CALL_STATE_NONE;
  VoIPController *inst;
};

#endif
