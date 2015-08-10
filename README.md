#Guitar and Bass Tuner Library for Teensy 3.1 v2.0

>Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm can be some what memory and processor hungry but will allow you to detect with fairly good accurcey the lower fundumential frequencies from electric guitars and basses. You can install this as a normal Arduino Library and will work with the Audio Library, no need to edit the Audio libraries source now.

<!-- language: lang-none -->
      Hookup Guide - 1.2v DC Bias and High Pass Filter - No Amplification


                       *-------------------------------------------------|
                       |                                   |   |         |
                       *------------/\/\/\------------|   _|___|_        |
                       |             47K              |  |` ` ` `|       |
                       *---)|+--*                     |  | ` ` ` |       |
                       |  10uF  |                     |  |` ` ` `|       |
                       |        |                     |  | ` ` ` |       |
     _______________   *-/\/\/\-*                     |  |` ` ` `|       |
    |GND  |___|  Vin|  |  2.2K  |                     |  | ` ` ` |       |
    |0      T   AGND|<-|        |                     |  |` ` ` `|       |
    |1      E   3.3V|<--/\/\/\--*--/\/\/\---|         |  | ` ` ` |       |
    |2      E     23|    10K        47K     |         |  |` ` ` `|       |
    |3      N     22|                       |         |  | ` ` ` |       |
    |4      S     21|                       |         |   \_`_`_/        |
    |5      Y     20|                       |         |    | :`|         |
    |6     3.1    19|                       |         |    | S`|         |
    |7            18|                       |   REMOVE|    | H`|         |
    |8            17|                       |     DC  |    | I`|         |
    |9         A2/16|<---SIGNAL-1.2v-BIAS---*----+|(--*    | E`|<--ANGD--|
    |10    ---    15|                     1.2V   10uF |    | L`|
    |11   |(`)|   14|                      DC         |    | D`|
    |12    ---    13|                                 |    | :`|
     ---------------                                  |    |===|
                                                      |     \_/
                                                      |     /T\
                                                      |    - I -
                                                      |    \ P /
                                                      |---->\_/



[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf