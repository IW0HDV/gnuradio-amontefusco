/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gr_null_sink.h>
#include <gr_io_signature.h>

gr_null_sink::gr_null_sink (size_t sizeof_stream_item)
  : gr_sync_block ("null_sink",
		   gr_make_io_signature (1, 1, sizeof_stream_item),
		   gr_make_io_signature (0, 0, 0))
{
}

gr_block_sptr
gr_make_null_sink (size_t sizeof_stream_item)
{
  return gr_block_sptr (new gr_null_sink (sizeof_stream_item));
}

int
gr_null_sink::work (int noutput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items)
{
  return noutput_items;
}
