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

class AudioTuner : public AudioStream
{
public:
    /**
     *  constructor to setup Audio Library and initialize
     *
     *  @return none
     */
    AudioTuner( void ) : AudioStream( 1, inputQueueArray ), enabled( false ), new_output(false) {}
    
    /**
     *  initialize variables and start conversion
     *
     *  @param threshold Allowed uncertainty
     *  @param cpu_max   How much cpu usage before throttling
     */
    void initialize( float threshold, float cpu_max);
    
    /**
     *  sets threshold value
     *
     *  @param thresh
     */
    void threshold( float p );
    
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
     *  @return probability of frequency found
     */
    float probability( void );
    
    /**
     *  Audio Library calls this update function ~2.9ms
     */
    virtual void update( void );
    
private:
    /**
     *  check the sampled data for fundamental frequency
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
    float    periodicity, yin_threshold, data, cpu_usage_max;
    int64_t  rs_buffer[5], yin_buffer[5];
    uint64_t running_sum;
    uint16_t tau_global, count_global, tau_cycles;
    uint8_t  yin_idx;
    bool     enabled, process_buffer, next_buffer;
    volatile bool new_output;
    audio_block_t *inputQueueArray[1];
};
#endif