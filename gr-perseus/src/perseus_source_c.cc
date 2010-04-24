/* -*- c++ -*- */
/*
 * Copyright 2010 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in he hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <perseus-sdr.h>
#include <perseus_source_c.h>
#include <gr_io_signature.h>
#include <gr_prefs.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
//#include <gnuradio/omnithread.h>
#include <string.h>

//#define	LOGGING 0		// define to 0 or 1

//#define SAMPLE_FORMAT 		paFloat32
typedef float sample_t;

// Number of portaudio buffers in the ringbuffer
static const unsigned int N_BUFFERS = 4;

// 
static int m_singleton = 0;

static std::string 
default_device_name ()
{
	return gr_prefs::singleton()->get_string("perseus", "default_input_device", "");
}

void
perseus_source_c::create_ringbuffer(void)
{
	int n_samples = d_perseus_buffer_size_frames; // 
  
	if (d_verbose) {
		fprintf(stderr, "************ring buffer size  = %d samples\n", n_samples );
		fprintf(stderr, "************item size         = %d bytes\n",   (sizeof(sample_t)*2) );
		fprintf(stderr, "************buffer size       = %d bytes\n",   N_BUFFERS * n_samples * (sizeof(sample_t)*2)) ;
		fflush (stderr);
	}

	// FYI, the buffer indicies are in units of samples.
	d_writer = gr_make_buffer(N_BUFFERS * n_samples, sizeof(sample_t)*2);
	d_reader = gr_buffer_add_reader(d_writer, 0);
}

//const float NORM_FACTOR =  static_cast<float>(1.0 / static_cast<double>(1L << 23));
//const float NORM_FACTOR = 8388607.0; // (float)((unsigned long int)2e23 - 1);

//const float NORM_FACTOR = (8388607.0 * 256); // (float)((unsigned long int)2e23 - 1);


const float NORM_FACTOR = static_cast<float>(static_cast<double>(1L << 31));

typedef union {
        struct __attribute__((__packed__)) {
                int32_t i;
                int32_t q;
                } iq;
        struct __attribute__((__packed__)) {
                uint8_t         i1;
                uint8_t         i2;
                uint8_t         i3;
                uint8_t         i4;
                uint8_t         q1;
                uint8_t         q2;
                uint8_t         q3;
                uint8_t         q4;
                };
} iq_sample;


int perseus_source_c::callbackPerseus (void *buf, int buf_size, void *param)
{
    // The buffer received contains 24-bit IQ samples (6 bytes per sample)
    // At the maximum sample rate (2 MS/s) the system should be capable
    // of processing data at a rate of at least 16 MB/s (almost 1 GB/min!)

    uint8_t *samplebuf      = (uint8_t*)buf;
    int nIQValues           = buf_size / 6;    // data from Perseus are in 24 bit signed couples of samples
    int k;
    iq_sample s;
    perseus_source_c *self = (perseus_source_c *)param;

    static float *ov    = 0;
    static int    ovLen = 0;

    // setup a intermediate buffer for converted IQ samples
    // these are static buffers for efficiency
    if ( ov == 0 ) {
        ovLen = nIQValues * 2;
        ov = new float [ovLen] ;
    }
    assert (ov);

    if (self == 0) {
        fprintf (stderr, "%s: no receiver data.\n", __FUNCTION__);
        return 0;
    }

    int nframes_to_copy = nIQValues/2; // the frames are couples of sample
                                       // items in d_writer jargon
                                       // each item is a couple of float

    int nframes_room = self->d_writer->space_available() ;
    if (self->d_verbose)fprintf (stderr, "%s: space available in items: %d\n", __FUNCTION__, self->d_writer->space_available());

    if (nframes_to_copy <= nframes_room) {  // We've got room for the data ..

    //if (LOGGING)
    //  self->d_log->printf("PAsrc  cb: f/b = %4ld\n", framesPerBuffer);

       if (self->d_verbose)
          fprintf (stderr, "%s: norm: %f input: %d  nIQ: %d nF2C: %d nFRoom in items: %d\n", 
                   __FUNCTION__, NORM_FACTOR, buf_size, nIQValues, nframes_to_copy, nframes_room),
          fflush(stderr);


       s.i1 = s.q1 = 0;
       int nItemsWritten = 0;
       
       for (k=0; k < nIQValues; k++) {

          s.i2 = *samplebuf++;
          s.i3 = *samplebuf++;
          s.i4 = *samplebuf++;
          s.q2 = *samplebuf++;
          s.q3 = *samplebuf++;
          s.q4 = *samplebuf++;

          if (self->d_verbose)
             if ((k % 1024) == 0) {
                 fprintf (stderr, "%s: i: %02x %02x %02x %02x  q: %02x %02x %02x %02x\n", 
                          __FUNCTION__, s.i1, s.i2, s.i3, s.i4, s.q1, s.q2, s.q3, s.q4 
                         );
                 fprintf (stderr, "%s: k: %d i: %d q: %d\n", __FUNCTION__, k, s.iq.i, s.iq.q);  
                 fflush(stderr);
             }

          ov [k*2+0] =  ((float)s.iq.i) / NORM_FACTOR;
          ov [k*2+1] =  ((float)s.iq.q) / NORM_FACTOR;
          
          if (self->d_verbose)
             if ((k % 1024) == 0) {
                  fprintf (stderr, "%s: ni: %f nq:%f\n", __FUNCTION__, ov [k*2+0], ov [k*2+1] );
                  fflush(stderr);
              }

          nItemsWritten += 1;
       }

       if (self->d_verbose)
          fprintf (stderr, "%s: written items: %d bytes copied: %d\n", __FUNCTION__, nItemsWritten, ovLen*sizeof(float)),
          fflush(stderr);

       memcpy(self->d_writer->write_pointer(), ov, ovLen*sizeof(float));
       self->d_writer->update_write_pointer(nItemsWritten);
       // Tell the source thread there is new data in the ringbuffer.
       self->d_ringbuffer_ready.post();
    }

    else {			// overrun
    //if (LOGGING)
    //  self->d_log->printf("PAsrc  cb: f/b = %4ld OVERRUN\n", framesPerBuffer);

       self->d_noverruns++;
       ::write(1, "pO", 2);	// FIXME change to non-blocking call

       self->d_ringbuffer_ready.post();  // Tell the sink to get going!

    }
    return 0;  
}


// ----------------------------------------------------------------

perseus_source_c_sptr
perseus_make_source_c (int sampling_rate, const std::string dev, bool ok_to_block) throw (std::runtime_error)
{
  if (m_singleton == 0) {
    int num_perseus = perseus_init();
    if (num_perseus > 0) {
       if (gr_prefs::singleton()->get_bool("perseus", "verbose", false))
           perseus_set_debug(3);
       else
           perseus_set_debug(0);
                                                                          
       m_singleton += 1;
    } else {
        printf("Perseus hardware not found: %s\n", perseus_errorstr());
        throw std::runtime_error ("perseus hw not found");
    }
  }
  return perseus_source_c_sptr (new perseus_source_c (sampling_rate, dev, ok_to_block));
}

typedef struct _sr {
    int srate;
    char *file_name;
} sample_rates;

static char *getFpgaFile (int xsr)
{
    static sample_rates sr[] = {
        { 125000,  "perseus125k24v21.rbs"   },
        { 1000000, "perseus1m24v21.rbs"     },
        { 250000,  "perseus250k24v21.rbs"   },
        { 2000000, "perseus2m24v21.rbs"     },
        { 500000,  "perseus500k24v21.rbs"   },
        { 95000,   "perseus95k24v31.rbs"    }
    };

    for (unsigned int  i=0; i< (sizeof(sr)/sizeof(sr[0])); ++i) {
        if (xsr == sr[i].srate) return sr[i].file_name;
    }
    return 0;
}


perseus_source_c::perseus_source_c (int sampling_rate,
			     	     	     	const std::string device_name,
			     			        bool ok_to_block)
  : gr_sync_block ("perseus_source_c",
		           gr_make_io_signature(0, 0, 0),
		           gr_make_io_signature(0, 0, 0)),
    d_sampling_rate(sampling_rate),
    d_device_name(device_name.empty() ? default_device_name() : device_name),
    d_ok_to_block(ok_to_block),
    d_verbose(gr_prefs::singleton()->get_bool("perseus", "verbose", false)),
    d_perseus_buffer_size_frames(2720), // bytes / bytes_per_I_Q_couple_of_sample == # of frames == # of couples of samples
    d_ringbuffer_ready(1, 1),		// binary semaphore
    d_noverruns(0)
{
    d_started = 0;
    d_fef     = 0; 
    d_freq    = 7100000.0;


    d_serial_number = 0;  // product id
    d_signature = 0;      // product signature"0000-0000-0000"; 
    d_hw_release = 0;     // hardware release
    d_hw_version = 0;     // hardware version

    // Open the first one...
    if ((pPd = perseus_open(0))==NULL) {
       printf("error: %s\n", perseus_errorstr());
       bail("perseus_open error !",0);
    }

    // Download the standard firmware to the unit
    printf("Downloading firmware...\n");
    if (perseus_firmware_download (pPd,NULL)<0) {
         printf("firmware download error: %s", perseus_errorstr());
         perseus_close (pPd);
         bail ("perseus firmware download error !", 0);
    }

    // Save some information about the receiver (S/N and HW rev)
    eeprom_prodid prodid; // Perseus library data structure

    if (pPd->is_preserie == TRUE)
       printf("The device is a preserie unit");
    else
       if (perseus_get_product_id(pPd,&prodid) >= 0) {
           d_signature = new char [SIGNATURE_LENGTH+1]; // product signature"0000-0000-0000"; 
           snprintf(d_signature, SIGNATURE_LENGTH, "%02hX%02hX-%02hX%02hX-%02hX%02hX",
                    (uint16_t) prodid.signature[5],
                    (uint16_t) prodid.signature[4],
                    (uint16_t) prodid.signature[3],
                    (uint16_t) prodid.signature[2],
                    (uint16_t) prodid.signature[1],
                    (uint16_t) prodid.signature[0]
                   );
           d_serial_number = (uint16_t) prodid.sn;
           d_hw_release = (uint16_t) prodid.hwrel;
           d_hw_version = (uint16_t) prodid.hwver;

       } else {
           printf("get product id error: %s", perseus_errorstr());
       }

    char *fn = getFpgaFile (sampling_rate);

    if ( fn ) {
        if (perseus_fpga_config (pPd, fn) < 0) {
            printf("fpga configuration error: %s\n", perseus_errorstr());
            perseus_close (pPd);
            bail ("perseus FPGA configuration error !",0);
        }
    } else {
        bail ("perseus doesn't support this sample rate !",0);
    }

    // Disable ADC Dither, Disable ADC Preamp
    perseus_set_adc (pPd, FALSE, FALSE);

    // Do the same cycling test with the WB front panel led.
    // Enable preselection filters (WB_MODE Off)
    //perseus_set_ddc_center_freq (pPd, 7100000.000, 1);

    // disable the attenuator
    perseus_set_attenuator(pPd, PERSEUS_ATT_0DB);

    d_device_name = "PERSEUS-0";
  
    // Perseus receiver outputs a complex stream on a single channel
	set_output_signature(gr_make_io_signature(1, 1, sizeof (gr_complex)));
}


bool
perseus_source_c::check_topology (int ninputs, int noutputs)
{

	fprintf(stderr, "d_perseus_buffer_size_frames = %d\n", d_perseus_buffer_size_frames);
	fprintf(stderr, "#input: %d #output: %d\n", ninputs, noutputs);

	assert(d_perseus_buffer_size_frames != 0);
	assert(ninputs==0);
	assert(noutputs==1);

	create_ringbuffer();

	if (perseus_start_async_input(pPd, d_perseus_buffer_size_frames*3*2, callbackPerseus, (void *)this) < 0) {
		return false;
	} else {
		d_started = 1;
		return true;
	}
}

perseus_source_c::~perseus_source_c ()
{
	if (d_started) {
	perseus_stop_async_input(pPd);
	d_started = 0;
	}
	perseus_close (pPd);
	m_singleton -= 1;
	if (m_singleton == 0) perseus_exit();
    delete [] d_signature;
}


int
perseus_source_c::work (int noutput_items,
			            gr_vector_const_void_star &input_items,
			            gr_vector_void_star &output_items)
{
	//float **out = (float **) &output_items[0];
	gr_complex *out = ((gr_complex *) output_items[0]);
	//gr_complex *out = &((gr_complex *) output_items[0])[0];

	const unsigned nchan = 2 ; //d_input_parameters.channelCount; // # of channels == samples/frame


	if (d_verbose) 
		fprintf (stderr, ">>> %s: nout: %d\n", __FUNCTION__, noutput_items); 
       
	int k;
	for (k = 0; k < noutput_items; ){

		int nframes = d_reader->items_available();	// # of frames in ringbuffer

		if (d_verbose) fprintf (stderr, "%s: k: %d nframes: %d\n", __FUNCTION__, k, nframes); 


		if (nframes == 0) {		// no data right now...
			if (k > 0)      		// If we've produced anything so far, return that
				return k;

			if (d_ok_to_block) {
				if (d_verbose) fprintf (stderr, "%s: blocking.....\n", __FUNCTION__); 
					d_ringbuffer_ready.wait();	// block here, then try again
					continue;
			}

			assert(k == 0);

      		// There's no data and we're not allowed to block.
      		// (A USRP is most likely controlling the pacing through the pipeline.)
      		// This is an underun.  The scheduler wouldn't have called us if it
      		// had anything better to do.  Thus we really need to produce some amount
      		// of "fill".
      		//
      		// There are lots of options for comfort noise, etc.
      		// FIXME We'll fill with zeros for now.  Yes, it will "click"...

      		// Fill with some frames of zeros
			int nf = std::min(noutput_items - k, (int) d_perseus_buffer_size_frames);
			for (int i = 0; i < nf; i++) {
				out[i] = gr_complex ((float) 0, (float) 0);
			}
			k += nf;
			return k;
		}

		// We can read the smaller of the request and what's in the buffer.
		int nf = std::min(noutput_items - k, nframes);
		if (d_verbose) fprintf (stderr, "%s: nf: %d nch: %d --- before copy loop...........\n", __FUNCTION__, nf, nchan); 
	
		const float *p = (const float *) d_reader->read_pointer();
		if (d_verbose) fprintf (stderr, "%s: i: %f q: %f\n", __FUNCTION__, *p, *(p+1)); 
		for (int i = 0; i < nf; i++) {
            register float ti, tq;

			ti = *p++, tq = *p++;
			out[i] = gr_complex (ti, tq);
		}

		d_reader->update_read_pointer(nf);
		k += nf;
	}

	if (d_verbose) fprintf (stderr, "%s: <<<< before exiting: returning k: %d\n", __FUNCTION__, k); 

	return k;  // tell how many we actually did
}


void
perseus_source_c::output_error_msg (const char *msg, int err)
{
  fprintf (stderr, "perseus_source[%s]: %s: %s\n", d_device_name.c_str (), msg, perseus_errorstr());
}

void
perseus_source_c::bail (const char *msg, int err) throw (std::runtime_error)
{
  output_error_msg (msg, err);
  throw std::runtime_error ("perseus");
}


void perseus_source_c::set_frontend_filters (bool flag_activate)
{
    (flag_activate == true) ? d_fef = 1 :  d_fef = 0 ;
}


void perseus_source_c::tune (float frequency)
{
    d_freq = frequency;
    if (pPd) {
        if (d_verbose) fprintf (stderr, "%s: freq: %f front end filter: %d\n", __FUNCTION__, d_freq, d_fef); 
        if (perseus_set_ddc_center_freq (pPd, (double)d_freq, d_fef) < 0)
            printf("perseus_set_ddc_center_freq error: %s\n", perseus_errorstr());
    }
}


typedef struct _atten {
    int id;
    int valInDb;
} AttenuatorValues ;

int perseus_source_c::set_attenuator (int levelInDb)
{
    static AttenuatorValues av [] = {
       { PERSEUS_ATT_0DB , 0  } , 
       { PERSEUS_ATT_10DB, 10 } , 
       { PERSEUS_ATT_20DB, 20 } , 
       { PERSEUS_ATT_30DB, 30 } , 
    } ;
    if (d_verbose) fprintf (stderr, "%s: attenuator: %d\n", __FUNCTION__, levelInDb); 
    if (pPd) {
        for (unsigned int i=0; i< (sizeof(av)/sizeof(av[0])); ++i) 
            if (levelInDb == av[i].valInDb) {
                if (d_verbose) fprintf (stderr, "%s: attenuator id: %d\n", __FUNCTION__, av[i].id); 
                if (perseus_set_attenuator(pPd, av[i].id) < 0) {
                    printf("%s: %s\n", __FUNCTION__, perseus_errorstr());
                }
            }
    }
    return 0;
}

void perseus_source_c::set_adc_dither (bool f_dither)
{
    d_adc_dither = f_dither;
    if (pPd) {
        if (perseus_set_adc(pPd, d_adc_dither, d_adc_preamp) < 0)
            printf("%s: %s\n", __FUNCTION__, perseus_errorstr());
    }
}

void perseus_source_c::set_adc_preamp (bool f_preamp)
{
    d_adc_preamp = f_preamp;
    if (pPd) {
        if (perseus_set_adc(pPd, d_adc_dither, d_adc_preamp) < 0)
            printf("%s: %s\n", __FUNCTION__, perseus_errorstr());
    }
}

