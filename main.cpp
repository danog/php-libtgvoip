/*
Copyright 2016-2017 Daniil Gentili
(https://daniil.it)
This file is part of php-libtgvoip.
php-libtgvoip is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the free Software Foundation, either version 3 of the License, or (at your option) any later version.
php-libtgvoip is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.
You should have received a copy of the GNU General Public License along with php-libtgvoip.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"
#include <string.h>
#include <wchar.h>
#include <map>
#include <string>
#include <vector>
#include <queue>

#include "libtgvoip/VoIPServerConfig.h"
#include "libtgvoip/tools/threading.h"
#include "libtgvoip/tools/logging.h"

using namespace tgvoip;
using namespace tgvoip::audio;
using namespace std;

void VoIP::__construct(Php::Parameters &params)
{
    Php::Value self(this);
    Php::Array empty;

    self["configuration"]["endpoints"] = empty;
    self["storage"] = empty;
    self["internalStorage"] = empty;

    self["internalStorage"]["creator"] = params[0];
    otherID = (int)params[1];
    self["madeline"] = params[2];
    callState = (int)params[3];

    initVoIPController();
}
void VoIP::setCall(Php::Parameters &params)
{
    Php::Array empty;
    Php::Value self(this);
    Php::Value call = params[0];
    Php::Value callId = call["id"];
    Php::Value callAccessHash = call["access_hash"];
    Php::Value protocol = params[0]["protocol"];

    Php::Array callArray;
    callArray["_"] = "inputPhoneCall";
    callArray["id"] = callId;
    callArray["access_hash"] = callAccessHash;

    self["internalStorage"]["callID"] = callArray;
    self["internalStorage"]["protocol"] = protocol;
}
void VoIP::initVoIPController()
{
    inst = std::make_unique<VoIPController>();

    outputFile = NULL;

    inst->implData = (void *)this;
    VoIPController::Callbacks callbacks;
    callbacks.connectionStateChanged = [](VoIPController *controller, int state) {
        ((VoIP *)controller->implData)->state = state;
        if (state == STATE_FAILED)
        {
            ((VoIP *)controller->implData)->deinitVoIPController();
        }
    };
    callbacks.signalBarCountChanged = NULL;
    callbacks.groupCallKeySent = NULL;
    callbacks.groupCallKeyReceived = NULL;
    callbacks.upgradeToGroupCallRequested = NULL;
    inst->SetCallbacks(callbacks);
    inst->SetAudioDataCallbacks([this](int16_t *buffer, size_t size) { this->sendAudioFrame(buffer, size); }, [this](int16_t *buffer, size_t size) { this->recvAudioFrame(buffer, size); }, [](int16_t *, size_t) {});
}

void VoIP::deinitVoIPController()
{
    if (callState != CALL_STATE_ENDED)
    {
        callState = CALL_STATE_ENDED;
        inst->Stop();

        while (holdFiles.size())
        {
            fclose(holdFiles.front());
            holdFiles.pop();
        }
        while (inputFiles.size())
        {
            fclose(inputFiles.front());
            inputFiles.pop();
        }
        unsetOutputFile();
    }
}

void VoIP::recvAudioFrame(int16_t *data, size_t size)
{
    MutexGuard m(outputMutex);
    if (this->outputFile != NULL)
    {
        if (fwrite(data, sizeof(int16_t), size, this->outputFile) != size)
        {
            LOGE("COULD NOT WRITE DATA TO FILE");
        }
    }
}
void VoIP::sendAudioFrame(int16_t *data, size_t size)
{
    MutexGuard m(inputMutex);

    if (!this->inputFiles.empty())
    {
        if ((readInput = fread(data, sizeof(int16_t), size, this->inputFiles.front())) != size)
        {
            fclose(this->inputFiles.front());
            this->inputFiles.pop();
            memset(data + (readInput % size), 0, size - (readInput % size));
        }
        this->playing = true;
    }
    else
    {
        this->playing = false;
        if (!this->holdFiles.empty())
        {
            if ((readInput = fread(data, sizeof(int16_t), size, this->holdFiles.front())) != size)
            {
                fseek(this->holdFiles.front(), 0, SEEK_SET);
                this->holdFiles.push(this->holdFiles.front());
                this->holdFiles.pop();
                memset(data + (readInput % size), 0, size - (readInput % size));
            }
        }
    }
}

Php::Value VoIP::discard(Php::Parameters &params)
{
    if (callState == CALL_STATE_ENDED)
    {
        return false;
    }
    Php::Value self(this);
    if (self["madeline"] && self["madeline"].value().instanceOf("danog\\MadelineProto\\MTProto"))
    {
        Php::Array reason;
        Php::Array rating;
        Php::Value debug;
        if (params.size() > 0)
        {
            reason = params[0];
        }
        else
        {
            reason["_"] = "phoneCallDiscardReasonDisconnect";
        }
        if (params.size() > 1)
        {
            rating = params[1];
        }
        if (params.size() > 2)
        {
            debug = params[2];
        }
        else
            debug = true;

        if (self["internalStorage"]["callID"].value()) {
            Php::Value res = self["madeline"].value().call("discardCallFrom", self, self["internalStorage"]["callID"].value(), reason, rating, debug);
            deinitVoIPController();
            return res;
        }
    }
    deinitVoIPController();
    return this;
}

Php::Value VoIP::accept()
{
    if (callState != CALL_STATE_INCOMING)
        return false;
    callState = CALL_STATE_ACCEPTED;
    Php::Value self(this);

    Php::Array method;
    method[0] = self["madeline"].value();
    method[1] = "acceptCallFrom";
    
    return Php::call("call_user_func", method, self, self["internalStorage"].value()["callID"].value());
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
    if (state == STATE_WAIT_INIT_ACK)
    {
        Php::Value self(this);
        if (!self["configuration"])
        {
            return false;
        }
        if (self["madeline"].value().instanceOf("danog\\MadelineProto\\MTProto"))
        {
            Php::Array reason;
            Php::Array rating;
            Php::Value debug;
            reason["_"] = "phoneCallDiscardReasonDisconnect";
            debug = false;
            self["madeline"].value().call("discardCallFrom", self, self["internalStorage"].value()["callID"].value(), reason, rating, debug);
        }
        deinitVoIPController();
        return false;
    }
    inst->Start();
    inst->Connect();
    Php::Value self(this);
    self["internalStorage"]["created"] = (int64_t)time(NULL);
    callState = CALL_STATE_READY;
    return true;
}

Php::Value VoIP::whenCreated()
{

    Php::Value self(this);
    if (self["internalStorage"].value()["created"])
    {
        return self["internalStorage"].value()["created"];
    }
    return false;
}
Php::Value VoIP::isCreator()
{

    Php::Value self(this);
    return self["internalStorage"].value()["creator"];
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
    return self["internalStorage"].value()["protocol"];
}
Php::Value VoIP::getCallID()
{
    Php::Value self(this);
    return self["internalStorage"].value()["callID"].value();
}

Php::Value VoIP::getCallState()
{
    return callState;
}

Php::Value VoIP::getVisualization()
{
    Php::Value self(this);
    Php::Value internalStorage = self["internalStorage"].value();
    if (internalStorage["visualization"])
    {
        return internalStorage["visualization"].value();
    }
    return false;
}
void VoIP::setVisualization(Php::Parameters &params)
{
    Php::Value self(this);
    Php::Value internalStorage = self["internalStorage"].value();
    internalStorage["visualization"] = params[0];
    self["internalStorage"] = internalStorage;
}

void VoIP::parseConfig()
{
    Php::Value self(this);
    if (!self["configuration"]["auth_key"])
        return;

    Php::Value config = self["configuration"].value();
    Php::Value internalStorage = self["internalStorage"].value();
    
    VoIPController::Config cfg;
    cfg.recvTimeout = (double)config["recv_timeout"];
    cfg.initTimeout = (double)config["init_timeout"];
    cfg.dataSaving = (int)config["data_saving"];
    cfg.enableAEC = (bool)config["enable_AEC"];
    cfg.enableNS = (bool)config["enable_NS"];
    cfg.enableAGC = (bool)config["enable_AGC"];
    cfg.enableCallUpgrade = (bool)config["enable_call_upgrade"];

    if (config["log_file_path"])
    {
        std::string logFilePath = config["log_file_path"];
        cfg.logFilePath = logFilePath;
    }

    if (config["stats_dump_file_path"])
    {
        std::string statsDumpFilePath = config["stats_dump_file_path"];
        cfg.statsDumpFilePath = statsDumpFilePath;
    }

    if (config["shared_config"])
    {
        Php::Value shared_config = config["shared_config"];
        ServerConfig::GetSharedInstance()->Update(Php::call("json_encode", Php::call("array_merge", Php::call("\\danog\\MadelineProto\\VoIPServerConfig::getFinal"), shared_config)));
    }
    inst->SetConfig(cfg);

    /*char *key = (char *)malloc(256);
    memcpy(key, config["auth_key"], 256);*/
    std::string strKey = config["auth_key"];
    std::vector<uint8_t> key(strKey.begin(), strKey.end());
    inst->SetEncryptionKey(key, (bool)internalStorage["creator"]);

    vector<Endpoint> eps;
    Php::Value endpoints = config["endpoints"];
    for (int i = 0; i < endpoints.size(); i++)
    {
        string ip = endpoints[i]["ip"];
        string ipv6 = endpoints[i]["ipv6"];
        string peer_tag = endpoints[i]["peer_tag"];

        IPv4Address v4addr(ip);
        IPv6Address v6addr("::0");
        unsigned char *pTag = (unsigned char *)malloc(16);

        if (ipv6 != "")
        {
            v6addr = IPv6Address(ipv6);
        }

        if (peer_tag != "")
        {
            memcpy(pTag, peer_tag.c_str(), 16);
        }
        eps.push_back(Endpoint(endpoints[i]["id"], (int32_t)endpoints[i]["port"], v4addr, v6addr, Endpoint::Type::UDP_RELAY, pTag));
        free(pTag);
    }

    inst->SetRemoteEndpoints(eps, (bool)internalStorage["protocol"]["udp_p2p"], (int)internalStorage["protocol"]["max_layer"]);
    inst->SetNetworkType(config["network_type"]);
    if (config["proxy"])
    {
        inst->SetProxy(config["proxy"]["protocol"], config["proxy"]["address"], (int32_t)config["proxy"]["port"], config["proxy"]["username"], config["proxy"]["password"]);
    }
}

