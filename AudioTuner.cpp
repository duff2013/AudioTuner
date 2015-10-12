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

#include "AudioTuner.h"
#include "utility/dspinst.h"

#if SAMPLE_RATE == SAMPLE_RATE_44100
    #define SAMPLE_RATE_EXACT AUDIO_SAMPLE_RATE_EXACT / 1
#elif SAMPLE_RATE == SAMPLE_RATE_22050
    #define SAMPLE_RATE_EXACT AUDIO_SAMPLE_RATE_EXACT / 2
#elif SAMPLE_RATE == SAMPLE_RATE_11025
    #define SAMPLE_RATE_EXACT AUDIO_SAMPLE_RATE_EXACT / 4
#endif

#define HALF_BUFFER NUM_SAMPLES / 2

#define LOOP1(a)  a
#define LOOP2(a)  a LOOP1(a)
#define LOOP3(a)  a LOOP2(a)
#define LOOP8(a)  a LOOP3(a) a LOOP3(a)
#define UNROLL(n,a) LOOP##n(a)

/**
 *  Audio update function.
 */
void AudioTuner::update( void ) {
    audio_block_t *block;
    const int16_t *p, *end;
    block = receiveReadOnly( );
    
    if ( !block ) return;
    
    if ( !enabled ) {
        release( block );
        return;
    }
    
    p = block->data;
    end = p + AUDIO_BLOCK_SAMPLES;
    
    /*
     * Set the number of cycles to processed per receiving block.
     *
     */
    uint16_t cycles;
    const uint16_t usage_max = cpu_usage_max;
    if ( AudioProcessorUsage( ) > usage_max ) {
#if NUM_SAMPLES >= 8192
        cycles = tau_global + 2;
#elif NUM_SAMPLES == 4096
        cycles = tau_global + 4;
#elif NUM_SAMPLES == 2048
        cycles = tau_global + 8;
#elif NUM_SAMPLES <= 1024
        cycles = tau_global + 16;
#endif
    }
    else {
#if NUM_SAMPLES >= 8192
        cycles = tau_global + 8;
#elif NUM_SAMPLES == 4096
        cycles = tau_global + 16;
#elif NUM_SAMPLES == 2048
        cycles = tau_global + 32;
#elif NUM_SAMPLES <= 1024
        cycles = tau_global + 64;
#endif
    }
    
    uint16_t count = count_global;
    
    /*
     * Double buffering, one fill while the other is processed
     * 2x the throughput.
    */
    uint16_t *dst;
    if ( next_buffer ) dst = ( uint16_t * )buffer;
    else dst = ( uint16_t * )buffer + NUM_SAMPLES;
    
    // gather data
    do {
        *( dst+count++ ) = *( uint16_t * )p;
        p += SAMPLE_RATE;
    } while ( p < end );
    release( block );
    
    /* 
     * If buffer full switch to start filling next
     * buffer and process the just filled buffer.
     */
    if ( count >= NUM_SAMPLES ) {
        //digitalWriteFast(2, !digitalReadFast(2));
        next_buffer = !next_buffer;
        process_buffer  = true;
        count_global    = 0;
        tau_global      = 1;
        yin_idx         = 1;
        running_sum     = 0;
        count           = 0;
    }
    count_global = count;// update global count
    
    if ( process_buffer ) {
        //digitalWriteFast(0, HIGH);
        uint16_t tau;
        uint16_t  next;
        next = next_buffer;
        tau = tau_global;
        do {
            int64_t sum  = 0;
            const int16_t *end, *buf;
            if ( next ) buf = buffer + NUM_SAMPLES;
            else buf = buffer;
            end = buf + HALF_BUFFER;
            do {
                int16_t current, lag, delta;
                UNROLL( 8,
                       lag = *( buf + tau );
                       current = *buf++;
                       delta = current - lag;
                       //sum = multiply_accumulate_32x32_rshift32_rounded(sum, delta, delta);
                       sum += delta*delta;
                       );
            } while ( buf < end );
            
            running_sum += sum;
            yin_buffer[yin_idx] = sum*tau;
            rs_buffer[yin_idx] = running_sum;
            yin_idx = ( ++yin_idx >= 5 ) ? 0 : yin_idx;
            
            tau = estimate( yin_buffer, rs_buffer, yin_idx, tau );
            
            if ( tau == 0 ) {
                process_buffer  = false;
                new_output      = true;
                //digitalWriteFast(0, LOW);
                return;
            }
            else if ( tau >= HALF_BUFFER ) {
                process_buffer  = false;
                new_output      = false;
                //digitalWriteFast(0, LOW);
                return;
            }
            
        } while ( tau <= cycles );
        tau_global = tau;
        //digitalWriteFast(0, LOW);
    }
}

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
uint16_t AudioTuner::estimate( int64_t *yin, int64_t *rs, uint16_t head, uint16_t tau ) {
    const int64_t *p = ( int64_t * )yin;
    const int64_t *r = ( int64_t * )rs;
    uint16_t _tau, _head;
    _tau = tau;
    _head = head;
    
    if ( _tau > 4 ) {
        
        uint16_t idx0, idx1, idx2;
        idx0 = _head;
        idx1 = _head + 1;
        idx1 = ( idx1 >= 5 ) ? 0 : idx1;
        idx2 = head + 2;
        idx2 = ( idx2 >= 5 ) ? 0 : idx2;
        
        float s0, s1, s2;
        s0 = ( ( float )*( p+idx0 ) / r[idx0] );
        s1 = ( ( float )*( p+idx1 ) / r[idx1] );
        s2 = ( ( float )*( p+idx2 ) / r[idx2] );
        
        if ( s1 < yin_threshold && s1 < s2 ) {
            uint16_t period = _tau - 3;
            periodicity = 1 - s1;
            data = period + 0.5f * ( s0 - s2 ) / ( s0 - 2.0f * s1 + s2 );
            return 0;
        }
        
        if ( s1 > 2.4 ) return _tau + 2;
        else return _tau + 1;
    }
    return _tau + 1;
}

/**
 *  Initialise
 *
 *  @param threshold Allowed uncertainty
 *  @param cpu_max   How much cpu usage before throttling
 */
void AudioTuner::initialize( float threshold, uint8_t cpu_max ) {
    __disable_irq( );
    cpu_usage_max = cpu_max;
    yin_threshold = threshold;
    process_buffer = false;
    periodicity    = 0.0f;
    next_buffer    = 1;
    running_sum    = 0;
    count_global   = 0;
    yin_idx        = 1;
    data           = 0;
    enabled        = true;
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
    d = SAMPLE_RATE_EXACT / d;
    return d;
}

/**
 *  Periodicity of the sampled signal from Yin algorithm from read function.
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
 *  Initialise parameters.
 *
 *  @param thresh    Allowed uncertainty
 */
void AudioTuner::threshold( float p ) {
    __disable_irq( );
    yin_threshold = p;
    __enable_irq( );
}