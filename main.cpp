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

#include "main.h"
#include <string.h>
#include <wchar.h>
#include <map>
#include <string>
#include <vector>

#include "libtgvoip/VoIPServerConfig.h"

#include "audio/AudioInputModule.h"
#include "audio/AudioOutputModule.h"

using namespace tgvoip;
using namespace tgvoip::audio;

void VoIP::__construct(Php::Parameters &params)
{
    PHPthis = (Php::Object)this;
    PHPthis.call("debug", "init started");

    madeline = params[1];
    current_call = params[2];

    inst = new VoIPController();

    inst->implData = static_cast<void *>(this);
    inst->SetStateCallback([](tgvoip::VoIPController *controller, int state) {
        static_cast<VoIP *>(controller->implData)->updateConnectionState(state);
    });
    PHPthis.call("debug", "init done");
}

void VoIP::start()
{
    PHPthis.call("debug", "start");
    inst->Start();
}

void VoIP::connect()
{
    PHPthis.call("debug", "connect");
    inst->Connect();
}
void VoIP::setEncryptionKey(Php::Parameters &params)
{
    PHPthis.call("debug", "set key");

    char *key = (char *)malloc(256);
    memcpy(key, params[0], 256);
    inst->SetEncryptionKey(key, (bool)params[1]);
    free(key);
}

void VoIP::setRemoteEndpoints(Php::Parameters &params)
{
    PHPthis.call("debug", "set endpoints");

    std::vector<Endpoint> eps;
    for (int i = 0; i < params[0].size(); i++)
    {
        std::string ip = params[0][i]["ip"];
        std::string ipv6 = params[0][i]["ipv6"];
        std::string peer_tag = params[0][i]["peer_tag"];

        tgvoip::IPv4Address v4addr(ip);
        tgvoip::IPv6Address v6addr("::0");
        unsigned char *pTag = (unsigned char*) malloc(16);

        if (ipv6 != "")
        {
            v6addr = IPv6Address(ipv6);
        }

        if (peer_tag != "")
        {
            memcpy(pTag, peer_tag.c_str(), 16);
        }

        eps.push_back(Endpoint(params[0][i]["id"], (int32_t)params[0][i]["port"], v4addr, v6addr, EP_TYPE_UDP_RELAY, pTag));
        free(pTag);
    }
    inst->SetRemoteEndpoints(eps, params[1]);
}

void VoIP::release()
{
    delete inst;
}

Php::Value VoIP::writeFrames(Php::Parameters &params)
{
    AudioInputModule *in = (AudioInputModule *)(intptr_t)inst;
    return in->writeFrames(params[0]);
}

Php::Value VoIP::readFrames()
{
    AudioOutputModule *out = (AudioOutputModule *)(intptr_t)inst;
    return out->readFrames();
}

Php::Value VoIP::getDebugString()
{
    char buf[10240];
    inst->GetDebugString(buf, 10240);
    return buf;
}

void VoIP::setNetworkType(Php::Parameters &params)
{
    PHPthis.call("debug", "set network");
    inst->SetNetworkType(params[0]);
}

void VoIP::setMicMute(Php::Parameters &params)
{
    inst->SetMicMute(params[0]);
}
// jdouble recvTimeout, jdouble initTimeout, jint dataSavingMode, jboolean enableAEC, jboolean enableNS, jboolean enableAGC, jstring logFilePath
void VoIP::setConfig(Php::Parameters &params)
{
    PHPthis.call("debug", "set config");

    voip_config_t cfg;
    cfg.recv_timeout = params[0];
    cfg.init_timeout = params[1];
    cfg.data_saving = params[2];
    cfg.enableAEC = params[3];
    cfg.enableNS = params[4];
    cfg.enableAGC = params[5];
    if (params.size() == 7)
    {
        strncpy(cfg.logFilePath, params[6], sizeof(cfg.logFilePath));
        cfg.logFilePath[sizeof(cfg.logFilePath) - 1] = 0;
    }
    else
    {
        memset(cfg.logFilePath, 0, sizeof(cfg.logFilePath));
    }

    if (params.size() == 8)
    {
        strncpy(cfg.statsDumpFilePath, params[7], sizeof(cfg.statsDumpFilePath));
        cfg.statsDumpFilePath[sizeof(cfg.statsDumpFilePath) - 1] = 0;
    }
    else
    {
        memset(cfg.statsDumpFilePath, 0, sizeof(cfg.statsDumpFilePath));
    }
    inst->SetConfig(&cfg);
}
// int protocol, std::string address, uint16_t port, std::string username, std::string password
void VoIP::setProxy(Php::Parameters &params)
{
    inst->SetProxy(params[0], params[1], (int32_t) params[2], params[3], params[4]);
}
void VoIP::debugCtl(Php::Parameters &params)
{
    inst->DebugCtl(params[0], params[1]);
}

