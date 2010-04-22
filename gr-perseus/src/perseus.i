/* -*- c++ -*- */
/*
 * Copyright 2004,2010 Free Software Foundation, Inc.
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

%include "gnuradio.i"				// the common stuff

%{
#include "perseus_source_c.h"
%}

// ----------------------------------------------------------------

GR_SWIG_BLOCK_MAGIC(perseus,source_c)

perseus_source_c_sptr
perseus_make_source_c (int sampling_rate,
		       const std::string dev = "",
		       bool ok_to_block = true
		      ) throw (std::runtime_error);

class perseus_source_c : public gr_sync_block {

 protected:
  perseus_source_c (int sampling_rate,
		    const std::string device_name,
		    bool ok_to_block
		   ) throw (std::runtime_error);

 public:
  void tune (float freq);
  int  set_attenuator (int levelInDb);
  void set_frontend_filters (bool activate);
  void set_adc_dither (bool f_dither);
  void set_adc_preamp (bool f_preamp);

  ~perseus_source_c ();
};


