<p align="center">
    <b>Guitar and Bass Tuner Library</b><br>
    <b>Teensy 3.1/2 v2.1</b><br>
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
>>While its still using a brute force method ( n<sup>2</sup> ) for finding the fundamental frequency f<sub>o</sub>, it is tuned to skip certain <b>tau</b> (<img src="http://latex.numberempire.com/render?%5Cinline%20%5Chuge%20%5Cmathbf%7B%5Ctau%7D&sig=845639da85c0dd8e2de679817b06639c"/></img>) values and focus mostly on frequencies found in the bass and guitar. 
>>>The input is double buffered so while you are processing one buffer it is filling the other to double throughput. 
>>>>There are a few parameters that can be adjusted to "dial in" the algorithm for better estimations located in AudioTuner.h. The defaults below are what I found that have the best trade off for speed and accuracy.

<h4>AudioTuner.h</h4>

```
/****************************************************************/
#define SAMPLE_RATE_44100  1      // 44100    sample rate
#define SAMPLE_RATE_22050  2      // 22050    sample rate
#define SAMPLE_RATE_11025  4      // 11025    sample rate
/****************************************************************/

/****************************************************************
*              Safe to adjust these values below               *
*                                                              *
*  These two parameters define how this object works.          *
*                                                              *
*  1.  NUM_SAMPLES - Size of the buffer. Since object uses     *
*      double buffering this value will be 4x in bytes of      *
*      memory.  !!! Must be power of 2 !!!!                    *
*                                                              *
*  2.  SAMPLE_RATE - Just what it says.                        *
*                                                              *
*  These two parameters work hand in hand. For example if you  *
*  want a high sample rate but do not allocate enough buffer   *
*  space, you will be limit how low of a frequency you can     *
*  measure. If you then increase the buffer you use up         *
*  precious ram and slow down the system since it takes longer *
*  to processes the buffer.                                    *
*                                                              *
*  Play around with these values to find what best suits your  *
*  needs. The max number of buffers you can have is 8192 bins. *
****************************************************************/
// !!! Must be power of 2 !!!!
#define NUM_SAMPLES 2048 // make a power of two

// Use defined sample rates above^
#define SAMPLE_RATE SAMPLE_RATE_22050
/****************************************************************/
```

<div>
<b>YIN Algorithm</b>
<ol>
<li><b>Difference Function -</b> Squared difference of the (signal - signal lag(<img src="http://latex.numberempire.com/render?%5Cinline%20%5Chuge%20%5Cmathbf%7B%5Ctau%7D&sig=845639da85c0dd8e2de679817b06639c"/></img>))<br>
<img src="http://latex.numberempire.com/render?%5Chuge%20d_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%20%3D%20%5Csum_%7Bj%3D1%7D%5EW%20%20%5Cbig%28x_%7Bj%7D-x_%7Bj%2B%5Ctau%7D%5Cbig%29%5E%7B2%7D&sig=da6a2a10a134437679df399dbb9327df" /></li>

<li><b>Cumulative Mean Normalized Difference Function -</b> Calculate the cumulative mean on the normalized difference signal<br><img src="http://latex.numberempire.com/render?%5Cinline%20%5Chuge%20d%27_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%20%3D%5Cbegin%7Bcases%7D1%2C%20%26%20%5Ctau%20%3D%200%5C%5Cd_%7Bt%7D%20%5Cbig%28%20%5Ctau%20%5Cbig%29%5Cdiagup%20%26%5Cleft%5B%28%5Cfrac%7B1%7D%7B%5Ctau%7D%29%5Csum_%7Bj%3D1%7D%5E%5Ctau%20%20d_%7Bt%7D%20%28j%29%5Cright%5D%5Cend%7Bcases%7D&sig=15a0b31a37bd5db3074f854711119bb3" /></li>
<li><b>Absolute Threshold -</b> Fix for subharmonic error with Autocorrelation, choose (<img src="http://latex.numberempire.com/render?%5Cinline%20%5Chuge%20%5Cmathbf%7B%5Ctau%7D&sig=845639da85c0dd8e2de679817b06639c"/></img>) that gives a minimum <img src="http://latex.numberempire.com/render?%5Chuge%20d%27&sig=ec41ac0239b48a3c53617da8e4ca64c7"/></li>
</ol>
</div>

[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf