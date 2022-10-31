
#include "cli.h"
#include "sam.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "wav.h"

#include <stdio.h>
// main
int main(int argc, char *argv[])
{

    SAMUtterance toSpeak;
    // default SAM values
    toSpeak.speed = 72;
    toSpeak.pitch = 64;
    toSpeak.mouth = 128;
    toSpeak.throat = 128;
    toSpeak.input = "I am SAM, the Software Automatic Mouth!";
    toSpeak.output_callback = &output_callback;
    toSpeak.finished_callback = &output_audio_finished_callback;
    // toSpeak.finished_callback = &write_wav_finished_callback;
    // create audio buffer and set as userdata
    SAMAudioBuffer audioBuffer;
    audioBuffer.length = 0;
    audioBuffer.buffer = NULL;
    toSpeak.userdata = &audioBuffer;
    SAMSpeak(&toSpeak);
    return 0;
}

// audio output callback

void output_callback(void *userdata, char *buffer, unsigned int length)
{
    // userdata is a pointer to a SAMAudioBuffer
    SAMAudioBuffer *audioBuffer = (SAMAudioBuffer *)userdata;
    // if the buffer is null, allocate it
    if (audioBuffer->buffer == NULL)
    {
        audioBuffer->buffer = malloc(length);
        audioBuffer->length = length;
    }
    else
    {
        // otherwise, reallocate it
        audioBuffer->buffer = realloc(audioBuffer->buffer, audioBuffer->length + length);
        audioBuffer->length += length;
    }
    // copy the buffer into the audio buffer

    memcpy(audioBuffer->buffer + audioBuffer->length - length, buffer, length);
}

// finish callback

void write_wav_finished_callback(void *userdata)
{
    // userdata is actually a SAMAudioBuffer
    SAMAudioBuffer *audioBuffer = (SAMAudioBuffer *)userdata;
    printf("Writing wav file...\n");
    // write the buffer to a wav file
    WriteWav("out.wav", audioBuffer->buffer, audioBuffer->length);
}

void output_audio_finished_callback(void *userdata)
{
    // output audio with miniaudio
    // userdata is actually a SAMAudioBuffer
    SAMAudioBuffer *audioBuffer = (SAMAudioBuffer *)userdata;
    printf("Output buffer length: %d\n", audioBuffer->length);
    // set up miniaudio
    ma_engine engine;
    ma_result result;
    ma_engine_config engineConfig;
    engineConfig = ma_engine_config_init();
    engineConfig.sampleRate = 22050;
    engineConfig.channels = 1;
    result = ma_engine_init(&engineConfig, &engine);
    if (result != MA_SUCCESS)
    {
        printf("Failed to initialize miniaudio engine.\n");
        return;
    }
    // setup miniaudio buffer
    ma_audio_buffer_config config = ma_audio_buffer_config_init(ma_format_u8, 22050, audioBuffer->length, audioBuffer->buffer, NULL);
    config.channels = 1;

    ma_audio_buffer miniaudio_buffer;
    result = ma_audio_buffer_init(&config, &miniaudio_buffer);
    if (result != MA_SUCCESS)
    {
        printf("Failed to initialize miniaudio buffer.\n");
        return;
    }
    // play the buffer
    // create sound
    ma_sound sound;
    result = ma_sound_init_from_data_source(&engine, &miniaudio_buffer, 0, NULL, &sound);
    if (result != MA_SUCCESS)
    {
        printf("Failed to initialize miniaudio sound.\n");
        return;
    }
    // play the sound
    ma_sound_start(&sound);
    printf("Playing audio...\n");
    // wait for the sound to finish
    while (ma_sound_is_playing(&sound))
    {
        ma_sleep(100);
    }
    // clean up
    ma_sound_uninit(&sound);
    ma_audio_buffer_uninit(&miniaudio_buffer);
    ma_engine_uninit(&engine);
}