// Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org)
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "sound_engine.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

/* stb_vorbis implementation goes after the implementation of miniaudio */
#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_IMPLEMENTATION
#include "tml.h"

#include <stdio.h>

typedef struct
{
    ma_data_source_base ds; /* the base data source */
    ma_read_proc onRead;
    ma_seek_proc onSeek;
    ma_tell_proc onTell;
    void *pReadSeekTellUserData;
    ma_format format; /* will be either f32 or s16 */
    tsf *sf;
    tml_message *midi_init_message;
    tml_message *midi_message;
    double msecs;
} ma_libtsf;

typedef struct
{
    void *buf;
    int size;
} tsf_soundfont;

static tsf_soundfont ma_current_soundfont;
static ma_engine ma_current_engine;
static ma_resource_manager ma_current_resource_manager;

static int soundfont_load(FILE *file, tsf_soundfont *sf);
static void soundfont_free(tsf_soundfont *sf);
static ma_result ma_midi_ds_read(ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead);
static ma_result ma_midi_ds_get_data_format(ma_data_source *pDataSource, ma_format *pFormat, ma_uint32 *pChannels, ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap);
static int stream_callback_read(void *pDataSource, void *ptr, unsigned int size);
static tml_message *tml_load_by_callback(ma_libtsf *tsf);
static ma_result ma_decoding_backend_init_midi(void *pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void *pReadSeekTellUserData, const ma_decoding_backend_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_data_source **ppBackend);
static void ma_decoding_backend_uninit_midi(void *pUserData, ma_data_source *pBackend, const ma_allocation_callbacks *pAllocationCallbacks);

static ma_data_source_vtable g_ma_midi_ds_vtable =
{
    ma_midi_ds_read, /* ma_result (* onRead)(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead); */
    NULL, /* ma_result (* onSeek)(ma_data_source* pDataSource, ma_uint64 frameIndex); */
    ma_midi_ds_get_data_format, /* ma_result (* onGetDataFormat)(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap); */
    NULL, /* ma_result (* onGetCursor)(ma_data_source* pDataSource, ma_uint64* pCursor); */
    NULL, /* ma_result (* onGetLength)(ma_data_source* pDataSource, ma_uint64* pLength); */
    NULL /* ma_result (* onSetLooping)(ma_data_source* pDataSource, ma_bool32 isLooping); */
};

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_midi =
{
    ma_decoding_backend_init_midi,
    NULL, /* onInitFile() */
    NULL, /* onInitFileW() */
    NULL, /* onInitMemory() */
    ma_decoding_backend_uninit_midi
};

static int soundfont_load(FILE *file, tsf_soundfont *sf)
{
    int len;
    void *data;
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    data = malloc(len);
    if (data == NULL) return -1;
    len = fread(data, 1, len, file);
    sf->buf = data;
    sf->size = len;
    return 0;
}

static void soundfont_free(tsf_soundfont *sf)
{
    if (sf->buf) free(sf->buf);
}

static ma_result ma_midi_ds_read(ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead)
{
    ma_uint64 samplesRead = 0;
    ma_libtsf *tsf = (ma_libtsf *)pDataSource;
    unsigned char *outputStream = (unsigned char *)pFramesOut;
    double msecs = tsf->msecs;
    int channels = (tsf->sf->outputmode == TSF_MONO) ? 1 : 2;
    float sampleRate = tsf->sf->outSampleRate;
    tml_message *midiMessage = tsf->midi_message;

    if (tsf->sf == NULL)
        return MA_ERROR;

    while (frameCount > 0 && midiMessage)
    {
        int sampleBlock = frameCount < TSF_RENDER_EFFECTSAMPLEBLOCK ? frameCount : TSF_RENDER_EFFECTSAMPLEBLOCK;
        msecs += (1000.0 / sampleRate) * sampleBlock;

        while (midiMessage)
        {
            if (midiMessage->type == TML_EOT) // end of track
            {
                midiMessage = NULL;
                break;
            }

            if (midiMessage->time > msecs)
                break;

            unsigned char channel = midiMessage->channel;
            switch (midiMessage->type)
            {
            case TML_PROGRAM_CHANGE:
                tsf_channel_set_presetnumber(tsf->sf, channel, midiMessage->program, (channel == 9));
                break;
            case TML_NOTE_ON:
                tsf_channel_note_on(tsf->sf, channel, midiMessage->key, (float)midiMessage->velocity / 127.0f);
                break;
            case TML_NOTE_OFF:
                tsf_channel_note_off(tsf->sf, channel, midiMessage->key);
                break;
            case TML_PITCH_BEND:
                tsf_channel_set_pitchwheel(tsf->sf, channel, midiMessage->pitch_bend);
                break;
            case TML_CONTROL_CHANGE:
                tsf_channel_midi_control(tsf->sf, channel, midiMessage->control, midiMessage->control_value);
                break;
            }

            midiMessage = midiMessage->next;
        }

        if (tsf->format == ma_format_f32)
        {
            tsf_render_float(tsf->sf, (float *)outputStream, sampleBlock, 0);
            outputStream += sampleBlock * channels * sizeof(float);
        }
        else
        {
            tsf_render_short(tsf->sf, (short *)outputStream, sampleBlock, 0);
            outputStream += sampleBlock * channels * sizeof(short);
        }

        frameCount -= sampleBlock;
        samplesRead += sampleBlock;
    }

    tsf->midi_message = midiMessage;
    tsf->msecs = msecs;
    if (pFramesRead != NULL)
        *pFramesRead = samplesRead;

    return midiMessage ? MA_SUCCESS : MA_AT_END;
}

