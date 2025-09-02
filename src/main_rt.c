#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "packet.h"
#include "channel.h"
#include "crypto.h"

static inline double clamp(double x, double lo, double hi) {
    return (x < lo) ? lo : (x > hi ? hi : x);
}

static inline double randf(double a, double b) {
    return a + (double)rand() / (double)RAND_MAX * (b - a);
}

int main(int argc, char** argv) {
        // default
  int  STEPS = 100;
double DT = 0.1;
double V_MAX = 200.0;
double TANK_L = 300.0;

 
 for (int ai = 1; ai < argc; ++ai) {
    if (strcmp(argv[ai], "--steps") == 0 && ai + 1 < argc) {
      STEPS = atoi(argv[++ai]);
    } else if (strcmp(argv[ai], "--dt") == 0 && ai + 1 < argc) {
      DT = atof(argv[++ai]);
    } else if (strcmp(argv[ai], "--help") == 0) {
      printf("Usage: %s [--steps N] [--dt SECONDS]\n", argv[0]);
      return 0;
    }
  }

    // --- KEM: paylaşılan anahtar eşitle ---
    uint8_t pk[KEM_PUB_BYTES], sk[KEM_SEC_BYTES];
    uint8_t ct[KEM_CT_BYTES];
    uint8_t shared_tx[AEAD_KEY_BYTES], shared_rx[AEAD_KEY_BYTES];

    srand((unsigned)time(NULL));
    mock_hqc_keypair(pk, sk);
    mock_hqc_encaps(pk, ct, shared_tx);
    mock_hqc_decaps(sk, ct, shared_rx);

    int same = 1;
    for (int i = 0; i < AEAD_KEY_BYTES; i++) {
        if (shared_tx[i] != shared_rx[i]) { same = 0; break; }
    }
    if (!same) {
        fprintf(stderr, "KEM FAIL: shared keys differ\n");
        return 1;
    } else {
        printf("KEM OK: shared key established (%d bytes)\n", AEAD_KEY_BYTES);
    }

    // --- RT simülasyon ---

    double v_kmh  = 0.0;
    double fuel_L = TANK_L;
    uint32_t ts_ms = 0;

    channel_init();

    for (int i = 0; i < STEPS; i++) {
        // Rastgele ivme; %5 şiddetli fren
        double a_ms2 = randf(-1.5, 2.5);
        if ((double)rand() / RAND_MAX < 0.05) a_ms2 = randf(-5.0, -3.0);

        v_kmh  = clamp(v_kmh + a_ms2 * DT * 3.6, 0.0, V_MAX);
        double cons_Lps = 0.0005 + 0.00002 * v_kmh + 0.0003 * fabs(a_ms2);
        fuel_L = clamp(fuel_L - cons_Lps * DT, 0.0, TANK_L);
        ts_ms += (uint32_t)(DT * 1000.0);

        // --- 8B düz veriyi LE olarak hazırla (speed,fuel_x10,ts_ms) ---
        uint16_t speed      = (uint16_t)llround(v_kmh);
        double   fuel_pct   = (fuel_L / TANK_L) * 100.0;
        uint16_t fuel_x10   = (uint16_t)llround(fuel_pct * 10.0);

        uint8_t wire[PACKET_WIRE_SIZE]; // 10
        wire[0] = (uint8_t)(speed & 0xFF);
        wire[1] = (uint8_t)(speed >> 8);
        wire[2] = (uint8_t)(fuel_x10 & 0xFF);
        wire[3] = (uint8_t)(fuel_x10 >> 8);
        wire[4] = (uint8_t)(ts_ms & 0xFF);
        wire[5] = (uint8_t)(ts_ms >> 8);
        wire[6] = (uint8_t)(ts_ms >> 16);
        wire[7] = (uint8_t)(ts_ms >> 24);

        // --- AEAD-toy şifrele + etiketle (nonce = ts_ms; DEMO amaçlı) ---
        uint64_t nonce = (uint64_t)ts_ms;
        wire_encrypt_aead(wire, shared_tx, nonce);


        // Kanal üzerinden gönder/al

//         //   if((i%20) == 10) { wire[0] ^=0x01; } // optional corruption ( every 20th step)
        if (channel_push(wire) == 0) {
            uint8_t rx[PACKET_WIRE_SIZE];
            if (channel_pop(rx) == 0) {
                if (wire_decrypt_aead(rx, shared_rx, nonce) == 0) {
                    // Çözülen düz veriyi struct’a aç
                    VehiclePacket out;
                    out.speed_kmh     = (uint16_t)(rx[0] | ((uint16_t)rx[1] << 8));
                    out.fuel_pct_x10  = (uint16_t)(rx[2] | ((uint16_t)rx[3] << 8));
                    out.ts_ms         = (uint32_t)rx[4]
                                      | ((uint32_t)rx[5] << 8)
                                      | ((uint32_t)rx[6] << 16)
                                      | ((uint32_t)rx[7] << 24);

                    printf("SECURE OK | SPEED:%4u | FUEL:%6.1f %% | TS:%6u\n",
                           out.speed_kmh, out.fuel_pct_x10 / 10.0, out.ts_ms);
                } else {
                    printf("SECURE FAIL (tag)\n");
                }
            }
        }
    }

    return 0;
}

