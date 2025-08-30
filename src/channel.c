  #include <string.h>
  #include "channel.h"

  static uint8_t rb[CHANNEL_CAPACITY][CHANNEL_PKT_SIZE];
  static int head=0, tail=0, count_=0;

  int channel_init(void) 
   { head = tail = count_ =0;
     return 0;
   }

  int channel_push(const uint8_t pkt[CHANNEL_PKT_SIZE]) 
   {
     if(count_ >= CHANNEL_CAPACITY) return -1; // ISFULL
       memcpy(rb[head], pkt, CHANNEL_PKT_SIZE);
       head = (head+1) % CHANNEL_CAPACITY ;
       count_++;
     return 0;
   }

  int channel_pop(uint8_t pkt_out[CHANNEL_PKT_SIZE]) 
   {
     if(count_ <= 0) return -1; // ISEMPTY
       memcpy(pkt_out, rb[tail], CHANNEL_PKT_SIZE);
       tail = (tail + 1) % CHANNEL_CAPACITY;
       count_--;
     return 0;
   }
  
  int channel_count(void) 
    { return count_; }  
      