static ma_result ma_midi_ds_get_data_format(ma_data_source *pDataSource, ma_format *pFormat, ma_uint32 *pChannels, ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap)
{
    ma_libtsf *tsf = (ma_libtsf *)pDataSource;
    if (tsf->sf == NULL)
        return MA_ERROR;

    *pFormat = tsf->format;
    *pChannels = (tsf->sf->outputmode == TSF_MONO) ? 1 : 2;
    *pSampleRate = (ma_uint32)tsf->sf->outSampleRate;
    if (pChannelMap != NULL)
        ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, *pChannels);
    return MA_SUCCESS;
}

static int stream_callback_read(void *pDataSource, void *ptr, unsigned int size)
{
    ma_libtsf *tsf = (ma_libtsf *)pDataSource;
    size_t bytesRead = 0;
    ma_result result = tsf->onRead(tsf->pReadSeekTellUserData, ptr, size, &bytesRead);
    if (result != MA_SUCCESS)
        return 0;
    return bytesRead;
}

static tml_message *tml_load_by_callback(ma_libtsf *tsf)
{
    struct tml_stream stream;
    stream.read = stream_callback_read;
    stream.data = tsf;
    return tml_load(&stream);
}

static ma_result ma_decoding_backend_init_midi(void *pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void *pReadSeekTellUserData, const ma_decoding_backend_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_data_source **ppBackend)
{
    ma_result result;
    ma_libtsf *tsf;
    ma_data_source_config dataSourceConfig;
    if (onRead == NULL)
        return MA_INVALID_ARGS;

    tsf = (ma_libtsf *)ma_malloc(sizeof(ma_libtsf), pAllocationCallbacks);
    if (tsf == NULL)
        return MA_OUT_OF_MEMORY;

    MA_ZERO_MEMORY(tsf, sizeof(ma_libtsf));
    tsf->format = ma_format_f32;

    if (pConfig != NULL && (pConfig->preferredFormat == ma_format_f32 || pConfig->preferredFormat == ma_format_s16))
        tsf->format = pConfig->preferredFormat;

    dataSourceConfig = ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_midi_ds_vtable;

    result = ma_data_source_init(&dataSourceConfig, &tsf->ds);
    if (result != MA_SUCCESS)
    {
        ma_free(tsf, pAllocationCallbacks);
        return result; /* failed to initialize the base data source. */
    }

    tsf->onRead = onRead;
    tsf->onSeek = onSeek;
    tsf->onTell = onTell;
    tsf->pReadSeekTellUserData = pReadSeekTellUserData;

    tsf->sf = NULL;
    tsf->midi_init_message = NULL;
    tsf->midi_message = NULL;
    tsf->msecs = 0.0;

    if (ma_current_soundfont.buf != NULL)
    {
        tsf->sf = tsf_load_memory(ma_current_soundfont.buf, ma_current_soundfont.size);
        if (tsf->sf == NULL)
        {
            ma_data_source_uninit(&tsf->ds);
            ma_free(tsf, pAllocationCallbacks);
            return MA_ERROR;
        }

        tsf_set_output(tsf->sf, TSF_STEREO_INTERLEAVED, ma_current_engine.sampleRate, 0.0f);

        tsf->midi_init_message = tml_load_by_callback(tsf);
        tsf->midi_message = tsf->midi_init_message;
    }

    *ppBackend = tsf;

    return MA_SUCCESS;
}

