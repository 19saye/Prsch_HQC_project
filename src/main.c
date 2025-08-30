#include <stdio.h>
#include <stdint.h>
#include "packet.h"

int main() {
    VehiclePacket pkt;
    
        pkt.speed_kmh =0;
        pkt.fuel_pct_x10 = 3000;
        pkt.ts_ms=0;
    

     for (int i =0 ; i<10 ; i++) {
          
         pkt.speed_kmh +=5;
         pkt.fuel_pct_x10 -=1;
         pkt.ts_ms +=100;
    printf("SPEED:%u km/h \n", pkt.speed_kmh);
    printf("FUEL: %.1f %%\n", pkt.fuel_pct_x10 / 10.0);
    printf("TIMESTAMP: %u ms\n", pkt.ts_ms);
  }

    return 0;
}

