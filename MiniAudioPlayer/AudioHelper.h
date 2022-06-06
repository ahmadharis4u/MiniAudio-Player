#pragma once
#include "AudioEngineConfig.h"

#ifndef USE_AUDIO_ENGINE


class CAudioHelper
{
public:
	CAudioHelper() = default;
	~CAudioHelper() = default;

	int SimplePlayback();
	int PausePlayback();
	int ResumePlayback();
	int SimpleCapture();
	int Forward10s();
	int Backward10s();
	int Quit();
	int PlayRecording();

private:
	int InitializePlayback();
	bool m_bPlayback = false;
};

#endif // !USE_AUDIO_ENGINE