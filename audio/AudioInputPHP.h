//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef LIBTGVOIP_AUDIOINPUTPHP_H
#define LIBTGVOIP_AUDIOINPUTPHP_H

#include "../libtgvoip/audio/AudioInput.h"
#include "../libtgvoip/threading.h"
#include "../main.h"
#include "../libtgvoip/VoIPController.h"


using namespace tgvoip;
using namespace tgvoip::audio;

namespace tgvoip{ namespace audio{
class AudioInputPHP : public AudioInput{

public:
	AudioInputPHP(void* controller);
	virtual ~AudioInputPHP();

	virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
	virtual void Start();
	virtual void Stop();
	bool writeFrames(const char* data);
private:
	
	VoIP* wrapper;
	bool running;
};
}}

#endif //LIBTGVOIP_AUDIOINPUTPHP_H
