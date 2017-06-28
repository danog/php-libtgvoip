//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef LIBTGVOIP_AUDIOINPUTPHP_H
#define LIBTGVOIP_AUDIOINPUTPHP_H

#include "../libtgvoip/audio/AudioInput.h"
#include "../libtgvoip/threading.h"
#include <phpcpp.h>

namespace tgvoip{ namespace audio{
class AudioInputPHP : public AudioInput{

public:
	AudioInputPHP(Php::Value callbacks);
	virtual ~AudioInputPHP();

	virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
	virtual void Start();
	virtual void Stop();
	bool writeFrames(const char* data);
private:
	Php::Value configureMethod;
	Php::Value startMethod;
	Php::Value stopMethod;
	bool running;
};
}}

#endif //LIBTGVOIP_AUDIOINPUTPHP_H
