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


#ifndef INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_H
#define INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_H

#include <ieee_868_915/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace ieee_868_915 {

    /*!
     * \brief <+description of block+>
     * \ingroup ieee_868_915
     *
     */
    class IEEE_868_915_API symbols_to_chips_bs : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<symbols_to_chips_bs> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of ieee_868_915::symbols_to_chips_bs.
       *
       * To avoid accidental use of raw pointers, ieee_868_915::symbols_to_chips_bs's
       * constructor is in a private implementation
       * class. ieee_868_915::symbols_to_chips_bs::make is the public interface for
       * creating new instances.
       */
      static sptr make();
    };

  } // namespace ieee_868_915
} // namespace gr

#endif /* INCLUDED_IEEE_868_915_SYMBOLS_TO_CHIPS_BS_H */

