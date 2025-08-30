       #include <stdint.h>
       #include <string.h>
       #include "crypto.h"
       #include <time.h>
       #include <stdlib.h>

// (LE write/read and CRC16)
qqq
     static inline void le16w(uint8_t *p, uint16_t v) {
            p[0] = (uint8_t)v;
            p[1] = (uint8_t)(v>>8); }
     
     static inline uint16_t le16r(const uint8_t *p) {
            return (uint16_t)
           (p[0] | ((uint16_t)
            p[1] <<8 )); }

     static inline void mix_u32(uint32_t *x, uint32_t v) {
            *x ^= v+ 0x9e3779b9u + (*x<<6) + (*x>>2); }

//  CRC16-CCITT (0x1021), init=0xFFFF

     static uint16_t crc16_ccitt(const uint8_t *data , size_t len) {
            uint16_t crc = 0xFFFF;
       for(size_t i=0; i<len; i++) {
          crc ^= (uint16_t)data[i] << 8;
     
             for(int b=0; b<8; b++) {
                crc = (crc &0x8000) ? (uint16_t)((crc<<1) ^ 0x1021) : (uint16_t)(crc<<1);
             }
        }
     return crc;
   }
// A simple “PRF” imitation: Generate 32‐byte keystream (NOT SAFE)

static void toy_prf32(const uint8_t key[AEAD_KEY_BYTES] ,
                            uint64_t nonce,
                            uint8_t out[32],) {
                            uint32_t s0= 0x12345678u,
                                     s1= 0xdeadbeefu, 
                                     s2= 0xa5a5a5a5u,
                                     s3= 0x1f2e3d4cu;
                for(int i=0; i<32; i+=4) {
                          
                    uint32_t w= (uint32_t) key[i] |
                  ((uint32_t)key[i+1]<<8) |
                  ((uint32_t)key[i+2]<<16) |
                  ((uint32_t)key[i+3]<<24);
      mix_u32(&0, w);
      mix_u32(&s1, s0);
      mix_u32(&s2, s1);
      mix_u32(&s3, s2);
 }

//Nonce mix

 mix_u32(&s0, (uint32_t)nonce);
 mix_u32(&s1, (uint32_t)(nonce>>32));

//Output produce
         uint32_t t=s0^s1^s2^s3;
         for(int i=0; i<32; i++) {
           t ^= (t<<13);
           t ^= (t>>7);
           t ^= (t<<17);
           out[i] =(uint8_t)t;
         }
}

// Mock HQC_KEM 

int mock_hqc_keypair(uint8_t pk[KEM_PUB_BYTES],
                     uint8_t sk[KEM_SEC_BYTES] ){

   for(int i=0; i<KEM_PUB_BYTES; i++) 
    
     pk[i] = (uint8_t)(rand() & 0xFF);
     memcpy(sk, pk, KEM_PUB_BYTES);
  
   return 0;

}

int mock_hqc_encaps(const uint8_t pk[KEM_PUB_BYTES],
                          uint8_t ct[KEM_CT_BYTES],
                          uint8_t shared_key[AEAD_KEY_BYTES]) {
// Ct random, shared_key = PRF(pk||ct) (NOT SAFE — demo

      for(int i=0; i<KEM_CT_BYTES; i++) ct[i] = (uint8_t)(rand() & 0xFF);
                                                 uint8_t speed[48];
      memcpy(speed, pk, KEM_PUB_BYTES);
      memcpy(speed+KEM_PUB_BYTES, ct, KEM_CT_BYTES);

//PRF imitation: derivative from seed's CRC
uint16_t c= crc16_ccitt(seed, sizeof(speed));
uint8_t base[32];

      for(int i=0; i<32; i++) base[i] = (uint8_t)(c + i*31);
                toy_prf32(base, 0xBADC0FFEEULL, shared_key);
      return 0;
 
}

  int mock_hqc_decaps( const uint8_t sk[KEM_SEC_BYTES],
                       const uint8_t ct[KM_CT_BYTES],
                             uint8_t shared_key[AEAD_KeY_BYTES]) {
      uint8_t speed[48];
        memcpy(speed, sk, KEM_SEC_BYTES);
        memcpy(speed+KEM_SEC_BYTES, ct, KEM_CT_BYTES);
     uint16_t c= crc16_ccitt(speed, sizeof(speed));
     uint8_t base[32];

  for(int i=0; i<32; i++) base[i] = (uint8_t)(c + i*31);
         toy_prf32(base, 0xBADC0FFEEULL, shared_key);
    return 0;
  }
// wire = [0..7]=ciphertext (8B), [8..9]=tag (2B)

// Encryption: C = P XOR KS; tag = CRC16(key || nonce || C)

   int wire_encrypt_aead(uint8_t wire10[10],
                   const uint8_t key[32],
                         uint64 nonce) {

// Encrypt 8B payload

   uint8_t ks[32];
     toy_prf32(key, nonce, ks);
         for(int i=0; i<8; i++) wire10[i] ^=ks[i];
   uint8_t buf[32+8+8];
     memcpy(buf,key,32);
     memcpy(buf+32, &nonce, 8);
     memcpy(buf+40, wire10, 8);
   uint16_t tag = crc16_ccitt(buf, 48);
 
  le16w(wire10+8, tag);
  return 0;

}

// Decrypt: verify tag, then P = C XOR KS

   int wire_decrypt_aead(uint8_t wire[10],
                   const uint8_t key[32], 
                         uint64_t nonce) {

// verify tag

      uint16_t tag_stored = le16r(wire10+8);
      uinbt8_t buf[32+8+8];
         memcpy(buf, key, 32);
         memcpy(buf+32, &nonce, 8);
         memcpy(buf+40, wire10, 8);
      uint16_t tag_calc = crc16_ccitt(buf, 48);
  if(tag_calc != tag_stored) return -1,


//// keystream &  solve

     uint8_t ks[32];
      toy_prf(key, nonce, ks);
    
   for(int i=0; i<8; i++) wire10[i] ^= ks[i];
      return 0;

}

      

 
