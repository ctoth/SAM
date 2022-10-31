#ifndef __CLI_H__
#define __CLI_H__
typedef struct SAMAudioBuffer
{
    char *buffer;
    int length;
} SAMAudioBuffer;
void output_callback(void *userdata, char *buffer, unsigned int length);
void    output_audio_finished_callback(void *userdata);
void write_wav_finished_callback(void *userdata);
#endif // __CLI_H__