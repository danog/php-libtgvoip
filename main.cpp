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

#include <phpcpp.h>
#include <string.h>
#include <wchar.h>
#include <map>
#include <string>
#include <vector>
#include "libtgvoip/VoIPServerConfig.h"
#include "libtgvoip/VoIPController.h"
#include "libtgvoip/NetworkSocket.h"
#include "libtgvoip/logging.cpp"
#include "libtgvoip/VoIPController.cpp"
#include "libtgvoip/BufferInputStream.cpp"
#include "libtgvoip/BufferOutputStream.cpp"
#include "libtgvoip/BlockingQueue.cpp"
#include "libtgvoip/MediaStreamItf.cpp"
#include "libtgvoip/OpusEncoder.cpp"
#include "libtgvoip/JitterBuffer.cpp"
#include "libtgvoip/OpusDecoder.cpp"
#include "libtgvoip/BufferPool.cpp"
#include "libtgvoip/EchoCanceller.cpp"
#include "libtgvoip/CongestionControl.cpp"
#include "libtgvoip/VoIPServerConfig.cpp"
#include "libtgvoip/NetworkSocket.cpp"
#include "libtgvoip/os/posix/NetworkSocketPosix.cpp"

#include "AudioInputPHP.h"
#include "AudioOutputPHP.h"
//#include "libtgvoip/os/android/AudioInputOpenSLES.cpp"
//#include "libtgvoip/os/android/AudioOutputOpenSLES.cpp"
//#include "libtgvoip/os/android/OpenSLEngineWrapper.cpp"
//#include "libtgvoip/os/android/AudioInputAndroid.cpp"
//#include "libtgvoip/os/android/AudioOutputAndroid.cpp"

using namespace tgvoip;

class VoIP : public Php::Base {
public:

    void __construct(Php::Parameters &params) {
        setStateMethod = params[0];
        inst=new VoIPController(params[1], params[2]);
        inst->implData = static_cast<void*>(this);
        inst->SetStateCallback([](tgvoip::VoIPController *controller, int state) {
	    	static_cast<VoIP*>(controller->implData)->updateConnectionState(controller, state);
    	});
        
    }

    void start() {
        inst->Start();
    }

    void connect() {
        inst->Connect();
    }
    void setEncryptionKey(Php::Parameters &params) {
        char *key = strdup(params[0]);
        inst->SetEncryptionKey(key, params[1]);
        free(key);
    }

    void setRemoteEndpoints(Php::Parameters &params) {
    
        size_t len=(size_t) params[0].size();
        //voip_endpoint_t* eps=(voip_endpoint_t *) malloc(sizeof(voip_endpoint_t)*len);
        std::vector<Endpoint> eps;
        uint i;
        
        for(i=0; i<len; i++) {
            std::string ip = params[0][i]["ip"];
            std::string ipv6 = params[0][i]["ipv6"];
            std::string peer_tag = params[0][i]["peer_tag"];


            tgvoip::IPv4Address v4addr(ip);
            tgvoip::IPv6Address v6addr("::0");
            unsigned char pTag[16];

            if(ipv6 != ""){
                v6addr=IPv6Address(ipv6);
            }
            if(peer_tag != "") {
                memcpy(pTag, params[0][i]["peer_tag"], 16);
            }
            
            eps.push_back(Endpoint(params[0][i]["id"], (int32_t)params[0][i]["port"], v4addr, v6addr, EP_TYPE_UDP_RELAY, pTag));

        }
        
        inst->SetRemoteEndpoints(eps, params[1]);
        
    }

    void setNativeBufferSize(Php::Parameters &params) {
        //CAudioOutputOpenSLES::nativeBufferSize=params[0];
        //CAudioInputOpenSLES::nativeBufferSize=params[0];
    }

    void release() {
        //env->DeleteGlobalRef(CAudioInputAndroid::jniClass);

        delete inst;
    }
    
    
    void writeFrames(Php::Parameters &params) {
        AudioInputPHP* in=(AudioInputPHP*)(intptr_t)inst;
        in->writeFrames(params);
    }

    Php::Value readFrames() {
        AudioOutputPHP* out=(AudioOutputPHP*)(intptr_t)inst;
        return out->readFrames();
    }
    
    
    Php::Value getDebugString() {
        char buf[10240];
        inst->GetDebugString(buf, 10240);
        return buf;
    }

    void setNetworkType(Php::Parameters &params) {
        inst->SetNetworkType(params[0]);
    }

