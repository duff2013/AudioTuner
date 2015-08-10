#Guitar and Bass Tuner Library 
#Teensy 3.1 v2.0

>Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm can be some what memory and processor hungry but will allow you to detect with fairly good accuracy the fundamental frequencies from electric guitars and basses. You can install this as a normal Arduino Library and will work with the Audio Library, no need to edit the Audio libraries source now.

>Many optimizations have been done to the [YIN] algorithm for frequencies between 29-360Hz. While its still using a brute force method ( n<sup>2</sup> ) for finding fundamental frequency f(o) it is tuned to skip <b>tau</b> values that are out of its frequency range and focus mostly on frequencies found in the bass and guitar. The input is double buffered so while you are processing one buffer it is filling the other to double throuput. There are a few parameters that can be adjusted to "dial in" the algorithm for better estimations. The defaults are what I found that have the best trade off for speed and accuracy. 

<h4>AudioTuner.h</h4>
```
/****************************************************************/
#define SAMPLE_RATE_DIVIDE_BY_1 1      // 44100    sample rate
#define SAMPLE_RATE_DIVIDE_BY_2 2      // 22050    sample rate
#define SAMPLE_RATE_DIVIDE_BY_4 4      // 11025    sample rate
#define SAMPLE_RATE_DIVIDE_BY_8 8      // 5512.5   sample rate
#define SAMPLE_RATE_DIVIDE_BY_16 16    // 2756.25  sample rate
#define SAMPLE_RATE_DIVIDE_BY_32 32    // 1378.125 sample rate
/****************************************************************
*              Safe to adjust these values below               *
****************************************************************/
// Adjust number of samples to collect in buffer here, also effects
// convergence speed and resolution.
#define NUM_SAMPLES 2048 // make a power of two

// larger the divide-by, less resolution and lower the frequency for
// a given number of samples that can be detected. Also effects
// convergence speed.
#define SAMPLE_SKIP SAMPLE_RATE_DIVIDE_BY_2
/****************************************************************/
```

```
SAMPLE_RATE_DIVIDE_BY_x --> This sets 'SAMPLE_SKIP' to pass on every (x) data piont from the 
Audio Block being saved to the buffer, it determines the sample rate.
```

```
NUM_SAMPLES --> This the size of each buffer, there two for double buffering.
```
```
SAMPLE_SKIP --> This sets your sample window lenght and sampling rate. Sample Window Size is
(NUM_SAMPLES * SAMPLE_SKIP) of the ~44100 samples every second. Sample Rate is 
(AUDIO_SAMPLE_RATE_EXACT / SAMPLE_SKIP). 
```
<!-- language: lang-none -->
      Hookup Guide - 1.2v DC Bias and High Pass Filter - No Amplification

                       *--------------------------------------------------*   
                       |                                                  |
                       |                                    | ' |         |
                       *------------/\/\/\-------------*    |' '|         | 
                       |             47K               |   _|_'_|_        |
                       |                               |  |` ` ` `|       |
                       *---)|+--*                      |  | ` ` ` |       |
                       |  10uF  |                      |  |` ` ` `|       |
                       |        |                      |  | ` ` ` |       |
     _______________   *-/\/\/\-*                      |  |` ` ` `|       |
    |GND  |___|  Vin|  |  2.2K  |                      |  | ` ` ` |       |
    |0      T   AGND|<-*        |                      |  |` ` ` `|       |
    |1      E   3.3V|>--/\/\/\--*--/\/\/\---*          |  | ` ` ` |       |
    |2      E     23|    10K        47K     |          |  |` ` ` `|       |
    |3      N     22|                       |          |  | ` ` ` |       |
    |4      S     21|                       |          |   \_`_`_/        |
    |5      Y     20|                       |          |    | :`|         |
    |6     3.1    19|                       |          |    | S`|         |
    |7            18|                       |  REMOVE  |    | H`|         |
    |8            17|                       |    DC    |    | I`|         |
    |9         A2/16|<---SIGNAL-1.2v-BIAS---*---+|(----*    | E`|>--ANGD--*
    |10    ---    15|                     1.2V  10uF   |    | L`|
    |11   |(`)|   14|                      DC          |    | D`|
    |12    ---    13|                                  |    | :`|
     ---------------                                   |    |===|
                                                       |     \_/
                                                       |     /T\
                                                       |    - I -
                                                       *---<\ P /
                                                             \_/



[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf