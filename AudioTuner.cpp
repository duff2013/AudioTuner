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

#include "utility/dspinst.h"
#include "arm_math.h"
#include "Arduino.h"
#include "AudioTuner.h"

#define NUM_SAMPLES ( AUDIO_GUITARTUNER_BLOCKS << 7 )

void AudioTuner::update( void ) {
    
    audio_block_t *block;
    
    block = receiveReadOnly();
    if (!block) return;
    
    if ( !enabled ) {
        release( block );
        return;
    }
    
    /**
     *  "factor" is the new block size calculated by
     *  the decimated shift to incremnt the buffer
     *  address.
     */
    const uint8_t factor = AUDIO_BLOCK_SAMPLES >> decimation_shift;
    
    // filter and decimate block by block the incoming signal and store in a buffer.
    arm_fir_decimate_fast_q15( &firDecimateInst, block->data, AudioBuffer + ( state * factor ), AUDIO_BLOCK_SAMPLES );
    
    /**
     *  when half the blocks + 1 of the total
     *  blocks have been stored in the buffer
     *  start processing the data.
     */
    if ( state++ >= AUDIO_GUITARTUNER_BLOCKS >> 1 ) {
        
        if ( process_buffer ) process( AudioBuffer );
        
        if ( state == 0 ) process_buffer = true;
    }
    
    release( block );
}

/**
 *  Start the Yin algorithm
 *
 *  TODO: Significant speed up would be to use spectral domain to find fundamental frequency.
 *  This paper explains: https://aubio.org/phd/thesis/brossier06thesis.pdf -> Section 3.2.4
 *  page 79. Might have to downsample for low fundmental frequencies because of fft buffer
 *  size limit.
 */
void AudioTuner::process( int16_t *p ) {
    
    const uint16_t inner_cycles = ( NUM_SAMPLES >> decimation_shift ) >> 1;
    uint16_t outer_cycles = inner_cycles / AUDIO_GUITARTUNER_BLOCKS;
    uint16_t tau = tau_global;
    do {
        uint64_t sum = 0;
        int32_t  a1, a2, b1, b2, c1, c2, d1, d2;
        int32_t  out1, out2, out3, out4;
        uint16_t blkCnt;
        int16_t * cur = p;
        int16_t * lag = p + tau;
        // unrolling the inner loop by 8
        blkCnt = inner_cycles >> 3;
        do {
            // a(n), b(n), c(n), d(n) each hold two samples
            a1 = *__SIMD32( cur )++;
            a2 = *__SIMD32( cur )++;
            b1 = *__SIMD32( lag )++;
            b2 = *__SIMD32( lag )++;
            c1 = *__SIMD32( cur )++;
            c2 = *__SIMD32( cur )++;
            d1 = *__SIMD32( lag )++;
            d2 = *__SIMD32( lag )++;
            // subract two samples at a time
            out1 = __QSUB16( a1, b1 );
            out2 = __QSUB16( a2, b2 );
            out3 = __QSUB16( c1, d1 );
            out4 = __QSUB16( c2, d2 );
            // square the difference
            sum = multiply_accumulate_16tx16t_add_16bx16b( sum, out1, out1 );
            sum = multiply_accumulate_16tx16t_add_16bx16b( sum, out2, out2 );
            sum = multiply_accumulate_16tx16t_add_16bx16b( sum, out3, out3 );
            sum = multiply_accumulate_16tx16t_add_16bx16b( sum, out4, out4 );
            
        } while( --blkCnt );
        
        uint64_t rs = running_sum;
        rs += sum;
        yin_buffer[yin_idx] = sum*tau;
        rs_buffer[yin_idx] = rs;
        running_sum = rs;
        yin_idx = ( ++yin_idx >= 5 ) ? 0 : yin_idx;
        tau = estimate( yin_buffer, rs_buffer, yin_idx, tau );
        
        if ( tau == 0 ) {
            process_buffer  = false;
            new_output      = true;
            yin_idx         = 1;
            running_sum     = 0;
            tau_global      = 1;
            state           = 0;
            return;
        }
        
    } while ( --outer_cycles );
    
    if ( tau >= inner_cycles ) {
        process_buffer  = true;
        new_output      = false;
        yin_idx         = 1;
        running_sum     = 0;
        tau_global      = 1;
        state           = 0;
        return;
    }
    tau_global = tau;
}

