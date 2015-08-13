<p align="center">
    <b>Guitar and Bass Tuner Library</b><br>
    <b>Teensy 3.1 v2.0</b><br>
</p>

>Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm can be some what memory and processor hungry but will allow you to detect with fairly good accuracy the fundamental frequencies f<sub>o</sub> from electric guitars and basses. 
>>You can install this as a normal Arduino Library and will work with the Audio Library, no need to edit the Audio libraries source now.

<!-- language: lang-none -->
      Hookup Guide - 1.2v DC Bias and High Pass Filter - No Amplification

                       *--------------------------------------------------*   
                       |                                                  |
                       |      Pull Down Resistor            | ' |         |
                       *------------/\/\/\-------------*    |' '|         | 
                       |             47K               |   _|_'_|_        |
                       |                               |  |` ` ` `|       |
                       *---)|+---*                     |  | ` ` ` |       |
                       |  10uF   |                     |  |` ` ` `|       |
       TEENSY 3.1      |         |                     |  | ` ` ` |       |
     _______________   *-/\/\/\--*                     |  |` ` ` `|       |
    |GND |_____| Vin|  |  2.2K   |                     |  | ` ` ` |       |
    |0  -----   AGND|<-*         |                     |  |` ` ` `|       |
    |1 |`````|  3.3V|>---/\/\/\--*--/\/\/\--*          |  | ` ` ` |       |
    |2 |     |    23|     10K        47K    |          |  |` ` ` `|       |
    |3  -----     22|                       |          |  | ` ` ` |       |
    |4 |'|        21|                       |          |   \=====/        |
    |5   ------   20|                       |          |    | '`|         |
    |6 |::::::::| 19|                       |  REMOVE  |    | S`|         |
    |7 |::::::::| 18|                       |    DC    |    | H`|         |
    |8 |::::::::| 17|                       |   BIAS   |    | I`|         |
    |9   ------A2/16|<---SIGNAL-1.2v-BIAS---*---+|(----*    | E`|>--ANGD--*
    |10    ---    15|                    1.2VDC 10uF   |    | L`|
    |11   |(`)|   14|                                  |    | D`|
    |12    ---    13|                                  |    |  `|
     ---------------                                   |    |===|
                                                       |     \_/
                                                       |     /T\
                                                       |    .-I-.
                                                       *---<\ P /
                                                             \_/

>Many optimizations have been done to the [YIN] algorithm for frequencies between 29-360Hz. 
>>While its still using a brute force method ( n<sup>2</sup> ) for finding the fundamental frequency f<sub>o</sub>, it is tuned to skip certain <b>tau</b> (<img src="http://latex.numberempire.com/render?%5Cinline%20%5Ctau__&sig=54bf27412fc70491a83bb73238395a62"/></img>) values and focus mostly on frequencies found in the bass and guitar. 
>>>The input is double buffered so while you are processing one buffer it is filling the other to double throughput. 
>>>>There are a few parameters that can be adjusted to "dial in" the algorithm for better estimations. The defaults are what I found that have the best trade off for speed and accuracy.

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
SAMPLE_RATE_DIVIDE_BY_x --> This sets 'SAMPLE_SKIP' to pass on every (x) data point from the 
Audio Block being saved to the buffer, it determines the sample rate.
```

```
NUM_SAMPLES --> This the size of each buffer, there two for double buffering.
```
```
SAMPLE_SKIP --> This sets your sample window length and sampling rate. Sample Window Size is
(NUM_SAMPLES * SAMPLE_SKIP) of the ~44100 samples every second. Sample Rate is 
(AUDIO_SAMPLE_RATE_EXACT / SAMPLE_SKIP). 
```

<div>
<b>YIN Algorithm</b>
<ol>
<li><b>Difference Function -</b> Squared difference of the (signal - signal lag(<img src="http://latex.numberempire.com/render?%5Cinline%20%5Ctau__&sig=54bf27412fc70491a83bb73238395a62"/></img>))<br>
<img src="http://latex.numberempire.com/render?%5Cinline%20d_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%20%3D%20%5Csum_%7Bj%3D1%7D%5EW%20%20%5Cbig%28x_%7Bj%7D-x_%7Bj%2B%5Ctau%7D%5Cbig%29%5E%7B2%7D&sig=0c376ade6bf9ddc6c58fef19e394ac5f" /></li>

<li><b>Cumulative Mean Normalized Difference Function -</b> Calculate the cumulative mean on the normalized difference signal<br><img src="http://latex.numberempire.com/render?%5Cinline%20d%27_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%20%3D%5Cbegin%7Bcases%7D1%2C%20%26%20%5Ctau%20%3D%200%5C%5Cd_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%5Cdiagup%20%26%5Cleft%5B%28%5Cfrac%7B1%7D%7B%5Ctau%7D%29%5Csum_%7Bj%3D1%7D%5E%5Ctau%20%20d_%7Bt%7D%20%28j%29%5Cright%5D%5Cend%7Bcases%7D&sig=3b3f85a5f10ea6ea256befa8e562d794" /></li>
<li><b>Absolute Threshold -</b> Fix for subharmonic error with Autocorrelation, choose (<img src="http://latex.numberempire.com/render?%5Cinline%20%5Ctau__&sig=54bf27412fc70491a83bb73238395a62"/></img>) that gives a minimum <img src="http://latex.numberempire.com/render?%5Cinline%20d%27_&sig=74986652162ec852bf82f79d1e2214c3"/></li>
</ol>
</div>

[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf