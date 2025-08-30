 #include <stdio.h>
 #include <stdint.h>
 #include <stdlib.h>
 #include <time.h>
 #include <math.h>
 #include "packet.h"
 #include "channel.h"
 #include "crypto.h" 

 static inline double clamp(double x, double lo, double hi) {
 return (x<lo) ? lo : (x>hi) ? hi :x;
 }

 static inline double randf(double a, double b) {
 return a + (double)rand() / (double)RAND_MAX * (b-a);
 }

 int main() {

  uint8_t pk[KEM_PUB_BYTES], sk[KEM_SEC_BYTES];
  uint8_t kem_ct[KEM_CT_BYTES];
  uint8_t shared_key[AEAD_KEY_BYTES];

  mock_hqc_keypair(pk, sk);
  mock_hqc_encaps(pk, kem_ct, shared_key); // this is sender

// receiver is smilar process: İn the real world ct goes to the other side

  uint8_t shared_key_rx[AEAD_KEY_BYTES];
  mock_hqc_decaps(sk, kem_ct, shared_key_rx);

// let's assert that two keys match (for demo purposes)
int same=1;
for(int i=0; i<AEAD_KEY_BYTES; i++){
    if(shared_key[i]!=shared_key_rx[i]{same=0; break;}
}
if(!same) {
      fprintf(stderr,"KEM FAIL: shared keys differ, and chechk in \n");
      return 1;
}
  const double DT =0.1;
  const double V_MAX= 200.0;
  const double TANK_L = 300.0;
  const int STEPS =100;

  srand((unsigned)time(NULL)); 

//This seeds the RNG, sets initial speed/fuel/time, and resets the channel.  
  double v_kmh = 0.0;
  double fuel_L =TANK_L;
  uint32_t ts_ms= 0;
  channel_init(); // clear the channel

  for( int i =0; i<STEPS; i++) {
  
    double a_ms2 =((double)rand() /RAND_MAX) * (2.5 - (-1.5))+(-1.5);

  // Hard brake with a 5% chance

    if (((double)rand() /RAND_MAX) < 0.05) {;
      a_ms2 = ((double)rand() /RAND_MAX) * (-3.0 - (-5.0)) + (-5.0);
    }

    // (Δv = a*Δt, m/s → km/h için *3.6)
     v_kmh += a_ms2 * DT * 3.6;
    
    if (v_kmh <0.0) v_kmh = 0.0; 
    if (v_kmh > V_MAX) v_kmh = V_MAX;

   //// 3) Fuel consumption: base + depends on speed and acceleration
 
 double cons_Lps = 0.0005 + 0.00002 * v_kmh + 0.0003 * fabs(a_ms2);
 fuel_L -=cons_Lps *DT;

 if(fuel_L <0.0) fuel_L =0.0;
 if(fuel_L >TANK_L) fuel_L =TANK_L;

 ts_ms += (uint32_t)(DT * 1000.0);

 //Fill the package
 VehiclePacket pkt;
 pkt.speed_kmh = (uint16_t) llround(v_kmh);
 double fuel_pct = (fuel_L /TANK_L) * 100.0;
 pkt.fuel_pct_x10 = (uint16_t) llround(fuel_pct * 10.0);
 pkt.ts_ms = ts_ms;

 uint8_t wire[PACKET_WIRE_SIZE];

wire[0] = (uint8_t)pkt.speed_kmh; 
wire[1] = (uint8_t)(pkt.speed_kmh >>8);
wire[2] = (uint8_t)pkt.fuel_pct_x10;
wire[3] = (uint8_t)(pkt.fuel_pct_x10>>8);
wire[4] = (uint8_t)pkt.ts_ms;
wire[5] = (uint8_t)(pkt.ts_ms >> 8);
wire[6] = (uint8_t)(pkt.ts_ms >>16); 
wire[7] = (uint8_t)(pkt.ts_ms >> 24);

// AEAD_toy : enc.

// Note: Nonce = ts_ms in DEMO; must be unique/unpredictable in the actual system.

uint64_t nonce = pkt.ts_ms;
wire_encrypt_aead(wire, shared_key, nonce);

// If you want to test for (optional) error, break it a little before the label:

// wire[0] ^= 0x01;


// --- channel ----
channel_push(wire);

//--- Receiver: VERIFY + DECODER + parse ---
uint8_t rx[PACKET_WIRE_SIZE];
if(channel_pop(rx) == 0) {
  if(wire_decrypt_aead(rx, shared_key_rx, nonce ) ==0) {
   VehiclePacket out;
   out.speed_kmh = (uint8_t)(rx[0] | ((uint16_t)rx[1]<<8));
   out.fuel_pct_x10 = (uint16_t)(rx[2] | ((uint16_t)rx[3] <<8));
   out.ts_ms = (uint32_t)rx[4] | ((uint32_t)rx[5] <<8) | ((uint32_t)rx[6] <<16) |((uint32_t)rx[7] <<24);
  
  printf("SECURE OK | SPEED: %3u | FUEL: %6.1f %% |TS: %6u\n",out.speed_kmh,
                                 out.fuel_pct_x10 / 10.0, out.ts_ms);
  }
   else{ printf("SECURE FAIL (tag) \n");
  }
}

 packet_encode_flat(&pkt, wire);

//... ...

channel_push(wire);
uint8_t rx[PACKET_WIRE_SIZE];
if(channel_pop(rx) ==0) {
   VehiclePacket out;
   int ok = packet_decode_flat(rx, &out);
//... ...
}


// (Optional) Corrup a bit every 20 steps and test the CRC

// if ((i 20%) == 10) { wire[0] ^= 0x01; }

  channel_push(wire);
 
 uint8_t rx[PACKET_WIRE_SIZE];
 if(channel_pop(rx)==0) {
   VehiclePacket out;
   int ok = packet_decode_flat(rx, &out);
   if(ok==0) {
      printf("TX->RX OK | SPEED: %3u | FUEL: %6.1f %% | TS: %6u\n",
                      out.speed_kmh,out.fuel_pct_x10 /10.0,out.ts_ms);
    
   }else{ printf("CRC ERROR on RX\n");
   
   }

  }

} //This body chooses acceleration, updates speed/fuel, advances time,
 // encodes the packet, sends it through the channel, and verifies it at the receiver.
return 0 ;

}




  
