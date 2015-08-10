/* Audio Library Guitar and Bass Tuner
 * Copyright (c) 2015, Colin Duffy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef AudioTuner_h_
#define AudioTuner_h_

#include "AudioStream.h"
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

class AudioTuner : public AudioStream
{
public:
    AudioTuner( void ) : AudioStream( 1, inputQueueArray ),
                                     enabled( false ), new_output(false),
                                     next_buffer(1), process_buffer(false),
                                     running_sum(0), block_count(0),
                                     yin_idx(1)
    {
        
        set_params( 0.05f );// threshold set 15ms
    }
    
    /**
     *  sets threshold value and window length
     *
     *  @param thresh
     */
    void set_params( float thresh );
    
    /**
     *  triggers true when valid frequency is found
     *
     *  @return flag to indicate valid frequency is found
     */
    bool available( void );
    /**
     *  get frequency
     *
     *  @return frequency in hertz
     */
    float read( void );
    
    /**
     *  get predicitity
     *
     *  @return probability of correct freq found
     */
    float probability( void );
    /**
     *  Audio Library calls this update function ~2.9ms
     */
    virtual void update( void );
private:
    /**
     *  check the sampled data for fundmental frequency
     *
     *  @param yin  buffer to hold sum*tau value
     *  @param rs   buffer to hold running sum for sampled window
     *  @param head buffer index
     *  @param tau  lag we are currently working on this gets incremented
     *
     *  @return tau
     */
    uint16_t estimate( int64_t *yin, int64_t *rs, uint16_t head, uint16_t tau );
    
    int16_t  buffer[NUM_SAMPLES*2] __attribute__ ( ( aligned ( 4 ) ) );
    float    periodicity, threshold, data;
    int64_t  rs_buffer[5], yin_buffer[5];
    uint64_t running_sum;
    uint16_t block_count, tau_global;
    uint8_t  next_buffer, yin_idx;
    bool     enabled, process_buffer;
    volatile bool new_output;
    audio_block_t *inputQueueArray[1];
};
#endif