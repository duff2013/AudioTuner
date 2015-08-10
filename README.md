#Guitar and Bass Tuner for Teensy 3.1 v2.0

Software algorithm ([YIN]) for guitar and bass tuning using a Teensy Audio Library. This audio object's algorithm is somewhat memory and processor hungry but will allow you to detect lower frequencies than the Audio library's Goertzel algorithm will.

[YIN]:http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf
<!-- language: lang-none -->

_______________                    _________
|GND  |   |  Vin|                   |       |
|0     ---  AGND|                   |       |
|1          3.3V|                   |       |
|2            23|                   |       |
|3            22|                   |       |
|4            21|                   |_______|
|5            20|                     |':`|
|6            19|                     |':`|
|7            18|                     |':`|
|8            17|                     |':`|
|9         A2/16|                     |':`|
|10           15|                     |':`|
|11           14|                     |':`|
|12           13|                     |===|
---------------                      |___|
\ /
/ \
-----
\   /
\_/
