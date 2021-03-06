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


#include <iostream>
#include <gnuradio/io_signature.h>
#include "packet_sink_impl.h"

#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <gnuradio/blocks/count_bits.h>


#include <stdlib.h>
// very verbose output for almost each sample
#define VERBOSE 1
// less verbose output for higher level debugging
#define VERBOSE2 1


static const int DEFAULT_THRESHOLD = 1;  // detect access code with up to DEFAULT_THRESHOLD bits wrong
static const int DEFAULT_THRESHOLD_SFD = 1;

  // this is the mapping between chips and symbols if
  // With standard specification for frequency band of 868-915
  //

static const unsigned int CHIP_MAPPING[] = {31432,  
					   1335};  
//for sequence number 111101011001000
//for sequence number 000010100110111



namespace gr {
  namespace ieee_868_915 {

packet_sink::sptr
packet_sink::make(gr::msg_queue::sptr target_queue,	int threshold)
{
	return gnuradio::get_initial_sptr(new packet_sink_impl(target_queue, threshold));
}


inline void packet_sink_impl::enter_search()
{

  // From enter packet construction
  nbr_bits =0;
  byte =0;

  d_packetlen_cnt = 0;
  d_packet_byte = 0;
  d_packet_byte_index = 0;
  //end from packet construction

  d_state = STATE_SYNC_SEARCH;
  d_shift_reg = 0;

  d_preamble_cnt = 0;
  d_chip_cnt = 0;
  d_packet_byte = 0;

  bit_0 =15;
  bit_1 =15;
  bit_breamble =0;
  nbr_bit_sfd = 0;
  byte_sfd =0;
  inc = 0;
  find_preamble = false;
  search_sfd = false;
  first_found = false;
}

inline void
packet_sink_impl::enter_have_sync()
{
  //if (VERBOSE)
   // fprintf(stderr, "@ enter_have_sync\n");

  bit_0 =15;
  bit_1 =15;

  byte_sfd =0;
  nbr_bit_sfd =0;
  is_sfd =8;

  nbr_bits =0;
  byte =0;
  d_chip_cnt =0;
  d_shift_reg =0;

  d_state = STATE_HAVE_SYNC;
  d_packetlen_cnt = 0;
  d_packet_byte = 0;
  d_packet_byte_index = 0;
}

inline void packet_sink_impl::enter_have_header(int payload_len)
{
  if (VERBOSE)
    fprintf(stderr, "@ enter_have_header (payload_len = %d)\n", payload_len);
  
  d_shift_reg = 0;

  d_preamble_cnt = 0;
  d_chip_cnt = 0;
  d_packet_byte = 0;


  d_state = STATE_HAVE_HEADER;
  d_packetlen  = payload_len;
  d_payload_cnt = 0;
  d_packet_byte = 0;

  d_packet_byte_index = 0;

  d_byte = 0;
  nbr_bits = 0;
}

inline void packet_sink_impl::enter_search_sfd(){
	 d_state = STATE_SFD_SEARCH;

	 byte_sfd = 0;
	 d_shift_reg = 0;
	 d_chip_cnt = 0;

	 is_sfd = 8;

 	 nbr_bit_sfd = 0;

     bit_1 = 16;
     bit_0 = 16;

}


inline unsigned char
packet_sink_impl::decode_chips(unsigned int chips){
  int i;
  int best_match = 0xFF;
  int min_threshold = 16; // Matching to 15 chips, could never have a error of 16 chips

  for(i=0; i<2; i++) {
    // FIXME: we can store the last chip
    // ignore the first and last chip since it depends on the last chip.
    unsigned int threshold = blocks::count_bits16((chips&0x7FFF) ^ (CHIP_MAPPING[i]&0x7FFF));

    if (threshold < min_threshold) {
        best_match = i;
        min_threshold = threshold;
    }
  }

  if (min_threshold < d_threshold) {
    if (VERBOSE)
	  fprintf(stderr, "Found sequence with %d errors at 0x%x\n", min_threshold, (chips&0x7FFFFFFE) ^ (CHIP_MAPPING[best_match]&0x7FFFFFFE)), fflush(stderr);

    return (char)best_match&0xF;
  }

  return 0xFF;
}

