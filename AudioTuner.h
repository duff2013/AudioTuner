/* Audio Library Note Frequency Detection & Guitar/Bass Tuner
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
#include "arm_math.h"
/***********************************************************************
 *              Safe to adjust these values below                      *
 *                                                                     *
 *  This parameter defines the size of the buffer.                     *
 *                                                                     *
 *  1.  AUDIO_GUITARTUNER_BLOCKS -  Buffer size is 128 * AUDIO_BLOCKS. *
 *                      The more AUDIO_GUITARTUNER_BLOCKS the lower    *
 *                      the frequency you can detect. The default      *
 *                      (24) is set to measure down to 29.14 Hz        *
 *                      or B(flat)0.                                   *
 *                                                                     *
 *  2.  MAX_COEFF - Maxium number of coefficeints for the FIR filter.  *
 *                                                                     *
 ***********************************************************************/
#define AUDIO_GUITARTUNER_BLOCKS  24
#define MAX_COEFF                 200
/***********************************************************************/

class AudioTuner : public AudioStream {
public:
    /**
     *  constructor to setup Audio Library and initialize
     *
     *  @return none
     */
    AudioTuner( void ) : AudioStream( 1, inputQueueArray ),
    data( 0.0 ),
    coeff_p( NULL ),
    enabled( false ),
    new_output( false ),
    coeff_size( 0 )
    
    {
        
    }
    
    /**
     *  initialize variables and start
     *
     *  @param threshold Allowed uncertainty
     *  @param coeff     coefficients for fir filter
     *  @param taps      number of coefficients, even
     *  @param factor    must be power of 2
     */
    void begin( float threshold, int16_t *coeff, uint8_t taps, uint8_t factor );
    
    /**
     *  sets threshold value
     *
     *  @param thresh
     *  @return none
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
     *  fir decimation coefficents
     *
     *  @return none
     */
    void coeff( int16_t *p, int n );
    
    /**
     *  Audio Library calls this update function ~2.9ms
     *
     *  @return none
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
    uint16_t estimate( uint64_t *yin, uint64_t *rs, uint16_t head, uint16_t tau );
    
    /**
     *  process audio data
     *
     *  @return none
     */
    void process( int16_t *p );
    
    /**
     *  disable yin
     *
     *  @return none
     */
    void disable( void );
    
    /**
     *  Variables
     */
    float    periodicity, yin_threshold, data;
    uint64_t running_sum, yin_buffer[5], rs_buffer[5];
    uint16_t tau_global;
    int16_t  AudioBuffer[AUDIO_GUITARTUNER_BLOCKS*AUDIO_BLOCK_SAMPLES] __attribute__ ( ( aligned ( 4 ) ) );
    int16_t  coeff_state[AUDIO_BLOCK_SAMPLES + MAX_COEFF];
    int16_t  *coeff_p;
    uint8_t  yin_idx, state, coeff_size, decimation_factor, decimation_shift;
    volatile bool new_output, process_buffer, enabled;
    audio_block_t *inputQueueArray[1];
    arm_fir_decimate_instance_q15 firDecimateInst;
};
#endif
