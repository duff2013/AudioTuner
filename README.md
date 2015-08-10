#Guitar and Bass Tuner for Teensy 3.1 v2.0

Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm is somewhat memory and processor hungry but will allow you to detect lower frequencies than the Audio library's Goertzel algorithm will.

[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf
<!-- language: lang-none -->
      Hookup Guide
     _______________                    _________
    |GND  |___|  Vin|                   |       |
    |0      T   AGND|<----------|       |       |
    |1      E   3.3V|           |       |       |
    |2      E     23|           |       |       |
    |3      N     22|           |       |       |
    |4      S     21|           |       |_______|
    |5      Y     20|           |         |':`|
    |6     3.1    19|           |         |'S`|
    |7            18|           |-------->|'H`|
    |8            17|                     |'I`|
    |9         A2/16|<--------------|     |'E`|
    |10           15|               |     |'L`|
    |11           14|               |     |'D`|
    |12           13|               |     |':`|
     ---------------                |     |===|
                                    |      \_/
                                    |      /T\
                                    |     - I -
                                    |     \ P /
                                    |----->\_/
