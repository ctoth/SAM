#ifndef __SAM_H__
#define __SAM_H__

// A struct with the input to speak and a callback to be called when speaking completes
// includes an output callback
// also includes parameters for voice

typedef struct SAMUtterance
{
    char *input;
    // callback called to output audio
    void (*output_callback)(void *userdata, char *buffer, unsigned int length);

    // callback called when speaking is complete
    void (*finished_callback)(void *userdata);
    unsigned char speed;
    unsigned char pitch;
    unsigned char mouth;
    unsigned char throat;
    int singmode;
    // callback userdata
    void *userdata;
} SAMUtterance;

// a struct containing shared context for SAM
typedef struct SAMContext
{

    SAMUtterance toSpeak;
    unsigned char stress[256];        // numbers from 0 to 8
    unsigned char phonemeLength[256]; // tab40160
    unsigned char phonemeIndex[256];

    unsigned char phonemeIndexOutput[60];  // tab47296
    unsigned char stressOutput[60];        // tab47365
    unsigned char phonemeLengthOutput[60]; // tab47416

    unsigned char pitches[256];
    unsigned char sampledConsonantFlag[256]; // tab44800
    unsigned char amplitude1[256];
    unsigned char amplitude2[256];
    unsigned char amplitude3[256];
    unsigned char frequency1[256];
    unsigned char frequency2[256];
    unsigned char frequency3[256];

    unsigned char freq1data[80];
    unsigned char freq2data[80];

    unsigned oldTimeTableIndex;
} SAMContext;

typedef enum
{
    pR = 23,
    pD = 57,
    pT = 69,
    BREAK = 254,
    END = 255
} Phonemes;

void PrintPhonemes(SAMContext *ctx);
void SAMInit(SAMContext *ctx);

void PrepareOutput(SAMContext *ctx);
void SAMSpeak(SAMUtterance *toSpeak);
void SAMFree(SAMContext *ctx);

#endif // __SAM_H__