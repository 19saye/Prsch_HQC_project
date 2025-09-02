 #include <stdint.h>
 #include <string.h>
 #include "packet.h"

 static inline void le16_write(uint8_t *p, uint16_t v) 
  { p[0]=(uint8_t)v;
    p[1]=(uint8_t)(v>>8);
   }

 static inline uint16_t le16_read(const uint8_t *p) 
  { return (uint16_t)(p[0] | ((uint16_t)p[1]<<8)); }

 static inline void le32_write(uint8_t *p,uint32_t v) 
  { p[0] = (uint8_t)v; 
    p[1] = (uint8_t)(v>>8);
    p[2] = (uint8_t)(v>>16);
    p[3] = (uint8_t)(v>>24);
  }

 static inline uint32_t le32_read(const uint8_t *p) 
  { return (uint32_t)p[0]
         | ((uint32_t) p[1]<<8) 
         | ((uint32_t)p[2]<<16)
         | ((uint32_t)p[3]<<24);
  }

 uint16_t packet_crc16_ccitt(const uint8_t *data, size_t len) 
  { uint16_t crc = 0xFFFF;
    for(size_t i=0; i<len; i++) {
      crc ^= (uint16_t)data[i] <<8;
        for(int b=0; b<8; b++) {
          crc = (crc & 0x8000 ) ? (uint16_t)((crc<<1) ^0x1021) : (uint16_t)(crc<<1);
        }
     }
     return crc;
   }

 size_t packet_encode_flat(const VehiclePacket * in, uint8_t out[PACKET_WIRE_SIZE]) 
   {  // 0..1 speed, 2..3 fuel_x10, 4..7 ts_ms, 8..9 crc
      le16_write(out+0 , in-> speed_kmh);
      le16_write(out+2 , in-> fuel_pct_x10);
      le32_write(out+4 , in-> ts_ms);
 
      uint16_t crc = packet_crc16_ccitt(out,8);
      le16_write(out+8 , crc);
      return PACKET_WIRE_SIZE;
   }
 
  int packet_decode_flat(const uint8_t in[PACKET_WIRE_SIZE], VehiclePacket *out) 
   {
      uint16_t exp = packet_crc16_ccitt(in,8);
      uint16_t got = le16_read(in+8);
      
        if(exp != got) return -1; // CRCERROR!
       
      out->speed_kmh  = le16_read(in+0);
      out->fuel_pct_x10 = le16_read(in+2);
      out->ts_ms = le32_read(in+4);
 
      return 0;
  }
  