static void ma_decoding_backend_uninit_midi(void *pUserData, ma_data_source *pBackend, const ma_allocation_callbacks *pAllocationCallbacks)
{
    ma_libtsf *tsf = (ma_libtsf *)pBackend;
    if (tsf == NULL)
        return;

    if (tsf->midi_init_message != NULL)
        tml_free(tsf->midi_init_message);

    if (tsf->sf != NULL)
    {
        tsf_reset(tsf->sf);
        tsf_close(tsf->sf);
    }

    ma_data_source_uninit(&tsf->ds);
    ma_free(tsf, pAllocationCallbacks);
}

int sound_init_engine()
{
    ma_result result;
    ma_engine_config engineConfig;
    ma_resource_manager_config resourceManagerConfig;

    ma_decoding_backend_vtable *pCustomBackendVTables[] =
    {
        &g_ma_decoding_backend_vtable_midi
    };

    MA_ZERO_MEMORY(&ma_current_engine, sizeof(ma_engine));
    MA_ZERO_MEMORY(&ma_current_resource_manager, sizeof(ma_resource_manager));
    MA_ZERO_MEMORY(&ma_current_soundfont, sizeof(tsf_soundfont));

    resourceManagerConfig = ma_resource_manager_config_init();
    resourceManagerConfig.decodedFormat = ma_format_f32;
    resourceManagerConfig.ppCustomDecodingBackendVTables = pCustomBackendVTables;
    resourceManagerConfig.customDecodingBackendCount     = sizeof(pCustomBackendVTables) / sizeof(pCustomBackendVTables[0]);
    resourceManagerConfig.pCustomDecodingBackendUserData = NULL;

    result = ma_resource_manager_init(&resourceManagerConfig, &ma_current_resource_manager);
    if (result != MA_SUCCESS)
        return -1;

    engineConfig = ma_engine_config_init();
    engineConfig.defaultVolumeSmoothTimeInPCMFrames = 4096;
    engineConfig.pResourceManager = &ma_current_resource_manager;

    result = ma_engine_init(&engineConfig, &ma_current_engine);
    if (result != MA_SUCCESS)
    {
        ma_resource_manager_uninit(&ma_current_resource_manager);
        return -1;
    }

    return 0;
}

void sound_free_engine()
{
    ma_engine_uninit(&ma_current_engine);
    ma_resource_manager_uninit(&ma_current_resource_manager);
    soundfont_free(&ma_current_soundfont);
}

int soundfont_init(const char *filePath)
{
    int result;
    FILE *file;
    if (ma_fopen(&file, filePath, "rb") != MA_SUCCESS)
        return -1;
    result = soundfont_load(file, &ma_current_soundfont);
    fclose(file);
    return result;
}

int soundfont_init_w(const wchar_t *filePath)
{
    int result;
    FILE *file;
    if (ma_wfopen(&file, filePath, L"rb", NULL) != MA_SUCCESS)
        return -1;
    result = soundfont_load(file, &ma_current_soundfont);
    fclose(file);
    return result;
}

ma_sound_file sound_play_file(const char *file)
{
    ma_result result;
    ma_sound_file sound = (ma_sound_file)malloc(sizeof(ma_sound));
    if (sound == NULL)
        return NULL;

    MA_ZERO_MEMORY(sound, sizeof(ma_sound));
    result = ma_sound_init_from_file(&ma_current_engine, file, MA_SOUND_FLAG_NO_SPATIALIZATION, 0, 0, sound);
    if (result != MA_SUCCESS)
    {
        free(sound);
        return NULL;
    }
    result = ma_sound_start(sound);
    if (result != MA_SUCCESS)
    {
        ma_sound_uninit(sound);
        free(sound);
        return NULL;
    }

    return sound;
}

ma_sound_file sound_play_file_w(const wchar_t *file)
{
    ma_result result;
    ma_sound_file sound = (ma_sound_file)malloc(sizeof(ma_sound));
    if (sound == NULL)
        return NULL;

    MA_ZERO_MEMORY(sound, sizeof(ma_sound));
    result = ma_sound_init_from_file_w(&ma_current_engine, file, MA_SOUND_FLAG_NO_SPATIALIZATION, 0, 0, sound);
    if (result != MA_SUCCESS)
    {
        free(sound);
        return NULL;
    }
    result = ma_sound_start(sound);
    if (result != MA_SUCCESS)
    {
        ma_sound_uninit(sound);
        free(sound);
        return NULL;
    }

    return sound;
}

void sound_close_file(ma_sound_file sound)
{
    ma_sound_stop(sound);
    ma_sound_uninit(sound);
    free(sound);
}

void sound_set_volume(ma_sound_file sound, float volume)
{
    ma_sound_set_volume(sound, volume);
}

int sound_is_playing(ma_sound_file sound)
{
    return (ma_sound_is_playing(sound) == MA_TRUE);
}