    /*
     * The private constructor
     */
    packet_sink_impl::packet_sink_impl(gr::msg_queue::sptr target_queue,	int threshold)
      : gr::sync_block("packet_sink",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)), d_target_queue(target_queue), d_threshold(threshold == -1 ? DEFAULT_THRESHOLD : threshold)
    {
  d_sync_vector = 0xA7;
  d_processed = 0;
  nbr_preambles = 0;

  //if ( VERBOSE )
    //fprintf(stderr, "syncvec: %x, threshold: %d\n", d_sync_vector, d_threshold),fflush(stderr);
    enter_search();
	}

    /*
     * Our virtual destructor.
     */
    packet_sink_impl::~packet_sink_impl()
    {
    }

    int
    packet_sink_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {

        // Do <+signal processing+>


 float *inbuf = (float *) input_items[0];
  int count=0;

  
 // if (VERBOSE)
//    fprintf(stderr,">>> Entering state machine\n"),fflush(stderr);
  d_processed += noutput_items;

  //For the case of search the STATE_SYNC_SEARCH
  //is the bits sequence constructed by the ships passed

  while (count<noutput_items) {


    switch(d_state) {

    case STATE_SYNC_SEARCH:    // Look for sync vector

      //if (VERBOSE)
      // 	  fprintf(stderr,"SYNC Search, noutput=%d syncvec=%x\n",noutput_items, d_sync_vector),fflush(stderr);

      while (count < noutput_items) {

    	  if(slice(inbuf[count]))
    		  d_shift_reg = (d_shift_reg << 1) | 1;
    	  else
    		  d_shift_reg = d_shift_reg << 1;
    	  count++;
    	  d_chip_cnt ++;

    	  //return 0
    	   bit_0 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[0]& 0xfffE));
    	   //return 1
    	   bit_1 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[1]& 0xfffE));