/**
 *  check the sampled data for fundamental frequency
 *
 *  @param yin  buffer to hold sum*tau value
 *  @param rs   buffer to hold running sum for sampled window
 *  @param head buffer index
 *  @param tau  lag we are curly working on gets incremented
 *
 *  @return tau
 */
uint16_t AudioTuner::estimate( uint64_t *yin, uint64_t *rs, uint16_t head, uint16_t tau ) {
    const uint64_t *y = ( uint64_t * )yin;
    const uint64_t *r = ( uint64_t * )rs;
    uint16_t _tau, _head;
    const float thresh = yin_threshold;
    _tau = tau;
    _head = head;
    
    if ( _tau > 4 ) {
        
        uint16_t idx0, idx1, idx2;
        idx0 = _head;
        idx1 = _head + 1;
        idx1 = ( idx1 >= 5 ) ? 0 : idx1;
        idx2 = _head + 2;
        idx2 = ( idx2 >= 5 ) ? idx2 - 5 : idx2;
        
        // maybe fixed point would be better here? But how?
        float s0, s1, s2;
        s0 = ( ( float )*( y+idx0 ) / ( float )*( r+idx0 ) );
        s1 = ( ( float )*( y+idx1 ) / ( float )*( r+idx1 ) );
        s2 = ( ( float )*( y+idx2 ) / ( float )*( r+idx2 ) );
        
        if ( s1 < thresh && s1 < s2 ) {
            uint16_t period = _tau - 3;
            periodicity = 1 - s1;
            data = period + 0.5f * ( s0 - s2 ) / ( s0 - 2.0f * s1 + s2 );
            return 0;
        }
    }
    return _tau + 1;
}

/**
 *  Initialise
 *
 *  @param threshold Allowed uncertainty
 */
void AudioTuner::begin( float threshold, int16_t *coeff, uint8_t taps, uint8_t factor ) {
    __disable_irq( );
    process_buffer      = true;
    yin_threshold       = threshold;
    periodicity         = 0.0f;
    running_sum         = 0;
    tau_global          = 1;
    yin_idx             = 1;
    enabled             = true;
    state               = 0;
    data                = 0.0f;
    decimation_factor   = factor;
    decimation_shift    = log( factor ) / log( 2 );
    coeff_size          = taps;
    coeff_p             = coeff;
    arm_fir_decimate_init_q15( &firDecimateInst, coeff_size, decimation_factor, coeff_p, &coeff_state[0], AUDIO_BLOCK_SAMPLES );
    __enable_irq( );
}

/**
 *  available
 *
 *  @return true if data is ready else false
 */
bool AudioTuner::available( void ) {
    __disable_irq( );
    bool flag = new_output;
    if ( flag ) new_output = false;
    __enable_irq( );
    return flag;
}

/**
 *  read processes the data samples for the Yin algorithm.
 *
 *  @return frequency in hertz
 */
float AudioTuner::read( void ) {
    __disable_irq( );
    float d = data;
    __enable_irq( );
    return ( AUDIO_SAMPLE_RATE_EXACT / decimation_factor ) / d;
}

/**
 *  Periodicity of the sampled signal.
 *
 *  @return periodicity
 */
float AudioTuner::probability( void ) {
    __disable_irq( );
    float p = periodicity;
    __enable_irq( );
    return p;
}

/**
 *  New LP coeffients for decimation.
 *
 *  @param p    array pointer of coeffients.
 *  @param n    array size.
 */
void AudioTuner::coeff( int16_t *p, int n ) {
    //coeff_size = n;
    //coeff_p = p;
    //arm_fir_decimate_init_q15(&firDecimateInst, coeff_size, 4, coeff_p, coeff_state, 128);
}

/**
 *  Initialise parameters.
 *
 *  @param thresh    Allowed uncertainty
 */
void AudioTuner::threshold( float p ) {
    __disable_irq( );
    yin_threshold = p;
    __enable_irq( );
}

/**
 *  disable yin from processing data, use begin to start back up
 *
 *  @return none
 */
void AudioTuner::disable( void ) {
    __disable_irq( );
    enabled = false;
    __enable_irq( );
}