Php::Value VoIP::unsetOutputFile()
{
    if (outputFile == NULL)
    {
        return false;
    }

    MutexGuard m(outputMutex);
    fflush(outputFile);
    fclose(outputFile);
    outputFile = NULL;

    return this;
}
Php::Value VoIP::setOutputFile(Php::Parameters &params)
{

    MutexGuard m(outputMutex);

    if (outputFile != NULL)
    {
        fclose(outputFile);
        outputFile = NULL;
    }
    outputFile = fopen(params[0], "wb");
    if (outputFile == NULL)
    {
        throw Php::Exception("Could not open file!");
        return false;
    }
    return this;
}

Php::Value VoIP::play(Php::Parameters &params)
{
    FILE *tmp = fopen(params[0], "rb");

    if (tmp == NULL)
    {
        throw Php::Exception("Could not open file!");
        return false;
    }

    MutexGuard m(inputMutex);
    inputFiles.push(tmp);

    return this;
}
Php::Value VoIP::playOnHold(Php::Parameters &params)
{
    FILE *tmp = NULL;

    MutexGuard m(inputMutex);
    while (holdFiles.size())
    {
        fclose(holdFiles.front());
        holdFiles.pop();
    }
    for (int i = 0; i < params[0].size(); i++)
    {
        tmp = fopen(params[0][i], "rb");
        if (tmp == NULL)
        {
            throw Php::Exception("Could not open file!");
            return false;
        }
        holdFiles.push(tmp);
    }
    return this;
}

