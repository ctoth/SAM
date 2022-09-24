#include <stdio.h>

void WriteWav(char *filename, char *buffer, int bufferlength)
{
    unsigned int filesize;
    unsigned int fmtlength = 16;
    unsigned short int format = 1; // PCM
    unsigned short int channels = 1;
    unsigned int samplerate = 22050;
    unsigned short int blockalign = 1;
    unsigned short int bitspersample = 8;

    FILE *file;
    fopen_s(&file, filename, "wb");
    if (file == NULL)
        return;
    // RIFF header
    fwrite("RIFF", 4, 1, file);
    filesize = bufferlength + 12 + 16 + 8 - 8;
    fwrite(&filesize, 4, 1, file);
    fwrite("WAVE", 4, 1, file);

    // format chunk
    fwrite("fmt ", 4, 1, file);
    fwrite(&fmtlength, 4, 1, file);
    fwrite(&format, 2, 1, file);
    fwrite(&channels, 2, 1, file);
    fwrite(&samplerate, 4, 1, file);
    fwrite(&samplerate, 4, 1, file); // bytes/second
    fwrite(&blockalign, 2, 1, file);
    fwrite(&bitspersample, 2, 1, file);

    // data chunk
    fwrite("data", 4, 1, file);
    fwrite(&bufferlength, 4, 1, file);
    fwrite(buffer, bufferlength, 1, file);

    fclose(file);
}