Php::Value VoIP::getVersion()
{
    return VoIPController::GetVersion();
}

Php::Value VoIP::getPreferredRelayID()
{
    return inst->GetPreferredRelayID();
}

Php::Value VoIP::getLastError()
{
    return inst->GetLastError();
}

Php::Value VoIP::getStats()
{
    voip_stats_t _stats;
    inst->GetStats(&_stats);
    Php::Array stats;
    stats["bytesSentWifi"] = (int64_t)_stats.bytesSentWifi;
    stats["bytesSentMobile"] = (int64_t)_stats.bytesSentMobile;
    stats["bytesRecvdWifi"] = (int64_t)_stats.bytesRecvdWifi;
    stats["bytesRecvdMobile"] = (int64_t)_stats.bytesRecvdMobile;
    return stats;
}

void VoIP::setSharedConfig(Php::Parameters &params)
{
    PHPthis.call("debug", "set shared config");
    ServerConfig::GetSharedInstance()->Update(params[0]);
}

Php::Value VoIP::getDebugLog()
{
    return inst->GetDebugLog();
}

void VoIP::updateConnectionState(int state)
{
    PHPthis.call("debug", "state");
    PHPthis.call("debug", "state");
    PHPthis.call("debug", "state");
    PHPthis.call("debug", "state");
    PHPthis.call("debug", "state");

    PHPthis.call("setState", state);
    //setStateMethod(state);
}

void VoIP::startInput()
{
    PHPthis.call("debug", "start input");
    PHPthis.call("startInput");
}

void VoIP::startOutput()
{
    PHPthis.call("debug", "start output");
    PHPthis.call("startOutput");

}

void VoIP::stopInput()
{
    PHPthis.call("stopInput");
}
void VoIP::stopOutput()
{
    PHPthis.call("startInput");
}


void VoIP::configureAudioInput(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels) {
    inputSampleRate = sampleRate;
    inputBitsPerSample = bitsPerSample;
    inputChannels = channels;
    inputSamplePeriod = 1/sampleRate*1000000;
    inputWritePeriod = 1/sampleRate*960*1000000;
    configuredInput = true;
    PHPthis.call("debug", "configure input");
}
void VoIP::configureAudioOutput(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels) {
    outputSampleRate = sampleRate;
    outputBitsPerSample = bitsPerSample;
    outputChannels = channels;
    outputSamplePeriod = 1/sampleRate;
    outputWritePeriod = 1/sampleRate*960*1000000;
    configuredOutput = true;
    PHPthis.call("debug", "configure output");
}
float VoIP::getOutputLevel() {
    return (double)PHPthis.call("getOutputLevel");
}