    void setMicMute(Php::Parameters &params) {
        inst->SetMicMute(params[0]);
    }
    // jdouble recvTimeout, jdouble initTimeout, jint dataSavingMode, jboolean enableAEC, jboolean enableNS, jboolean enableAGC, jstring logFilePath
    void setConfig(Php::Parameters &params) {
        voip_config_t cfg;
        cfg.recv_timeout=params[0];
        cfg.init_timeout=params[1];
        cfg.data_saving=params[2];
        cfg.enableAEC=params[3];
        cfg.enableNS=params[4];
        cfg.enableAGC=params[5];
        if (params.size() == 7) {
            strncpy(cfg.logFilePath, params[6], sizeof(cfg.logFilePath));
            cfg.logFilePath[sizeof(cfg.logFilePath) - 1] = 0;
        } else {
            memset(cfg.logFilePath, 0, sizeof(cfg.logFilePath));
        }

        if (params.size() == 8) {
            strncpy(cfg.statsDumpFilePath, params[7], sizeof(cfg.statsDumpFilePath));
            cfg.statsDumpFilePath[sizeof(cfg.statsDumpFilePath) - 1] = 0;
        } else {
            memset(cfg.statsDumpFilePath, 0, sizeof(cfg.statsDumpFilePath));
        }
        inst->SetConfig(&cfg);
    }

    void debugCtl(Php::Parameters &params) {
        inst->DebugCtl(params[0], params[1]);
    }

    Php::Value getVersion() {
        return VoIPController::GetVersion();
    }

    Php::Value getPreferredRelayID() {
        return inst->GetPreferredRelayID();
    }

    Php::Value getLastError() {
        return inst->GetLastError();
    }

    Php::Value getStats() {
        voip_stats_t _stats;
        inst->GetStats(&_stats);
        Php::Array stats;
        stats["bytesSentWifi"] = (int64_t)_stats.bytesSentWifi;
        stats["bytesSentMobile"] = (int64_t)_stats.bytesSentMobile;
        stats["bytesRecvdWifi"] = (int64_t)_stats.bytesRecvdWifi;
        stats["bytesRecvdMobile"] = (int64_t)_stats.bytesRecvdMobile;
        return stats;
    }

    void setSharedConfig(Php::Parameters &params) {
        ServerConfig::GetSharedInstance()->Update(params[0]);
    }

    Php::Value getDebugLog() {
        return inst->GetDebugLog();
    }
    
    void updateConnectionState(VoIPController* cntrlr, int state) {
        setStateMethod(state);
    }
private:

    VoIPController* inst;
    Php::Value setStateMethod;

};

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
        voip.method<&VoIP::__construct> ("__construct", {
            Php::ByVal("setStateCallable", Php::Type::Callable)
        });
        voip.method<&VoIP::setEncryptionKey> ("setEncryptionKey", {
            Php::ByVal("key", Php::Type::String),
            Php::ByVal("isOutgoing", Php::Type::Bool),
        });
        voip.method<&VoIP::setNetworkType> ("setNetworkType", {
            Php::ByVal("type", Php::Type::Numeric),
        });
        voip.method<&VoIP::setMicMute> ("setMicMute", {
            Php::ByVal("type", Php::Type::Bool),
        });
        voip.method<&VoIP::setNativeBufferSize> ("setNativeBufferSize", {
            Php::ByVal("type", Php::Type::Numeric),
        });
        voip.method<&VoIP::debugCtl> ("debugCtl", {
            Php::ByVal("request", Php::Type::Numeric),
            Php::ByVal("param", Php::Type::Numeric),
        });
        voip.method<&VoIP::setConfig> ("setConfig", { // jdouble recvTimeout, jdouble initTimeout, jint dataSavingMode, jboolean enableAEC, jboolean enableNS, jboolean enableAGC, jstring logFilePath
            Php::ByVal("recvTimeout", Php::Type::Float),
            Php::ByVal("initTimeout", Php::Type::Float),
            Php::ByVal("dataSavingMode", Php::Type::Numeric),
            Php::ByVal("enableAEC", Php::Type::Bool),
            Php::ByVal("enableAEC", Php::Type::Bool),
            Php::ByVal("enableAGC", Php::Type::Bool),
            Php::ByVal("logFilePath", Php::Type::String, false),
            Php::ByVal("statsDumpFilePath", Php::Type::String, false),
        });
        voip.method<&VoIP::setSharedConfig> ("setSharedConfig", {
            Php::ByVal("config", Php::Type::Array)
        });
        voip.method<&VoIP::setRemoteEndpoints> ("setRemoteEndpoints", {
            Php::ByVal("endpoints", Php::Type::Array),
            Php::ByVal("allowP2P", Php::Type::Bool)
        });
        voip.method<&VoIP::getDebugLog> ("getDebugLog");
        voip.method<&VoIP::getLastError> ("getLastError");
        voip.method<&VoIP::getPreferredRelayID> ("getPreferredRelayID");
        voip.method<&VoIP::getVersion> ("getVersion");
        voip.method<&VoIP::getDebugString> ("getDebugString");
        voip.method<&VoIP::getStats> ("getStats");
        voip.method<&VoIP::release> ("release");
        voip.method<&VoIP::start> ("start");
        voip.method<&VoIP::connect> ("connect");

        voip.method<&VoIP::readFrames> ("readFrames");
        voip.method<&VoIP::writeFrames> ("writeFrames", {
            Php::ByVal("frames", Php::Type::String)
        });
        
        Php::Namespace danog("danog");
        Php::Namespace MadelineProto("MadelineProto");

        MadelineProto.add(std::move(voip));
        danog.add(std::move(MadelineProto));
        extension.add(std::move(danog));
        
        return extension;
    }
}
