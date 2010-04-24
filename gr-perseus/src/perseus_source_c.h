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
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INCLUDED_PERSEUS_SOURCE_C_H
#define INCLUDED_PERSEUS_SOURCE_C_H

#include <gr_sync_block.h>
#include <gr_buffer.h>
#include <gnuradio/omnithread.h>
#include <string>
#include <stdexcept>
//#include <gri_logger.h>
#include <perseus-in.h>
#include <perseus-sdr.h>

class perseus_source_c;
typedef boost::shared_ptr<perseus_source_c> perseus_source_c_sptr;

/*!
 * \brief PERSUES I/Q source.
 *
 * \param sampling_rate	sampling rate in Hz
 * \param device_name PERSEUS device name, e.g., "pa:"
 * \param ok_to_block	true if it's ok for us to block
 */
perseus_source_c_sptr
perseus_make_source_c (int sampling_rate,
					   const std::string device_name = "",
					   bool ok_to_block = true)  throw (std::runtime_error);

#define SIGNATURE_LENGTH 32

/*!
 * \ I/Q source using PERSEUS
 *
 * Input samples must be in the range [-1,1].
 */
class perseus_source_c : public gr_sync_block {
  friend perseus_source_c_sptr
  perseus_make_source_c (int sampling_rate,
			     const std::string device_name,
			     bool ok_to_block) throw (std::runtime_error);

  //friend PaStreamCallback portaudio_source_callback;
  static int callbackPerseus (void *buf, int buf_size, void *extra);


  unsigned int		d_sampling_rate;// samples per seconds
  std::string		d_device_name;	//
  bool				d_ok_to_block;	//
  bool				d_verbose;		// activates more debug messages

  // status variables
  int				d_started;		// flags to indicate asynchrounous I/O is running
  int				d_fef;			// front end filter
  float				d_freq;			// frequency
  bool              d_adc_dither;	// 
  bool              d_adc_preamp;	//

  unsigned int		d_perseus_buffer_size_frames;	// number of frames in a perseus buffer

  perseus_descr *pPd; 

  gr_buffer_sptr	    d_writer;			// buffer used between work and callback
  gr_buffer_reader_sptr	d_reader;
  omni_semaphore	    d_ringbuffer_ready;	// binary semaphore

  // random stats
  int	d_noverruns;		// count of overruns
  //gri_logger_sptr	d_log;			// handle to non-blocking logging instance

  unsigned short d_serial_number;        // product id
  char *d_signature;                     // product signature "0000-0000-0000"
  unsigned short d_hw_release;           // hardware release
  unsigned short d_hw_version;           // hardware version


  void output_error_msg (const char *msg, int err);
  void bail (const char *msg, int err) throw (std::runtime_error);
  void create_ringbuffer();


 protected:
  perseus_source_c (int sampling_rate, const std::string device_name,
			        bool ok_to_block);

 public:
  ~perseus_source_c ();

  void tune (float freq);
  int  set_attenuator (int levelInDb);
  void set_frontend_filters (bool activate);
  void set_adc_dither (bool f_dither);
  void set_adc_preamp (bool f_preamp);
  int  get_sn (void) { return d_serial_number; }
  std::string get_signature (void) { return std::string(d_signature); }
  int  get_hw_release (void) { return d_hw_release; }
  int  get_hw_version (void) { return d_hw_version; } 

  bool check_topology (int ninputs, int noutputs);

  int work (int ninput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
};

#endif /* INCLUDED_PERSEUS_SOURCE_C_H */

