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

#define HALF_BUFFER         NUM_SAMPLES / 2
#define QUARTER_BUFFER      NUM_SAMPLES / 4
#define EIGTH_BUFFER        NUM_SAMPLES / 8
#define SIXTEENTH_BUFFER    NUM_SAMPLES / 16
#define SAMPLE_RATE AUDIO_SAMPLE_RATE_EXACT / SAMPLE_SKIP

#define LOOP1(a)  a
#define LOOP2(a)  a LOOP1(a)
#define LOOP3(a)  a LOOP2(a)
#define LOOP8(a)  a LOOP3(a) a LOOP3(a)
#define UNROLL(n,a) LOOP##n(a)

#define WINDOW SAMPLE_SKIP - 1

/**
 *  audio update function.
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
    
    uint16_t *dst;
    if ( next_buffer ) dst = ( uint16_t * )buffer;
    else dst = ( uint16_t * )buffer + NUM_SAMPLES;
    
    uint8_t get_sample = 0;
    uint16_t count = block_count;
    do {
        if ( get_sample++ >= WINDOW ) {
            *( dst+count++ ) = *( uint16_t * )p;
            get_sample = 0;
        }
    } while ( p++ < end );
    release( block );

    if ( count >= NUM_SAMPLES ) {
        digitalWriteFast(2, !digitalReadFast(2));
        next_buffer = !next_buffer;
        process_buffer = true;
        tau_global = 1;
        yin_idx = 1;
        running_sum = 0;
        count = 0;
    }
    block_count = count;
    
    if ( process_buffer ) {
        digitalWriteFast(0, HIGH);
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
                       sum += delta*delta;
                       );
            } while ( buf < end );
            
            running_sum += sum;
            yin_buffer[yin_idx] = sum*tau;
            rs_buffer[yin_idx] = running_sum;
            yin_idx = ( ++yin_idx >= 5 ) ? 0 : yin_idx;
            tau = estimate( yin_buffer, rs_buffer, yin_idx, tau );
            if ( tau == 0 ) {
                process_buffer = false;
                new_output = true;
                digitalWriteFast(0, LOW);
                return;
            }
            else if ( tau >= HALF_BUFFER ) {
                process_buffer = false;
                new_output = false;
                digitalWriteFast(0, LOW);
                return;
            }
        } while ( tau <= ( tau_global + 31 ) );
        tau_global = tau;
        digitalWriteFast(0, LOW);
    }
}

/**
 *  process data in from Audio Library interrupt
 */
uint16_t AudioTuner::estimate( int64_t *yin, int64_t *rs, uint16_t head, uint16_t tau ) {
    const int64_t *p = ( int64_t * )yin;
    const int64_t *r = ( int64_t * )rs;
    uint16_t period = 0, _tau, _head;
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
        
        if ( s1 < threshold && s1 < s2 ) {
            period = _tau - 3;
            periodicity = 1 - s1;
            data = period + 0.5f * ( s0 - s2 ) / ( s0 - 2.0f * s1 + s2 );
            return 0;
        }
        
        if ( s1 > 2.2 ) return _tau + 2;
        else return _tau + 1;
    }
    return _tau + 1;
}

/**
 *  available
 *
 *  @return true if data is ready else false
 */
bool AudioTuner::available( void ) {
    __disable_irq();
    bool flag = new_output;
    if (flag) new_output = false;
    __enable_irq();
    return flag;
}

/**
 *  read processes the data samples for the Yin algorithm.
 *
 *  @return frequency in hertz
 */
float AudioTuner::read( void ) {
    return SAMPLE_RATE / data;
}
/**
 *  Periodicity of the sampled signal from Yin algorithm from read function.
 *
 *  @return periodicity
 */
float AudioTuner::probability( void ) {
    return periodicity;
}
/**
 *  Initialise parameters.
 *
 *  @param thresh    Allowed uncertainty
 */
void AudioTuner::set_params( float thresh ) {
    __disable_irq( );
    //arm_cfft_radix4_init_q15(&fft_inst, 1024, 0, 1);
    //arm_cfft_radix4_init_q15(&ifft_inst, 1024, 1, 1);
    threshold = thresh;
    periodicity = 0.0f;
    block_count = 0;
    enabled = true;
    __enable_irq( );
}