//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef LIBTGVOIP_AUDIOOUTPUTPHP_H
#define LIBTGVOIP_AUDIOOUTPUTPHP_H

#include "libtgvoip/audio/AudioOutput.h"
#include "phpcpp.h"

namespace tgvoip{ namespace audio{
class AudioOutputPHP : public AudioOutput{

public:

	AudioOutputPHP(Php::Value callbacks);
	virtual ~AudioOutputPHP();
	virtual void Configure(uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels);
	virtual void Start();
	virtual void Stop();
	virtual bool IsPlaying() override;
	virtual float GetLevel() override;
	virtual Php::Value readFrames();

	Php::Value configureMethod;
	Php::Value startMethod;
	Php::Value stopMethod;
	Php::Value isPlayingMethod;
	Php::Value getLevelMethod;
private:
	bool running;
};
}}

#endif //LIBTGVOIP_AUDIOOUTPUTPHP_H
