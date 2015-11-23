<p align="center">
    <b>Guitar and Bass Tuner Library v2.3</b><br>
    <b>Teensy 3.1/2</b><br>
</p>

>Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm can be some what memory and processor hungry but will allow you to detect with fairly good accuracy the fundamental frequencies f<sub>o</sub> from electric guitars and basses. 
>>You can install this as a normal Arduino Library and will work with the Audio Library, no need to edit the Audio libraries source now.

<!-- language: lang-none -->
      Hookup Guide - ~.6v DC Bias and High Pass Filter - No Amplification

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
    |9   ------A2/16|<---SIGNAL-.6v-BIAS----*---+|(----*    | E`|>--ANGD--*
    |10    ---    15|                    .6VDC 10uF    |    | L`|
    |11   |(`)|   14|                                  |    | D`|
    |12    ---    13|                                  |    |  `|
     ---------------                                   |    |===|
                                                       |     \_/
                                                       |     /T\
                                                       |    .-I-.
                                                       *---<\ P /
                                                             \_/

>Many optimizations have been done to the [YIN] algorithm for frequencies between 29-400Hz. 
>>While its still using a brute force method ( n<sup>2</sup> ) for finding the fundamental frequency f<sub>o</sub>, it is tuned to skip certain <b>tau</b> (<img src="http://latex.numberempire.com/render?%5Cinline%20%5Chuge%20%5Cmathbf%7B%5Ctau%7D&sig=845639da85c0dd8e2de679817b06639c"/></img>) values and focus mostly on frequencies found in the bass and guitar. 
>>>The input is double buffered so while you are processing one buffer it is filling the other to double throughput. 
>>>>The parameter AUDIO_BLOCKS below can be adjusted but its default of 24 I found to be best to work with the guitar and bass frequency range (29- 400)Hz. 
>>>>>Looking into finding the Auto Correlation using FFT and IFFT to speed up processing of data! Not that simple because the YIN algorithm uses a squared difference tweak to the Auto Correlation.

<h4>AudioTuner.h</h4>

```
/****************************************************************
*              Safe to adjust these values below               *
*                                                              *
*  This parameter defines the size of the buffer.              *
*                                                              *
*  1.  AUDIO_BLOCKS -  Buffer size is 128 * AUDIO_BLOCKS.      *
*                      The more AUDIO_BLOCKS the lower the     *
*                      frequency you can detect. The default   *
*                      (24) is set to measure down to 29.14    *
*                      Hz or B(flat)0.                         *
*                                                              *
****************************************************************/
#define AUDIO_BLOCKS  24
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
[Teensy Audio Library]:http://www.pjrc.com/teensy/td_libs_Audio.html