    	  if ((bit_0 <= DEFAULT_THRESHOLD) || (bit_1 <= DEFAULT_THRESHOLD)){
    		  //printf("la valeur de d_shift_reg est : %x\n", d_shift_reg&0xffff);
    		  d_shift_reg = 0;
    		  d_chip_cnt = 0;

    		   if (not find_preamble){
    		     	search_sfd = false;
    		     	if ((bit_0 <=DEFAULT_THRESHOLD)&&(bit_1 >DEFAULT_THRESHOLD)){
    		     		//count the number of 0, the 0 is like 1
    		     		 bit_breamble = bit_breamble <<1 | 1;
    		     		 //printf("nbr de 0 %d Le chips num %d la sortie %d est d_shift_reg %x\n", inc, d_chip_cnt, count, d_shift_reg&0x7FFF);
    		     		 inc++;
    		     		 if (bit_breamble == 0xffffffff){
    		     			 if (VERBOSE)
    		     				 fprintf(stderr,"Find all preamble, %d\n", inc), fflush(stderr);
    		     			 enter_search_sfd();
    		     		     bit_breamble = 0;
    		     			 inc =0;
    		     		     nbr_preambles ++;
    		     		     fprintf(stderr,"Number of preambles, %d\n", nbr_preambles), fflush(stderr);
    		     			 //exit(0);
    		     			 break;
    		     		}
    		     	}else{
  		     		  if ((bit_1 <= DEFAULT_THRESHOLD) && (bit_0 > DEFAULT_THRESHOLD)){
    		    	  	 enter_search();
    		    	  	 break;
  		     		  }
    		     	}
    		   }
    		    //}if((not first_found) && (find_preamble)){ //if find preamble and a first bit of sfd wad found
    		    //Construct the successive bits of the SFD
    		   	//If it's the begin of the SFD search and the preamble is found
    	  }
      }
      break;

    case STATE_SFD_SEARCH:    // Look for sync vector

    	while ((count < noutput_items)) {
    		if(slice(inbuf[count]))
    			d_shift_reg = (d_shift_reg << 1) | 1;
    		else
    			d_shift_reg = d_shift_reg << 1;

    		count++;
    		d_chip_cnt ++;

    		//return 0
    		bit_0 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[0]& 0xfffE));
    		//return 1
    		bit_1 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[1]& 0xfffE));

    		if ((bit_0 <= DEFAULT_THRESHOLD) || (bit_1 <= DEFAULT_THRESHOLD)){
    			//printf("la valeur de d_shift_reg est : %x\n", d_shift_reg&0xffff);
    			d_shift_reg = 0;
    			d_chip_cnt = 0;

    			if ((bit_1 <= DEFAULT_THRESHOLD_SFD)&&(bit_0 >DEFAULT_THRESHOLD_SFD)&&(nbr_bit_sfd == 0))
    				first_found = true;

    			if (first_found){
    				if ((bit_0 <= DEFAULT_THRESHOLD_SFD)&&(bit_1 > DEFAULT_THRESHOLD_SFD)){
    					byte_sfd = byte_sfd << 1;
    					nbr_bit_sfd++;
    				}
    				//construct the stream of the 0 bits
    				if ((bit_1 <= DEFAULT_THRESHOLD_SFD)&&(bit_0 > DEFAULT_THRESHOLD_SFD)){
    					byte_sfd = byte_sfd << 1 | 1;
    					nbr_bit_sfd++;
    				}
    				//Test if the SFD is found
    				is_sfd = blocks::count_bits8((byte_sfd &0xff)^(0xa7 & 0xff));
    				//if ((byte_sfd == 0xa7)&&(nbr_bit_sfd ==8)){
    				if((is_sfd <= DEFAULT_THRESHOLD_SFD)&&(nbr_bit_sfd ==8) ){
    					printf("The value of SFD is : %x \n", byte_sfd&0xff);
    					is_sfd = 8;
    					if (VERBOSE)
    						fprintf(stderr,"Start Frame Delimeter was found \n"), fflush(stderr);
    					enter_have_sync();
    					break;
    					}else{
    						//the SFD field is not found
    						if((is_sfd >=DEFAULT_THRESHOLD_SFD)&&(nbr_bit_sfd ==8)){
    							//if ((byte_sfd != 0xa7)&&(nbr_bit_sfd ==8)){
    		    		    if (VERBOSE)
    		    			 fprintf(stderr,"The value of SFD is Wrong*********** : %x\n",byte_sfd&0xff), fflush(stderr);
    		    		     //printf("Start Frame Delimeter is not found \n");
    		    		     //Run the new search
    		    		     enter_search();
    		    		     break;
    						}
    		    	   }
    		       }
      	  	  }

    	  }//End of While loop
        break;

    case STATE_HAVE_SYNC:
      if (VERBOSE2)
    	  //fprintf(stderr,"Header Search bitcnt=%d, header=0x%08x\n", d_headerbitlen_cnt, d_header), fflush(stderr);

     // fprintf(stderr,"I am in the PHR field \n"), fflush(stderr);
      while (count < noutput_items) {		// Decode the bytes one after another.

    	  if(slice(inbuf[count]))
    		  d_shift_reg = (d_shift_reg << 1) | 1;
    	  else
    		  d_shift_reg = d_shift_reg << 1;

    	  d_chip_cnt = d_chip_cnt+1;
    	  count ++;

    	  //construct the byte by using the chips
          //return 0
          bit_0 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[0]& 0xfffE));
          //return 1
          bit_1 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[1]& 0xfffE));

          if ((bit_0 <= DEFAULT_THRESHOLD) || (bit_1 <=DEFAULT_THRESHOLD)){
        	      d_chip_cnt = 0;
        	      //fprintf(stderr,"The sequence of d_shift_reg %x \n", d_shift_reg&0xffff), fflush(stderr);
        		  //construct the stream of the 1 bits
        		  if ((bit_0 <= DEFAULT_THRESHOLD)&&(bit_1 >DEFAULT_THRESHOLD)){
        			  byte = byte << 1;
		     		  nbr_bits++;
        		  }
        		  //construct the stream of the 0 bits
        		  if ((bit_1 <=DEFAULT_THRESHOLD)&&(bit_0 >DEFAULT_THRESHOLD)){
		     		  byte = byte << 1 | 1;
		     		  nbr_bits++;
        		  }

        		  d_shift_reg =0;
        		  bit_1 =15;
        		  bit_0 =15;

        		  if (nbr_bits == 8){
        			  //found the Byte
        			  nbr_bits = 0;
        			  //fprintf(stderr,"The value of PHR is %x\n", byte), fflush(stderr);
        			  // we have a complete byte which repr      esents the frame length.
        			  int frame_len = byte;

        			  if(frame_len <= MAX_PKT_LEN){
        				  enter_have_header(frame_len);
        				  if (VERBOSE)
        					  fprintf(stderr,"The value of PHR is %d and a max_pkt_length %d \n", byte, MAX_PKT_LEN), fflush(stderr);
        			      //enter_search();
        			      //exit(0);
        			  } else {
        				  //exit(0);
        				  fprintf(stderr, "The value of PHR is wrong \n"), fflush(stderr);
        			      enter_search();
        			      break;
        			  }
        			  break;
        		  }
          }else{
        	  if (d_chip_cnt == 16){
        		  fprintf(stderr,"The sequence of d_shift_reg %x \n", d_shift_reg&0xffff), fflush(stderr);
        		  fprintf(stderr,"The chips are not equale to that of the table \n"), fflush(stderr);
        		  //exit(0);
        		  enter_search();
        	  }
          }
      	}
      break;

    case STATE_HAVE_HEADER:
      if (VERBOSE2)
    	 // fprintf(stderr,"Packet Build count=%d, noutput_items=%d, packet_len=%d\n", count, noutput_items, d_packetlen),fflush(stderr);

      while (count < noutput_items) {   // shift bits into bytes of packet one at a time
    	  if(slice(inbuf[count]))
    		  d_shift_reg = (d_shift_reg << 1) | 1;
    	  else
    		  d_shift_reg = d_shift_reg << 1;

    	  d_chip_cnt = d_chip_cnt+1;
    	  count ++;

    	  //construct the byte by using the chips
          //return 0
          bit_0 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[0]& 0xfffE));
          //return 1
          bit_1 = blocks::count_bits16((d_shift_reg&0x7ffE)^(CHIP_MAPPING[1]& 0xfffE));

          if ((bit_0 <=DEFAULT_THRESHOLD) || (bit_1 <=DEFAULT_THRESHOLD)){
    		  // the first symbol represents the first part of the byte.
    	      d_chip_cnt = 0;
    	      //fprintf(stderr,"The sequence of d_shift_reg %x \n", d_shift_reg&0xffff), fflush(stderr);
    		  //construct the stream of the 1 bits
    		  if ((bit_0 <=DEFAULT_THRESHOLD)&&(bit_1 >DEFAULT_THRESHOLD)){
    			  d_byte = d_byte << 1;
	     		  nbr_bits++;
    		  }
    		  //construct the stream of the 0 bits
    		  if ((bit_1 <=DEFAULT_THRESHOLD)&&(bit_0 >DEFAULT_THRESHOLD)){
	     		  d_byte = d_byte << 1 | 1;
	     		  nbr_bits++;
    		  }
			  bit_0 = 15;
			  bit_1 = 15;
			  d_shift_reg = 0;
    		  //fprintf(stderr, "%d: 0x%x\n", d_packet_byte_index, c);
    		  if (nbr_bits == 8){
    			  // we have a complete byte
    			  if (VERBOSE2)
    				  //std::cout<<"packetcnt: "<< d_packetlen_cnt <<" payloadcnt: " <<d_payload_cnt <<" d_packet_byte: " <<std::hex<<d_packet_byte <<"\n";
    				  fprintf(stderr, "packetcnt: %d, payloadcnt: %d, payload %x \n", d_packetlen_cnt, d_payload_cnt, d_byte), fflush(stderr);
    			  //Assambeled bytes
    			  d_packet[d_packetlen_cnt++] = d_byte;
    			  d_payload_cnt++;

    			  d_byte = 0;
    			  nbr_bits = 0;

    			  if (d_payload_cnt >= d_packetlen){	// packet is filled (rempli), including CRC. might do check later in here
    				  // build a message
    				  gr::message::sptr msg = gr::message::make(0, 0, 0, d_packetlen_cnt);
    				  //Load the packet in the memory space
    				  memcpy(msg->msg(), d_packet, d_packetlen_cnt);

    				  d_target_queue->insert_tail(msg);		// send it
    				  msg.reset();  						// free it up
    				  if(VERBOSE2)
    					  fprintf(stderr, "Adding message of size %d to queue\n", d_packetlen_cnt);
    				  enter_search();
    				  break;
    			  }
    		  }
    	  }else{
        	  if (d_chip_cnt == 16){
        		  fprintf(stderr,"The sequence of d_shift_reg %x \n", d_shift_reg&0xffff), fflush(stderr);
        		  //fprintf(stderr,"The chips are not equale to that of the table \n"), fflush(stderr);
        		  //exit(0);
        		  enter_search();
        	  }
    	  }
      }
      break;

    default:
      assert(0); // when there are no case is found--generate the error messages

    } // switch

  }   // while

  if(VERBOSE2)
    //fprintf(stderr, "Samples Processed: %d\n", d_processed), fflush(stderr);

  return noutput_items;
    }



  } /* namespace ieee_868_915 */
} /* namespace gr */

