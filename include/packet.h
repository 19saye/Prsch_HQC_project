#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t speed_kmh;     // km/h
    uint16_t fuel_pct_x10;  // %0.1 (e.g., 52.4% -> 524)
    uint32_t ts_ms;         // ms timestamp
} VehiclePacket;

// Flat wire format = 10 bytes total:
// [speed(uint16 LE)=2][fuel_x10(uint16 LE)=2][ts_ms(uint32 LE)=4][crc16=2]
#define PACKET_WIRE_SIZE 10

size_t   packet_encode_flat(const VehiclePacket* in, uint8_t out[PACKET_WIRE_SIZE]);
int      packet_decode_flat(const uint8_t in[PACKET_WIRE_SIZE], VehiclePacket* out);
uint16_t packet_crc16_ccitt(const uint8_t* data, size_t len);

