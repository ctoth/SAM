
#include "cli.h"
#include "sam.h"
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
    toSpeak.input = "Fuck off, world";
    toSpeak.callback = &callback;
    SAMSpeak(&toSpeak);
    return 0;
}

void callback(void *userdata, char *buffer, int length)

{
    // print some data bout what we were given
    printf("userdata: %p, buffer: %p, length: %d\n", userdata, buffer, length);
    WriteWav("out.wav", buffer, length/50);
}