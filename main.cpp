/*
Copyright 2016-2017 Daniil Gentili
(https://daniil.it)
This file is part of php-libtgvoip.
php-libtgvoip is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the free Software Foundation, either version 3 of the License, or (at your option) any later version.
The PWRTelegram API is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.
You should have received a copy of the GNU General Public License along with php-libtgvoip.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"
#include "audio/AudioInputModule.h"
#include "audio/AudioOutputModule.h"
#include <string.h>
#include <wchar.h>
#include <map>
#include <string>
#include <vector>
#include <queue>

#include "libtgvoip/VoIPServerConfig.h"
#include "libtgvoip/threading.h"
#include "libtgvoip/logging.h"

using namespace tgvoip;
using namespace tgvoip::audio;
using namespace std;

void VoIP::__construct(Php::Parameters &params)
{
    Php::Value self(this);
    Php::Array empty;

    self["configuration"]["endpoints"] = empty;
    self["configuration"]["shared_config"] = empty;
    self["storage"] = empty;
    self["internalStorage"] = empty;

    self["internalStorage"]["creator"] = params[0];
    otherID = (int) params[1];
    self["internalStorage"]["callID"] = params[2];
    self["madeline"] = params[3];
    callState = (int) params[4];
    self["internalStorage"]["protocol"] = params[5];

    initVoIPController();
}
void VoIP::initVoIPController() {
    in=NULL;
    out=NULL;
    inst = new VoIPController();

    outputFile=NULL;
    configuringOutput = false;
    init_mutex(outputMutex);
    init_mutex(inputMutex);
    configuringInput = false;

    inst->implData = (void *) this;
    inst->SetStateCallback([](VoIPController *controller, int state) {
        ((VoIP *)controller->implData)->state = state;
        if (state == STATE_FAILED) {
            ((VoIP *)controller->implData)->deinitVoIPController();
        }
    });
    inst->Start();
}

void VoIP::deinitVoIPController() {
    if (callState != CALL_STATE_ENDED) {
        callState = CALL_STATE_ENDED;
        delete inst;

        lock_mutex(inputMutex);
        unlock_mutex(inputMutex);
        free_mutex(inputMutex);
        lock_mutex(outputMutex);
        unlock_mutex(outputMutex);
        free_mutex(outputMutex);

        while (holdFiles.size()) {
            fclose(holdFiles.front());
            holdFiles.pop();
        }
        while (inputFiles.size()) {
            fclose(inputFiles.front());
            inputFiles.pop();
        }
        unsetOutputFile();

    }
}

Php::Value VoIP::discard(Php::Parameters &params)
{
    if (callState == CALL_STATE_ENDED) {
        return false;
    }
    Php::Value self(this);
    if (!self["configuration"]) {
        return false;
    }
    if (self["madeline"] && self["madeline"].value().instanceOf("danog\\MadelineProto\\MTProto")) {
        Php::Array reason;
        Php::Array rating;
        Php::Value debug;
        if (params.size() > 0) {
            reason = params[0];
        } else {
            reason["_"] = "phoneCallDiscardReasonDisconnect";
        }
        if (params.size() > 1) {
            rating = params[1];
        }
        if (params.size() > 2) {
            debug = params[2];
        } else debug = true;
        self["madeline"].value().call("discard_call", self["internalStorage"]["callID"].value(), reason, rating, debug);
    }
    deinitVoIPController();
    return this;
}

Php::Value VoIP::accept()
{
    if (callState != CALL_STATE_INCOMING) return false;
    callState = CALL_STATE_ACCEPTED;
    Php::Value self(this);
    if (self["madeline"].value().call("accept_call", self["internalStorage"]["callID"].value()) == false) {
        if (!self["configuration"]) {
            return false;
        }
        if (self["madeline"].value().instanceOf("danog\\MadelineProto\\MTProto")) {
            Php::Array reason;
            Php::Array rating;
            Php::Value debug;
            reason["_"] = "phoneCallDiscardReasonDisconnect";
            debug = false;
            self["madeline"].value().call("discard_call", self["internalStorage"]["callID"].value(), reason, rating, debug);
        }
        deinitVoIPController();
        return false;
    }
    return this;
}

Php::Value VoIP::close()
{
  deinitVoIPController();
  return this;
}

void VoIP::__wakeup()
{
    callState = CALL_STATE_ENDED;
}

Php::Value VoIP::__sleep()
{
    Php::Array res;
    return res;
}


Php::Value VoIP::startTheMagic()
{
    Php::Value self(this);
    if (state==STATE_WAIT_INIT_ACK) {
        if (!self["configuration"]) {
            return false;
        }
        if (self["madeline"].value().instanceOf("danog\\MadelineProto\\MTProto")) {
            Php::Array reason;
            Php::Array rating;
            Php::Value debug;
            reason["_"] = "phoneCallDiscardReasonDisconnect";
            debug = false;
            self["madeline"].value().call("discard_call", self["internalStorage"]["callID"].value(), reason, rating, debug);
        }
        deinitVoIPController();
        return false;
    }
    inst->Connect();
    self["internalStorage"]["created"] = (int64_t) time(NULL);
    callState = CALL_STATE_READY;
    return true;
}

Php::Value VoIP::whenCreated()
{

    Php::Value self(this);
    if (self["internalStorage"]["created"]) {
        return self["internalStorage"]["created"];
    }
    return false;
}
Php::Value VoIP::isCreator()
{

    Php::Value self(this);
    return self["internalStorage"]["creator"];
}
Php::Value VoIP::getOtherID()
{
    return otherID;
}
Php::Value VoIP::setMadeline(Php::Parameters &params)
{
    Php::Value self(this);
    return self["madeline"] = params[0];
}
Php::Value VoIP::getProtocol()
{
    Php::Value self(this);
    return self["internalStorage"]["protocol"];
}
Php::Value VoIP::getCallID()
{
    Php::Value self(this);
    return self["internalStorage"]["callID"];
}

Php::Value VoIP::getCallState()
{
    return callState;
}

Php::Value VoIP::getVisualization()
{
    Php::Value self(this);
    if (self["internalStorage"]["visualization"]) {
        return self["internalStorage"]["visualization"];
    }
    return false;
}
void VoIP::setVisualization(Php::Parameters &params)
{
    Php::Value self(this);
    self["internalStorage"]["visualization"] = params[0];
}

void VoIP::parseConfig() {
    Php::Value self(this);
    if (!self["configuration"]["auth_key"]) return;

    voip_config_t cfg;
    cfg.recv_timeout = (double) self["configuration"]["recv_timeout"];
    cfg.init_timeout = (double) self["configuration"]["init_timeout"];
    cfg.data_saving = (int) self["configuration"]["data_saving"];
    cfg.enableAEC = (bool) self["configuration"]["enable_AEC"];
    cfg.enableNS = (bool) self["configuration"]["enable_NS"];
    cfg.enableAGC = (bool) self["configuration"]["enable_AGC"];

    if (self["configuration"]["log_file_path"])
    {
        strncpy(cfg.logFilePath, self["configuration"]["log_file_path"], sizeof(cfg.logFilePath));
        cfg.logFilePath[sizeof(cfg.logFilePath) - 1] = 0;
    }
    else
    {
        memset(cfg.logFilePath, 0, sizeof(cfg.logFilePath));
    }

    if (self["configuration"]["stats_dump_file_path"])
    {
        strncpy(cfg.statsDumpFilePath, self["configuration"]["stats_dump_file_path"], sizeof(cfg.statsDumpFilePath));
        cfg.statsDumpFilePath[sizeof(cfg.statsDumpFilePath) - 1] = 0;
    }
    else
    {
        memset(cfg.statsDumpFilePath, 0, sizeof(cfg.statsDumpFilePath));
    }
    Php::Value shared_config = self["configuration"]["shared_config"];
    std::map<std::string, std::string> copyconfig(shared_config);
    ServerConfig::GetSharedInstance()->Update(copyconfig);
    inst->SetConfig(&cfg);

    char *key = (char *) malloc(256);
    memcpy(key, self["configuration"]["auth_key"], 256);
    inst->SetEncryptionKey(key, (bool) self["internalStorage"]["creator"]);
    free(key);

    vector<Endpoint> eps;
    Php::Value endpoints = self["configuration"]["endpoints"];
    for (int i = 0; i < endpoints.size(); i++)
    {
        string ip = endpoints[i]["ip"];
        string ipv6 = endpoints[i]["ipv6"];
        string peer_tag = endpoints[i]["peer_tag"];

        IPv4Address v4addr(ip);
        IPv6Address v6addr("::0");
        unsigned char *pTag = (unsigned char *) malloc(16);

        if (ipv6 != "")
        {
            v6addr = IPv6Address(ipv6);
        }

        if (peer_tag != "")
        {
            memcpy(pTag, peer_tag.c_str(), 16);
        }

        eps.push_back(Endpoint(endpoints[i]["id"], (int32_t)endpoints[i]["port"], v4addr, v6addr, EP_TYPE_UDP_RELAY, pTag));
        eps.push_back(Endpoint(endpoints[i]["id"], (int32_t)endpoints[i]["port"], v4addr, v6addr, EP_TYPE_TCP_RELAY, pTag));
        free(pTag);
    }

    inst->SetRemoteEndpoints(eps, (bool) self["internalStorage"]["protocol"]["udp_p2p"]);
    inst->SetNetworkType(self["configuration"]["network_type"]);
    if (self["configuration"]["proxy"]) {
        inst->SetProxy(self["configuration"]["proxy"]["protocol"], self["configuration"]["proxy"]["address"], (int32_t) self["configuration"]["proxy"]["port"], self["configuration"]["proxy"]["username"], self["configuration"]["proxy"]["password"]);
    }
}


Php::Value VoIP::unsetOutputFile() {
    if (outputFile == NULL) {
        return false;
    }

    configuringOutput = true;
    lock_mutex(outputMutex);
    fflush(outputFile);
    fclose(outputFile);
    outputFile = NULL;
    configuringOutput = false;
    unlock_mutex(outputMutex);

    return this;

}
Php::Value VoIP::setOutputFile(Php::Parameters &params) {
    configuringOutput = true;

    lock_mutex(outputMutex);
    if (outputFile != NULL) {
        fclose(outputFile);
        outputFile=NULL;
    }
    outputFile = fopen(params[0], "wb");
    if (outputFile == NULL) {
        throw Php::Exception("Could not open file!");
        configuringOutput = false;
        unlock_mutex(outputMutex);
        return false;
    }
    configuringOutput = false;
    unlock_mutex(outputMutex);
    return this;
}


Php::Value VoIP::play(Php::Parameters &params) {
    FILE *tmp = fopen(params[0], "rb");

    if (tmp == NULL) {
        throw Php::Exception("Could not open file!");
        return false;
    }

    configuringInput = true;
    lock_mutex(inputMutex);
    inputFiles.push(tmp);
    configuringInput = false;
    unlock_mutex(inputMutex);

    return this;
}
Php::Value VoIP::playOnHold(Php::Parameters &params) {
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
    return this;
}

Php::Value VoIP::setMicMute(Php::Parameters &params)
{
    inst->SetMicMute((bool) params[0]);
    return this;
}


Php::Value VoIP::debugCtl(Php::Parameters &params)
{
    inst->DebugCtl((int) params[0], (int) params[1]);
    return this;
}
Php::Value VoIP::setBitrate(Php::Parameters &params)
{
    inst->DebugCtl(1, (int) params[0]);
    return this;
}

Php::Value VoIP::getDebugLog()
{
    Php::Value data;
    string encoded = inst->GetDebugLog();
    if (!encoded.empty()) {
        data = Php::call("json_decode", encoded, true);
    }
    return data;
}

Php::Value VoIP::getVersion()
{
    return VoIPController::GetVersion();
}

Php::Value VoIP::getSignalBarsCount()
{
    return inst->GetSignalBarsCount();
}


Php::Value VoIP::getPreferredRelayID()
{
    return inst->GetPreferredRelayID();
}

Php::Value VoIP::getLastError()
{
    return inst->GetLastError();
}
Php::Value VoIP::getDebugString()
{
    char *buf = (char *) malloc(10240);
    inst->GetDebugString(buf, 10240);
    Php::Value returnvalue = buf;
    free(buf);
    return returnvalue;
}
Php::Value VoIP::getStats()
{
    voip_stats_t _stats;
    inst->GetStats(&_stats);
    Php::Value stats;
    stats["bytesSentWifi"] = (int64_t)_stats.bytesSentWifi;
    stats["bytesSentMobile"] = (int64_t)_stats.bytesSentMobile;
    stats["bytesRecvdWifi"] = (int64_t)_stats.bytesRecvdWifi;
    stats["bytesRecvdMobile"] = (int64_t)_stats.bytesRecvdMobile;
    return stats;
}


void VoIP::setOutputLevel(Php::Parameters &params) {
    out->outputLevel = (double) params[0];
}

Php::Value VoIP::getState()
{
    return state;
}

Php::Value VoIP::isPlaying()
{
    return playing;
}

Php::Value VoIP::getOutputState()
{
    return outputState;
}

Php::Value VoIP::getInputState()
{
    return inputState;
}

Php::Value VoIP::getOutputParams()
{
    Php::Value params;
    params["bitsPerSample"] = out->outputBitsPerSample;
    params["sampleRate"] = out->outputSampleRate;
    params["channels"] = out->outputChannels;
    params["samplePeriod"] = out->outputSamplePeriod;
    params["writePeriod"] = out->outputWritePeriod;
    params["sampleNumber"] = out->outputSampleNumber;
    params["samplesSize"] = out->outputSamplesSize;
    params["level"] = out->outputLevel;

    return params;

}

Php::Value VoIP::getInputParams()
{
    Php::Value params;
    params["bitsPerSample"] = in->inputBitsPerSample;
    params["sampleRate"] = in->inputSampleRate;
    params["channels"] = in->inputChannels;
    params["samplePeriod"] = in->inputSamplePeriod;
    params["writePeriod"] = in->inputWritePeriod;
    params["sampleNumber"] = in->inputSampleNumber;
    params["samplesSize"] = in->inputSamplesSize;

    return params;

}



extern "C" {

    /**
         *  Function that is called by PHP right after the PHP process
         *  has started, and that returns an address of an internal PHP
         *  strucure with all the details and features of your extension
         *
         *  @return void*   a pointer to an address that is understood by PHP
         */
    PHPCPP_EXPORT void *get_module()
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("php-libtgvoip", "1.1.2");

        // description of the class so that PHP knows which methods are accessible
        Php::Class<VoIP> voip("VoIP");

        voip.method<&VoIP::getState>("getState", Php::Public | Php::Final);
        voip.method<&VoIP::getCallState>("getCallState", Php::Public | Php::Final);
        voip.method<&VoIP::getVisualization>("getVisualization", Php::Public | Php::Final);
        voip.method<&VoIP::setVisualization>("setVisualization", Php::Public | Php::Final, {Php::ByVal("visualization", Php::Type::Array)});
        voip.method<&VoIP::getOtherID>("getOtherID", Php::Public | Php::Final);
        voip.method<&VoIP::getProtocol>("getProtocol", Php::Public | Php::Final);
        voip.method<&VoIP::setMadeline>("setMadeline", Php::Public | Php::Final);
        voip.method<&VoIP::getCallID>("getCallID", Php::Public | Php::Final);
        voip.method<&VoIP::isCreator>("isCreator", Php::Public | Php::Final);
        voip.method<&VoIP::whenCreated>("whenCreated", Php::Public | Php::Final);
        voip.method<&VoIP::isPlaying>("isPlaying", Php::Public | Php::Final);
        voip.method<&VoIP::getOutputState>("getOutputState", Php::Public | Php::Final);
        voip.method<&VoIP::getInputState>("getInputState", Php::Public | Php::Final);
        voip.method<&VoIP::getOutputParams>("getOutputParams", Php::Public | Php::Final);
        voip.method<&VoIP::getInputParams>("getInputParams", Php::Public | Php::Final);

        voip.method<&VoIP::discard>("__destruct", Php::Public | Php::Final);
        voip.method<&VoIP::discard>("discard", Php::Public | Php::Final, {Php::ByVal("reason", Php::Type::Array, false), Php::ByVal("rating", Php::Type::Array, false), Php::ByVal("debug", Php::Type::Bool, false)});
        voip.method<&VoIP::accept>("accept", Php::Public | Php::Final);
        voip.method<&VoIP::close>("close", Php::Public | Php::Final);
        voip.method<&VoIP::__construct>("__construct", Php::Public | Php::Final, {
            Php::ByVal("creator", Php::Type::Bool), Php::ByVal("otherID", Php::Type::Numeric), Php::ByVal("InputPhoneCall", Php::Type::Array), Php::ByRef("madeline", Php::Type::Object), Php::ByVal("callState", Php::Type::Numeric), Php::ByVal("protocol", Php::Type::Array)
        });
        voip.method<&VoIP::__wakeup>("__wakeup", Php::Public | Php::Final);
        voip.method<&VoIP::__sleep>("__sleep", Php::Public | Php::Final);
        voip.method<&VoIP::setMicMute>("setMicMute", Php::Public | Php::Final, {
            Php::ByVal("type", Php::Type::Bool),
        });
        voip.method<&VoIP::debugCtl>("debugCtl", Php::Public | Php::Final, {
            Php::ByVal("request", Php::Type::Numeric), Php::ByVal("param", Php::Type::Numeric),
        });
        voip.method<&VoIP::parseConfig>("parseConfig", Php::Public | Php::Final);
        voip.method<&VoIP::getDebugLog>("getDebugLog", Php::Public | Php::Final);
        voip.method<&VoIP::getLastError>("getLastError", Php::Public | Php::Final);
        voip.method<&VoIP::getPreferredRelayID>("getPreferredRelayID", Php::Public | Php::Final);
        voip.method<&VoIP::getVersion>("getVersion", Php::Public | Php::Final);
        voip.method<&VoIP::getSignalBarsCount>("getSignalBarsCount", Php::Public | Php::Final);
        voip.method<&VoIP::getDebugString>("getDebugString", Php::Public | Php::Final);
        voip.method<&VoIP::getStats>("getStats", Php::Public | Php::Final);
        voip.method<&VoIP::startTheMagic>("startTheMagic", Php::Public | Php::Final);

        voip.method<&VoIP::play>("then", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::play>("play", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::playOnHold>("playOnHold", Php::Public | Php::Final, {Php::ByVal("files", Php::Type::Array)});

        voip.method<&VoIP::setOutputFile>("setOutputFile", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::unsetOutputFile>("unsetOutputFile", Php::Public | Php::Final);

        voip.method<&VoIP::setBitrate>("setBitrate", Php::Public | Php::Final, {Php::ByVal("bitrate", Php::Type::Numeric)});

        voip.property("configuration", 0, Php::Public);
        voip.property("storage", 0, Php::Public);
        voip.property("internalStorage", 0, Php::Private);

        voip.constant("STATE_CREATED", STATE_CREATED);
        voip.constant("STATE_WAIT_INIT", STATE_WAIT_INIT);
        voip.constant("STATE_WAIT_INIT_ACK", STATE_WAIT_INIT_ACK);
        voip.constant("STATE_ESTABLISHED", STATE_ESTABLISHED);
        voip.constant("STATE_FAILED", STATE_FAILED);
        voip.constant("STATE_RECONNECTING", STATE_RECONNECTING);

        voip.constant("TGVOIP_ERROR_UNKNOWN", TGVOIP_ERROR_UNKNOWN);
        voip.constant("TGVOIP_ERROR_INCOMPATIBLE", TGVOIP_ERROR_INCOMPATIBLE);
        voip.constant("TGVOIP_ERROR_TIMEOUT", TGVOIP_ERROR_TIMEOUT);
        voip.constant("TGVOIP_ERROR_AUDIO_IO", TGVOIP_ERROR_AUDIO_IO);

        voip.constant("NET_TYPE_UNKNOWN", NET_TYPE_UNKNOWN);
        voip.constant("NET_TYPE_GPRS", NET_TYPE_GPRS);
        voip.constant("NET_TYPE_EDGE", NET_TYPE_EDGE);
        voip.constant("NET_TYPE_3G", NET_TYPE_3G);
        voip.constant("NET_TYPE_HSPA", NET_TYPE_HSPA);
        voip.constant("NET_TYPE_LTE", NET_TYPE_LTE);
        voip.constant("NET_TYPE_WIFI", NET_TYPE_WIFI);
        voip.constant("NET_TYPE_ETHERNET", NET_TYPE_ETHERNET);
        voip.constant("NET_TYPE_OTHER_HIGH_SPEED", NET_TYPE_OTHER_HIGH_SPEED);
        voip.constant("NET_TYPE_OTHER_LOW_SPEED", NET_TYPE_OTHER_LOW_SPEED);
        voip.constant("NET_TYPE_DIALUP", NET_TYPE_DIALUP);
        voip.constant("NET_TYPE_OTHER_MOBILE", NET_TYPE_OTHER_MOBILE);

        voip.constant("DATA_SAVING_NEVER", DATA_SAVING_NEVER);
        voip.constant("DATA_SAVING_MOBILE", DATA_SAVING_MOBILE);
        voip.constant("DATA_SAVING_ALWAYS", DATA_SAVING_ALWAYS);

        voip.constant("PROXY_NONE", PROXY_NONE);
        voip.constant("PROXY_SOCKS5", PROXY_SOCKS5);

        voip.constant("AUDIO_STATE_NONE", AUDIO_STATE_NONE);
        voip.constant("AUDIO_STATE_CREATED", AUDIO_STATE_CREATED);
        voip.constant("AUDIO_STATE_CONFIGURED", AUDIO_STATE_CONFIGURED);
        voip.constant("AUDIO_STATE_RUNNING", AUDIO_STATE_RUNNING);

        voip.constant("CALL_STATE_NONE", CALL_STATE_NONE);
        voip.constant("CALL_STATE_REQUESTED", CALL_STATE_REQUESTED);
        voip.constant("CALL_STATE_INCOMING", CALL_STATE_INCOMING);
        voip.constant("CALL_STATE_ACCEPTED", CALL_STATE_ACCEPTED);
        voip.constant("CALL_STATE_CONFIRMED", CALL_STATE_CONFIRMED);
        voip.constant("CALL_STATE_READY", CALL_STATE_READY);
        voip.constant("CALL_STATE_ENDED", CALL_STATE_ENDED);

        voip.constant("PHP_LIBTGVOIP_VERSION", "1.1.2");

        Php::Namespace danog("danog");
        Php::Namespace MadelineProto("MadelineProto");

        MadelineProto.add(move(voip));
        danog.add(move(MadelineProto));
        extension.add(move(danog));

        return extension;
    }
}
