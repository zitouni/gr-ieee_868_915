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

#ifndef INCLUDED_IEEE_868_915_PACKET_SINK_IMPL_H
#define INCLUDED_IEEE_868_915_PACKET_SINK_IMPL_H

#include <ieee_868_915/packet_sink.h>

#include <gnuradio/msg_queue.h>


namespace gr {
  namespace ieee_868_915 {

    class packet_sink_impl : public packet_sink
    {
     public :

     //gr::ieee_868_915::packet_sink::sptr
     //typedef boost::shared_ptr<packet_sink> sptr;

      static sptr make (gr::msg_queue::sptr target_queue,
			      int threshold = -1	                // -1 -> use default
			      );

     private:
      // Nothing to declare in this block.
   enum state_t {STATE_SYNC_SEARCH, STATE_HAVE_SYNC, STATE_HAVE_HEADER};

  static const int MSG_LEN_POS   = 8+1;         // 8 byte sos header, 1 byte AM type
  static const int MAX_PKT_LEN    = 128 - MSG_LEN_POS - 1; // remove header and CRC

  //static const unsigned int CHIP_MAPPING[];     // will hold the mapping between chip and symbols

  gr::msg_queue::sptr  d_target_queue;		// where to send the packet when received
  unsigned int       d_sync_vector;		// 802.15.4 standard is 4x 0 bytes and 1x0xA7
  unsigned int	     d_threshold;		// how many bits may be wrong in sync vector

  state_t            d_state;

  unsigned short       		d_shift_reg;		// used to look for sync_vector

  int bit_0;
  int bit_1;
  int bit_breamble;
  int bit_sfd;
  int nbr_bit_sfd;
  int inc;
  bool find_preamble;
  bool search_sfd;
  bool first_found;

  int nbr_bits;
  int byte;

  int d_byte;


  int                d_preamble_cnt;            // count on where we are in preamble
  int                d_chip_cnt;                // counts the chips collected

  unsigned int       d_header;			// header bits
  int		     d_headerbitlen_cnt;	// how many so far

  unsigned char      d_packet[MAX_PKT_LEN];	// assembled payload
  unsigned char	     d_packet_byte;		// byte being assembled
  int		     d_packet_byte_index;	// which bit of d_packet_byte we're working on
  int 		     d_packetlen;		// length of packet
  int		     d_packetlen_cnt;		// how many so far
  int		     d_payload_cnt;		// how many bytes in payload
  int                d_processed;

protected:
//  packet_sink_impl(gr::msg_queue::sptr target_queue,
//				int threshold);
  
  void enter_search();
  void enter_have_sync();
  void enter_have_header(int payload_len);
  unsigned char decode_chips(unsigned int chips);
  int slice(float x) { return x > 0 ? 1 : 0; }
  
  bool header_ok()
  {
    // might do some checks on the header later...
    return 1;
  }



     public:
       packet_sink_impl(gr::msg_queue::sptr target_queue,
				int threshold);
      ~packet_sink_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);

  //! return true if we detect carrier
  bool carrier_sensed() const
  {
    return d_state != STATE_SYNC_SEARCH;
  }

    };

  } // namespace ieee_868_915
} // namespace gr

#endif /* INCLUDED_IEEE_868_915_PACKET_SINK_IMPL_H */

