#include "pch.h"
#include "AudioHelper.h"

#ifndef USE_AUDIO_ENGINE

#include <stdio.h>
#include <stdlib.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <string>

namespace
{
    ma_decoder m_decoder;
    ma_device_config m_deviceConfig;
    ma_device m_device;
    ma_uint64 m_nTotalFrames;
    drwav m_wav;
    std::string m_fileName;
}

void play_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

void capture_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    drwav* pWav = (drwav*)pDevice->pUserData;
    //MA_ASSERT(pWav != NULL);
    assert(pWav != NULL);

    drwav_write_pcm_frames(pWav, frameCount, pInput);

    (void)pOutput;
}

int CAudioHelper::SimplePlayback()
{
    m_bPlayback = true;
    m_fileName = "TestPlayback.mp3";

    if (!m_decoder.outputSampleRate)
        InitializePlayback();

    return ResumePlayback();

}

int CAudioHelper::PausePlayback()
{
    if (ma_device_is_started(&m_device) &&
        ma_device_stop(&m_device) != MA_SUCCESS) {
        printf("Failed to stop playback device.\n");
        ma_device_uninit(&m_device);
        ma_decoder_uninit(&m_decoder);
        return -4;
    }
    return 0;
}

int CAudioHelper::ResumePlayback()
{
    if (!ma_device_is_started(&m_device) &&
        ma_device_start(&m_device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&m_device);
        ma_decoder_uninit(&m_decoder);
        return -4;
    }

    return 0;
}

int CAudioHelper::SimpleCapture()
{
    m_bPlayback = false;

    ma_result result;
    drwav_data_format wavFormat;

    wavFormat.container = drwav_container_riff;
    wavFormat.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    wavFormat.channels = 2;
    wavFormat.sampleRate = 44100;
    wavFormat.bitsPerSample = 32;

    if (drwav_init_file_write(&m_wav, "TestRecording.wav", &wavFormat, NULL) == DRWAV_FALSE) {
        printf("Failed to initialize output file.\n");
        return -1;
    }

    m_deviceConfig = ma_device_config_init(ma_device_type_capture);
    m_deviceConfig.capture.format = ma_format_f32;
    m_deviceConfig.capture.channels = wavFormat.channels;
    m_deviceConfig.sampleRate = wavFormat.sampleRate;
    m_deviceConfig.dataCallback = capture_data_callback;
    m_deviceConfig.pUserData = &m_wav;

    result = ma_device_init(NULL, &m_deviceConfig, &m_device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
        return -2;
    }

    result = ma_device_start(&m_device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&m_device);
        printf("Failed to start device.\n");
        return -3;
    }

    return 0;
}

int CAudioHelper::Forward10s()
{
    if (ma_device_is_started(&m_device))
    {
        ma_uint64 nFrameIndex = 0;
        ma_uint64 nFrameRate = m_decoder.outputSampleRate / m_decoder.outputChannels;
        if (MA_SUCCESS == ma_decoder_get_cursor_in_pcm_frames(&m_decoder, &nFrameIndex))
        {

            nFrameIndex += nFrameRate * 10; // adding 10s

            if (nFrameIndex > m_nTotalFrames)
                nFrameIndex = m_nTotalFrames;

            ma_decoder_seek_to_pcm_frame(&m_decoder, nFrameIndex);
        }

    }
    return 0;
}

int CAudioHelper::Backward10s()
{
    if (ma_device_is_started(&m_device))
    {
        ma_uint64 nFrameIndex = 0;
        ma_uint64 nFrameRate = m_decoder.outputSampleRate / m_decoder.outputChannels;
        if (MA_SUCCESS == ma_decoder_get_cursor_in_pcm_frames(&m_decoder, &nFrameIndex))
        {
            if (nFrameIndex > nFrameRate * 10)
                nFrameIndex -= nFrameRate * 10;
            else
                nFrameIndex = 0;

            ma_decoder_seek_to_pcm_frame(&m_decoder, nFrameIndex);
        }
    }
    return 0;
}

int CAudioHelper::Quit()
{
    printf("Press Enter to quit...");
    ma_device_uninit(&m_device);
    if (m_bPlayback)
    {
        ma_decoder_uninit(&m_decoder);
        memset(&m_decoder, 0, sizeof(ma_decoder));
    }
    else
        drwav_uninit(&m_wav);

    return 0;
}

int CAudioHelper::PlayRecording()
{
    m_bPlayback = true;
    m_fileName = "TestRecording.wav";

    if (!m_decoder.outputSampleRate)
        InitializePlayback();

    if (ma_device_start(&m_device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&m_device);
        ma_decoder_uninit(&m_decoder);
        return -4;
    }

    return 0;
}

int CAudioHelper::InitializePlayback()
{
    ma_result result;

    result = ma_decoder_init_file(m_fileName.c_str(), NULL, &m_decoder);
    if (result != MA_SUCCESS) {
        return -2;
    }

    m_deviceConfig = ma_device_config_init(ma_device_type_playback);
    m_deviceConfig.playback.format = m_decoder.outputFormat;
    m_deviceConfig.playback.channels = m_decoder.outputChannels;
    m_deviceConfig.sampleRate = m_decoder.outputSampleRate;
    m_deviceConfig.dataCallback = play_data_callback;
    m_deviceConfig.pUserData = &m_decoder;
    /** Decode and send frames to the soundcard in chunks of 100ms of data */
    m_deviceConfig.periodSizeInMilliseconds = 100;

    if (ma_device_init(NULL, &m_deviceConfig, &m_device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&m_decoder);
        return -3;
    }

    ma_decoder_get_length_in_pcm_frames(&m_decoder, &m_nTotalFrames);

    return result;
}

#endif // !USE_AUDIO_ENGINE