Php::Value VoIP::setMicMute(Php::Parameters &params)
{
    inst->SetMicMute((bool)params[0]);
    return this;
}

Php::Value VoIP::getDebugLog()
{
    Php::Value data;
    string encoded = inst->GetDebugLog();
    if (!encoded.empty())
    {
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
    return inst->GetDebugString();
}
Php::Value VoIP::getStats()
{
    Php::Value stats;
    VoIPController::TrafficStats _stats;
    inst->GetStats(&_stats);
    stats["bytesSentWifi"] = (int64_t)_stats.bytesSentWifi;
    stats["bytesSentMobile"] = (int64_t)_stats.bytesSentMobile;
    stats["bytesRecvdWifi"] = (int64_t)_stats.bytesRecvdWifi;
    stats["bytesRecvdMobile"] = (int64_t)_stats.bytesRecvdMobile;
    return stats;
}

Php::Value VoIP::getPeerCapabilities()
{
    return (int64_t)inst->GetPeerCapabilities();
}
Php::Value VoIP::getConnectionMaxLayer()
{
    return VoIPController::GetConnectionMaxLayer();
}
void VoIP::requestCallUpgrade()
{
    return inst->RequestCallUpgrade();
}

void VoIP::sendGroupCallKey(Php::Parameters &params)
{
    unsigned char *key = (unsigned char *)malloc(256);
    memcpy(key, params[0], 256);
    inst->SendGroupCallKey(key);
}

Php::Value VoIP::getState()
{
    return state;
}

Php::Value VoIP::isPlaying()
{
    return playing;
}

void VoIPServerConfigInternal::update(Php::Parameters &params)
{
    Php::Array settings = params[0];
    ServerConfig::GetSharedInstance()->Update(Php::call("json_encode", settings));
}
extern "C"
{

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
        static Php::Extension extension("php-libtgvoip", "1.5.0");

        Php::Class<VoIPServerConfigInternal> voipServerConfigInternal("VoIPServerConfigInternal");
        voipServerConfigInternal.method<&VoIPServerConfigInternal::update>("updateInternal", Php::Protected | Php::Static, {Php::ByVal("config", Php::Type::Array)});

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

        voip.method<&VoIP::discard>("__destruct", Php::Public | Php::Final);
        voip.method<&VoIP::discard>("discard", Php::Public | Php::Final, {Php::ByVal("reason", Php::Type::Array, false), Php::ByVal("rating", Php::Type::Array, false), Php::ByVal("debug", Php::Type::Bool, false)});
        voip.method<&VoIP::accept>("accept", Php::Public | Php::Final);
        voip.method<&VoIP::close>("close", Php::Public | Php::Final);
        voip.method<&VoIP::setCall>("setCall", Php::Public | Php::Final, {Php::ByVal("phoneCall", Php::Type::Array)});
        voip.method<&VoIP::__construct>("__construct", Php::Public | Php::Final, {Php::ByVal("creator", Php::Type::Bool), Php::ByVal("otherID", Php::Type::Numeric), Php::ByRef("madeline", Php::Type::Object), Php::ByVal("callState", Php::Type::Numeric)});
        voip.method<&VoIP::__wakeup>("__wakeup", Php::Public | Php::Final);
        voip.method<&VoIP::__sleep>("__sleep", Php::Public | Php::Final);
        voip.method<&VoIP::setMicMute>("setMicMute", Php::Public | Php::Final, {
                                                                                   Php::ByVal("type", Php::Type::Bool),
                                                                               });
        voip.method<&VoIP::parseConfig>("parseConfig", Php::Public | Php::Final);
        voip.method<&VoIP::getDebugLog>("getDebugLog", Php::Public | Php::Final);
        voip.method<&VoIP::getLastError>("getLastError", Php::Public | Php::Final);
        voip.method<&VoIP::getPreferredRelayID>("getPreferredRelayID", Php::Public | Php::Final);
        voip.method<&VoIP::getVersion>("getVersion", Php::Public | Php::Final);
        voip.method<&VoIP::getSignalBarsCount>("getSignalBarsCount", Php::Public | Php::Final);
        voip.method<&VoIP::getDebugString>("getDebugString", Php::Public | Php::Final);
        voip.method<&VoIP::getStats>("getStats", Php::Public | Php::Final);
        voip.method<&VoIP::getPeerCapabilities>("getPeerCapabilities", Php::Public | Php::Final);
        voip.method<&VoIP::getConnectionMaxLayer>("getConnectionMaxLayer", Php::Public);
        voip.method<&VoIP::sendGroupCallKey>("sendGroupCallKey", Php::Public | Php::Final, {Php::ByVal("key", Php::Type::String)});
        voip.method<&VoIP::requestCallUpgrade>("requestCallUpgrade", Php::Public | Php::Final);
        voip.method<&VoIP::startTheMagic>("startTheMagic", Php::Public | Php::Final);

        voip.method<&VoIP::play>("then", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::play>("play", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::playOnHold>("playOnHold", Php::Public | Php::Final, {Php::ByVal("files", Php::Type::Array)});

        voip.method<&VoIP::setOutputFile>("setOutputFile", Php::Public | Php::Final, {Php::ByVal("file", Php::Type::String)});
        voip.method<&VoIP::unsetOutputFile>("unsetOutputFile", Php::Public | Php::Final);

        voip.property("configuration", 0, Php::Public);
        voip.property("storage", 0, Php::Public);
        voip.property("internalStorage", 0, Php::Private);

        voip.constant("STATE_CREATED", STATE_CREATED);
        voip.constant("STATE_WAIT_INIT", STATE_WAIT_INIT);
        voip.constant("STATE_WAIT_INIT_ACK", STATE_WAIT_INIT_ACK);
        voip.constant("STATE_ESTABLISHED", STATE_ESTABLISHED);
        voip.constant("STATE_FAILED", STATE_FAILED);
        voip.constant("STATE_RECONNECTING", STATE_RECONNECTING);

        voip.constant("TGVOIP_ERROR_UNKNOWN", ERROR_UNKNOWN);
        voip.constant("TGVOIP_ERROR_INCOMPATIBLE", ERROR_INCOMPATIBLE);
        voip.constant("TGVOIP_ERROR_TIMEOUT", ERROR_TIMEOUT);
        voip.constant("TGVOIP_ERROR_AUDIO_IO", ERROR_AUDIO_IO);
        voip.constant("TGVOIP_ERROR_PROXY", ERROR_PROXY);

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

        voip.constant("TGVOIP_PEER_CAP_GROUP_CALLS", TGVOIP_PEER_CAP_GROUP_CALLS);

        voip.constant("PHP_LIBTGVOIP_VERSION", "1.5.0");

        Php::Namespace danog("danog");
        Php::Namespace MadelineProto("MadelineProto");

        MadelineProto.add(move(voipServerConfigInternal));
        MadelineProto.add(move(voip));
        danog.add(move(MadelineProto));
        extension.add(move(danog));

        return extension;
    }
}
