/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "symbols_to_chips_bs_impl.h"

#include <assert.h>
#include <iostream>
#include <cstring>
#include <stdio.h>

//decimal coding of the squence chips of bits
static const unsigned int d_symbol_table[] = {31432,  
				               1335}; 
//for sequence number 111101011001000
 //for sequence number 000010100110111

static const int TABLE_SIZE = 2;

namespace gr {
  namespace ieee_868_915 {

    symbols_to_chips_bs::sptr
    symbols_to_chips_bs::make()
    {
      return gnuradio::get_initial_sptr
        (new symbols_to_chips_bs_impl());
    }

    /*
     * The private constructor
     */
    symbols_to_chips_bs_impl::symbols_to_chips_bs_impl()
      : gr::block("symbols_to_chips_bs",
              gr::io_signature::make(1, 1, sizeof(unsigned char)),
              gr::io_signature::make(1, 1, sizeof(unsigned short)))
    {}


    /*
     * Our virtual destructor.
     */
    symbols_to_chips_bs_impl::~symbols_to_chips_bs_impl()
    {
    }

    void
    symbols_to_chips_bs_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        ninput_items_required[0] = noutput_items;
    }

    int
    symbols_to_chips_bs_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
	const unsigned char *in = (const unsigned char *) input_items[0];
        unsigned short *out = (unsigned short *) output_items[0];

           for (int i = 0; i < noutput_items; i+=1){
     		 memcpy(&out[i], &d_symbol_table[(unsigned short)(in[i]&0x1)], sizeof(unsigned short));
		}
   	 // end of per stream processing

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace ieee_868_915 */
} /* namespace gr */

