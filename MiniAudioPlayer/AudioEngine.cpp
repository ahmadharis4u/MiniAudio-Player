#include "pch.h"
#include <vector>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "AudioEngine.h"

#ifdef USE_AUDIO_ENGINE

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace std;

namespace
{
    struct AudioPlayback : FileData
    {
        AudioPlayback()
        {
            memset(&m_sound, 0, sizeof(ma_sound));
            memset(&m_decoder, 0, sizeof(ma_decoder));
        }

        ma_sound m_sound;
        ma_decoder m_decoder;
    };

    struct AudioEngineParams
    {
        // Main engine
        ma_engine m_engine;

        // Playback sounds list
        vector<AudioPlayback> m_vtAudioPlayback;

        // Recording params
        ma_device_config m_deviceConfig;
        ma_device m_device;
        drwav m_wav;
    };

    //Single Instances
    unique_ptr<AudioEngineParams> gAudioEngineParams;
    unique_ptr<CAudioEngine> gAudioEngine;


    // Audio recording callback function
    void capture_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
    {
        drwav* pWav = (drwav*)pDevice->pUserData;
        // MA_ASSERT(pWav != NULL);
        assert(pWav != NULL);

        drwav_write_pcm_frames(pWav, frameCount, pInput);

        (void)pOutput;
    }
}

int CAudioEngine::PlayAudio()
{
    m_bRecord = false;
    m_fileName = "TestPlayback.mp3";

    if (!gAudioEngineParams->m_engine.ownsDevice)
        InitializePlayback();

    return ResumePlayback();
}

int CAudioEngine::PlayAudios(int nCount)
{
    if (gAudioEngineParams->m_vtAudioPlayback.empty())
    {
        for (int i = 0; i < nCount; ++i)
        {
            string fName = "TestAudio_";
            fName.append(to_string(i + 1));
            fName.append(".mp3");

            ifstream ifs(fName.c_str(), ios::in | ios::binary);
            if (ifs.is_open())
            {
                AudioPlayback audioPlayback;
                audioPlayback.nSize = filesystem::file_size(fName);
                audioPlayback.pData = new char[audioPlayback.nSize];

                ifs.read(audioPlayback.pData, audioPlayback.nSize);

                gAudioEngineParams->m_vtAudioPlayback.push_back(audioPlayback);
                int nSize = gAudioEngineParams->m_vtAudioPlayback.size() - 1;

                PrepareAudio(&gAudioEngineParams->m_vtAudioPlayback[nSize]);
            }
            ifs.close();
        }
    }
    return ResumePlayback();
}

int CAudioEngine::PausePlayback()
{
    ma_result result = MA_ERROR;
    // Pause any sound, if present
    for (auto& audio : gAudioEngineParams->m_vtAudioPlayback)
    {
        if (audio.m_sound.pDataSource && ma_sound_is_playing(&audio.m_sound))
            result = ma_sound_stop(&audio.m_sound);
    }

    return result;
}

int CAudioEngine::ResumePlayback()
{
    ma_result result = MA_ERROR;
    // Resume any sound, if present
    for (auto& audio : gAudioEngineParams->m_vtAudioPlayback)
    {
        if (audio.m_sound.pDataSource && !ma_sound_is_playing(&audio.m_sound))
            result = ma_sound_start(&audio.m_sound);
    }

    return result;
}

int CAudioEngine::Forward10s()
{
    ma_result result = MA_ERROR;
    ma_uint64 nFrameRate = ma_engine_get_sample_rate(&gAudioEngineParams->m_engine); // / ma_engine_get_channels(&gAudioEngineParams->m_engine);

    for (auto& audio : gAudioEngineParams->m_vtAudioPlayback)
    {
        if (ma_sound_is_playing(&audio.m_sound))
        {
            ma_uint64 nFrameIndex = 0;
            if (MA_SUCCESS == ma_sound_get_cursor_in_pcm_frames(&audio.m_sound, &nFrameIndex))
            {
                ma_uint64 nTotalFrames = 0;
                ma_sound_get_length_in_pcm_frames(&audio.m_sound, &nTotalFrames);

                nFrameIndex += nFrameRate * 10; // adding 10s
                if (nFrameIndex > nTotalFrames)
                    nFrameIndex = nTotalFrames;

                result = ma_sound_seek_to_pcm_frame(&audio.m_sound, nFrameIndex);
            }
        }
    }
    return result;
}

