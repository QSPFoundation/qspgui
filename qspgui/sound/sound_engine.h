// Copyright (C) 2001-2024 Val Argunov (byte AT qsp DOT org)
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

#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

    #include <stddef.h>

    typedef void *ma_sound_file;

    #ifdef __cplusplus
    extern "C"
    {
    #endif

    int sound_init_engine();
    void sound_free_engine();

    int soundfont_init(const char *filePath);
    int soundfont_init_w(const wchar_t *filePath);

    ma_sound_file sound_play_file(const char *file);
    ma_sound_file sound_play_file_w(const wchar_t *file);
    void sound_close_file(ma_sound_file sound);
    void sound_set_volume(ma_sound_file sound, float volume);
    int sound_is_playing(ma_sound_file sound);

    #ifdef __cplusplus
    }
    #endif

#endif
