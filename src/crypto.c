#include "crypto.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ---------- Yardımcılar (toy PRF + CRC16) ----------
// (Eğitsel amaçlı; gerçek güvenlik sağlamaz)

static inline uint32_t rotl32(uint32_t x, int r) {
    return (uint32_t)((x << r) | (x >> (32 - r)));
}

static inline uint32_t mix32(uint32_t a, uint32_t b) {
    a ^= rotl32(b + 0x9e3779b9u, 5);
    a *= 0x85ebca6bu;
    return a ^ (a >> 13);
}

// 8 baytlık anahtar akışı üret (key + nonce → 8B keystream)
static void toy_prf8(const uint8_t key[AEAD_KEY_BYTES],
                     uint64_t nonce,
                     uint8_t out[8]) {
    uint32_t s0 = 0x12345678u;
    uint32_t s1 = 0x9e3779b9u;

    // Anahtarı 32 bit parçalara katla
    for (int i = 0; i < AEAD_KEY_BYTES; i += 4) {
        uint32_t w = (uint32_t)key[i]
                   | ((uint32_t)key[i+1] << 8)
                   | ((uint32_t)key[i+2] << 16)
                   | ((uint32_t)key[i+3] << 24);
        s0 = mix32(s0, w);
        s1 = mix32(s1, s0 ^ w);
    }

    // Nonce’u katla
    uint32_t n0 = (uint32_t)nonce;
    uint32_t n1 = (uint32_t)(nonce >> 32);
    s0 = mix32(s0, n0);
    s1 = mix32(s1, n1);

    uint64_t ks = ((uint64_t)s0 << 32) ^ (uint64_t)s1;
    for (int i = 0; i < 8; i++) out[i] = (uint8_t)((ks >> (i * 8)) & 0xFF);
}

// CRC16-CCITT (0x1021, init=0xFFFF, xorout=0)
static uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000u) crc = (uint16_t)((crc << 1) ^ 0x1021u);
            else               crc = (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static inline void le16_write(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)(v >> 8);
}
static inline uint16_t le16_read(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

// ---------- Mock KEM (eğitsel) ----------

#if 0
static void fill_rand(uint8_t *buf, size_t n) {
    for (size_t i = 0; i < n; i++) buf[i] = (uint8_t)(rand() & 0xFF);
}
#endif

// Basit: ct rastgele; shared_key sadece ct’den türetilir → iki uç eşleşir
int mock_hqc_keypair(uint8_t pk[KEM_PUB_BYTES],
                     uint8_t sk[KEM_SEC_BYTES]) {
    for (int i = 0; i < KEM_SEC_BYTES; i++) {
        sk[i] = (uint8_t)(i * 37u + 11u);
    }
    for (int i = 0; i < KEM_PUB_BYTES; i++) {
        pk[i] = sk[i % KEM_SEC_BYTES] ^ 0xA5;
    }
    return 0;
}

int mock_hqc_encaps(const uint8_t pk[KEM_PUB_BYTES],
                    uint8_t ct[KEM_CT_BYTES],
                    uint8_t shared_key[AEAD_KEY_BYTES]) {
    for (int i = 0; i < KEM_CT_BYTES; i++) {
        ct[i] = (uint8_t)(i * 13u + 7u);
    }
    for (int i = 0; i < AEAD_KEY_BYTES; i++) {
        shared_key[i] = ct[i % KEM_CT_BYTES] ^ pk[i % KEM_PUB_BYTES];
    }
    return 0;
}


int mock_hqc_decaps(const uint8_t sk[KEM_SEC_BYTES],
                    const uint8_t ct[KEM_CT_BYTES],
                    uint8_t shared_key[AEAD_KEY_BYTES]) {
    uint8_t pk_local[KEM_PUB_BYTES];
    for (int i = 0; i < KEM_PUB_BYTES; i++) {
        pk_local[i] = sk[i % KEM_SEC_BYTES] ^ 0xA5;
    }
    for (int i = 0; i < AEAD_KEY_BYTES; i++) {
        shared_key[i] = ct[i % KEM_CT_BYTES] ^ pk_local[i % KEM_PUB_BYTES];
    }
    return 0;
}

// ---------- AEAD-toy: 8B şifreli alan + 2B tag = 10B tel ----------

int wire_encrypt_aead(uint8_t wire10[10],
                      const uint8_t key[AEAD_KEY_BYTES],
                      uint64_t nonce) {
    // Girdi: wire10[0..7] = AÇIK METİN (speed,fuel_x10,ts_ms LE)
    // Çıktı: wire10[0..7] = ŞİFRELİ METİN, wire10[8..9] = tag(LE)
    uint8_t ks[8];
    toy_prf8(key, nonce, ks);

    // Şifreleme (XOR keystream)
    for (int i = 0; i < 8; i++) wire10[i] ^= ks[i];

    // Tag: CRC(cipher[0..7]) XOR nonce parçaları
    uint16_t tag = crc16_ccitt(wire10, 8);
    tag ^= (uint16_t)nonce;
    tag ^= (uint16_t)(nonce >> 16);
    tag ^= (uint16_t)(nonce >> 32);
    tag ^= (uint16_t)(nonce >> 48);

    le16_write(wire10 + 8, tag);
    return 0;
}

int wire_decrypt_aead(uint8_t wire10[10],
                      const uint8_t key[AEAD_KEY_BYTES],
                      uint64_t nonce) {
    // Önce etiketi doğrula
    uint16_t tag_stored = le16_read(wire10 + 8);

    uint16_t tag_calc = crc16_ccitt(wire10, 8);
    tag_calc ^= (uint16_t)nonce;
    tag_calc ^= (uint16_t)(nonce >> 16);
    tag_calc ^= (uint16_t)(nonce >> 32);
    tag_calc ^= (uint16_t)(nonce >> 48);

    if (tag_calc != tag_stored) return -1;

    // Etiket doğruysa çöz
    uint8_t ks[8];
    toy_prf8(key, nonce, ks);
    for (int i = 0; i < 8; i++) wire10[i] ^= ks[i];

    // Çıkış: wire10[0..7] = AÇIK METİN
    return 0;
}

 