int CAudioEngine::Backward10s()
{
    ma_result result = MA_ERROR;

    ma_uint64 nFrameRate = ma_engine_get_sample_rate(&gAudioEngineParams->m_engine); // / ma_engine_get_channels(&gAudioEngineParams->m_engine);

    for (auto& audio : gAudioEngineParams->m_vtAudioPlayback)
    {
        if (ma_sound_is_playing(&audio.m_sound))
        {
            ma_uint64 nFrameIndex = 0;
            if (MA_SUCCESS == ma_sound_get_cursor_in_pcm_frames(&audio.m_sound, &nFrameIndex))
            {
                ma_uint64 nSecToFrames = nFrameRate * 10;
                if (nFrameIndex > nSecToFrames)
                {
                    nFrameIndex -= nSecToFrames; // Taking off 10s
                    result = MA_SUCCESS;
                }
                else
                    nFrameIndex = 0;

                ma_sound_seek_to_pcm_frame(&audio.m_sound, nFrameIndex);
            }
        }
    }
    return result;
}

int CAudioEngine::StartRecording()
{
    m_bRecord = true;
    ma_result result;
    drwav_data_format wavFormat;

    wavFormat.container = drwav_container_riff;
    wavFormat.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    wavFormat.channels = 2;
    wavFormat.sampleRate = 44100;
    wavFormat.bitsPerSample = 32;

    if (drwav_init_file_write(&gAudioEngineParams->m_wav, "TestRecording.wav", &wavFormat, NULL) == DRWAV_FALSE) {
        printf("Failed to initialize output file.\n");
        return -1;
    }

    gAudioEngineParams->m_deviceConfig = ma_device_config_init(ma_device_type_capture);
    gAudioEngineParams->m_deviceConfig.capture.format = ma_format_f32;
    gAudioEngineParams->m_deviceConfig.capture.channels = wavFormat.channels;
    gAudioEngineParams->m_deviceConfig.sampleRate = wavFormat.sampleRate;
    gAudioEngineParams->m_deviceConfig.dataCallback = capture_data_callback;
    gAudioEngineParams->m_deviceConfig.pUserData = &gAudioEngineParams->m_wav;

    result = ma_device_init(NULL, &gAudioEngineParams->m_deviceConfig, &gAudioEngineParams->m_device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
        return -2;
    }

    result = ma_device_start(&gAudioEngineParams->m_device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&gAudioEngineParams->m_device);
        printf("Failed to start device.\n");
        return -3;
    }

    return 0;
}

int CAudioEngine::PlayRecording()
{
    m_fileName = "TestRecording.wav";

    if (!gAudioEngineParams->m_engine.ownsDevice)
        InitializePlayback();

    return ResumePlayback();
}

int CAudioEngine::Quit()
{
    if (m_bRecord)
        drwav_uninit(&gAudioEngineParams->m_wav);

    if (m_bRecord)
    {
        ma_device_uninit(&gAudioEngineParams->m_device);
        drwav_uninit(&gAudioEngineParams->m_wav);
        m_bRecord = false;
    }

    // Reset any sound, if present
    for (auto& audio : gAudioEngineParams->m_vtAudioPlayback)
    {
        if (audio.m_sound.pDataSource)
        {
            ma_sound_uninit(&audio.m_sound);
            audio.m_sound.ownsDataSource = 0;
            ma_decoder_uninit(&audio.m_decoder);
        }
        if (audio.pData)
            delete[] audio.pData;
    }
    gAudioEngineParams->m_vtAudioPlayback.clear();

    return 0;
}

int CAudioEngine::InitializePlayback()
{
    ma_result result;

    result = ma_engine_init(NULL, &gAudioEngineParams->m_engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return -1;
    }

    result = ma_sound_init_from_file(&gAudioEngineParams->m_engine, m_fileName.c_str(), MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &gAudioEngineParams->m_vtAudioPlayback[0].m_sound);
    if (result != MA_SUCCESS) {
        return result;
    }

    //ma_sound_get_length_in_pcm_frames(&gAudioEngineParams->m_vtAudioPlayback[0].m_sound, &m_nTotalFrames);

    return result;
}

int CAudioEngine::PrepareAudio(void* pItem)
{
    ma_result result = MA_SUCCESS;
    AudioPlayback* pAudioPlayback = (AudioPlayback*)pItem;

    //Initialize only if needed
    if (!(pAudioPlayback)->m_sound.pDataSource && pAudioPlayback->pData)
    {
        result = ma_decoder_init_memory(pAudioPlayback->pData, pAudioPlayback->nSize, NULL, &pAudioPlayback->m_decoder);
        if (!result)
            result = ma_sound_init_from_data_source(&gAudioEngineParams->m_engine, &pAudioPlayback->m_decoder,
                MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, &pAudioPlayback->m_sound);
    }
    return result;
}

CAudioEngine& AudioEngine()
{
    if (!gAudioEngine)
    {
        gAudioEngine = make_unique<CAudioEngine>();
        gAudioEngineParams = make_unique<AudioEngineParams>();
        ma_engine_init(NULL, &gAudioEngineParams->m_engine);
    }

    return *gAudioEngine.get();
}

#endif // USE_AUDIO_ENGINE