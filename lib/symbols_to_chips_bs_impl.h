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

#ifndef INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_IMPL_H
#define INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_IMPL_H

#include <ieee_868_915/symbols_to_chips_bs.h>

namespace gr {
  namespace ieee_868_915 {

    class symbols_to_chips_bs_impl : public symbols_to_chips_bs
    {
     private:
      // Nothing to declare in this block.

     public:
      symbols_to_chips_bs_impl();
      ~symbols_to_chips_bs_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace ieee_868_915
} // namespace gr

#endif /* INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_IMPL_H */

