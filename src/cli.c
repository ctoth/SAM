
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
    toSpeak.input = "I am SAM, the Software Automatic Mouth!";
    toSpeak.callback = &callback;
    SAMSpeak(&toSpeak);
    return 0;
}

void callback(void *userdata, char *buffer, int length)
{
    // print some data bout what we were given
    WriteWav("out.wav", buffer, length / 50);
}