Php::Value VoIP::getCallConfig() {
    Php::Value result;
    if (configuredInput) {
        result["input"]["sampleRate"] = inputSampleRate;
        result["input"]["bitsPerSample"] = inputBitsPerSample;
        result["input"]["channels"] = inputChannels;
        result["input"]["samplePeriod"] = inputSamplePeriod;
        result["input"]["writePeriod"] = inputWritePeriod;
    }
    if (configuredOutput) {
        result["output"]["sampleRate"] = outputSampleRate;
        result["output"]["bitsPerSample"] = outputBitsPerSample;
        result["output"]["channels"] = outputChannels;
        result["output"]["samplePeriod"] = outputSamplePeriod;
        result["output"]["writePeriod"] = outputWritePeriod;
    }

    return result;
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
    static Php::Extension extension("php-libtgvoip", "1.0");

    // description of the class so that PHP knows which methods are accessible
    Php::Class<VoIP> voip("VoIP");

    voip.method("setState", {
        Php::ByVal("state", Php::Type::Numeric),
    });

    voip.method("startOutput");
    voip.method("stopOutput");
    voip.method("stopInput");
    voip.method("getOutputLevel");

    voip.method<&VoIP::getCallConfig>("getCallConfig");
    voip.method<&VoIP::__construct>("__construct", Php::Public | Php::Final, {
        Php::ByRef("madelineProto", Php::Type::Object), Php::ByVal("currentCall", Php::Type::Numeric),
    });
    voip.method<&VoIP::setEncryptionKey>("setEncryptionKey", Php::Public | Php::Final, {
        Php::ByVal("key", Php::Type::String), Php::ByVal("isOutgoing", Php::Type::Bool),
    });
    voip.method<&VoIP::setNetworkType>("setNetworkType", Php::Public | Php::Final, {
        Php::ByVal("type", Php::Type::Numeric),
    });
    voip.method<&VoIP::setMicMute>("setMicMute", Php::Public | Php::Final, {
        Php::ByVal("type", Php::Type::Bool),
    });
    voip.method<&VoIP::debugCtl>("debugCtl", Php::Public | Php::Final, {
        Php::ByVal("request", Php::Type::Numeric), Php::ByVal("param", Php::Type::Numeric),
    });
    voip.method<&VoIP::setConfig>("setConfig", Php::Public | Php::Final, {
        // jdouble recvTimeout, jdouble initTimeout, jint dataSavingMode, jboolean enableAEC, jboolean enableNS, jboolean enableAGC, jstring logFilePath
        Php::ByVal("recvTimeout", Php::Type::Float), Php::ByVal("initTimeout", Php::Type::Float), Php::ByVal("dataSavingMode", Php::Type::Bool), Php::ByVal("enableAEC", Php::Type::Bool), Php::ByVal("enableNS", Php::Type::Bool), Php::ByVal("enableAGC", Php::Type::Bool), Php::ByVal("logFilePath", Php::Type::String, false), Php::ByVal("statsDumpFilePath", Php::Type::String, false),
    });
    voip.method<&VoIP::setProxy>("setProxy", Php::Public | Php::Final, {
        // int protocol, std::string address, uint16_t port, std::string username, std::string password
        Php::ByVal("protocol", Php::Type::Numeric), Php::ByVal("address", Php::Type::String), Php::ByVal("port", Php::Type::Numeric), Php::ByVal("username", Php::Type::String), Php::ByVal("password", Php::Type::String),
    });
    voip.method<&VoIP::setSharedConfig>("setSharedConfig", Php::Public | Php::Final, {Php::ByVal("config", Php::Type::Array)});
    voip.method<&VoIP::setRemoteEndpoints>("setRemoteEndpoints", Php::Public | Php::Final, {Php::ByVal("endpoints", Php::Type::Array), Php::ByVal("allowP2P", Php::Type::Bool)});
    voip.method<&VoIP::getDebugLog>("getDebugLog", Php::Public | Php::Final);
    voip.method<&VoIP::getLastError>("getLastError", Php::Public | Php::Final);
    voip.method<&VoIP::getPreferredRelayID>("getPreferredRelayID", Php::Public | Php::Final);
    voip.method<&VoIP::getVersion>("getVersion", Php::Public | Php::Final);
    voip.method<&VoIP::getDebugString>("getDebugString", Php::Public | Php::Final);
    voip.method<&VoIP::getStats>("getStats", Php::Public | Php::Final);
    voip.method<&VoIP::release>("release", Php::Public | Php::Final);
    voip.method<&VoIP::start>("start", Php::Public | Php::Final);
    voip.method<&VoIP::connect>("connect", Php::Public | Php::Final);

    voip.method<&VoIP::readFrames>("readFrames", Php::Public | Php::Final);
    voip.method<&VoIP::writeFrames>("writeFrames", Php::Public | Php::Final, {Php::ByVal("frames", Php::Type::String)});

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


    voip.constant("FRAME_NUMBER", 960);
    voip.constant("FRAME_SIZE", 960*2);

    Php::Namespace danog("danog");
    Php::Namespace MadelineProto("MadelineProto");
    
    MadelineProto.add(std::move(voip));
    danog.add(std::move(MadelineProto));
    extension.add(std::move(danog));

    return extension;
}
}
