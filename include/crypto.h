#pragma once
#include <stdint.h>
#include <stddef.h>

#define KEM_PUB_BYTES   32
#define KEM_SEC_BYTES   32
#define KEM_CT_BYTES    16
#define AEAD_KEY_BYTES  32

// Mock HQC-KEM (eğitsel; gerçek güvenlik sağlamaz)
int mock_hqc_keypair(uint8_t pk[KEM_PUB_BYTES], uint8_t sk[KEM_SEC_BYTES]);
int mock_hqc_encaps(const uint8_t pk[KEM_PUB_BYTES],
                    uint8_t ct[KEM_CT_BYTES],
                    uint8_t shared_key[AEAD_KEY_BYTES]);
int mock_hqc_decaps(const uint8_t sk[KEM_SEC_BYTES],
                    const uint8_t ct[KEM_CT_BYTES],
                    uint8_t shared_key[AEAD_KEY_BYTES]);

// AEAD-toy (eğitsel; gerçek güvenlik sağlamaz)
// wire10: [0..7]=ciphertext (8B), [8..9]=tag(2B)
int wire_encrypt_aead(uint8_t wire10[10],
                      const uint8_t key[AEAD_KEY_BYTES],
                      uint64_t nonce);
int wire_decrypt_aead(uint8_t wire10[10],
                      const uint8_t key[AEAD_KEY_BYTES],
                      uint64_t nonce);

 
