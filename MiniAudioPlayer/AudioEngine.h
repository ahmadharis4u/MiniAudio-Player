#pragma once

#include "AudioEngineConfig.h"

#ifdef USE_AUDIO_ENGINE
#include <string>

struct FileData
{
	char* pData = NULL;
	size_t nSize = 0;
};

class CAudioEngine
{
public:
	CAudioEngine() = default;
	~CAudioEngine() = default;

	int PlayAudio();
	int PlayAudios(int nCount);
	int PausePlayback();
	int ResumePlayback();
	int Forward10s();
	int Backward10s();
	int StartRecording();
	int PlayRecording();
	int Quit();

private:
	int InitializePlayback();
	int PrepareAudio(void* pItem);
	bool m_bRecord = false;
	std::string m_fileName;
};

CAudioEngine& AudioEngine();

#endif // USE_AUDIO_ENGINE
