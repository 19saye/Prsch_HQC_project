#pragma once
#include <stddef.h>
#include <stdint.h>
#include "packet.h"

#define CHANNEL_CAPACITY 256  // kaç paketlik halka kuyruklu olduğu sayıca 
#define CHANNEL_PKT_SIZE PACKET_WIRE_SIZE

int  channel_init(void); // ring buffer temizle
int  channel_push(const uint8_t pkt[CHANNEL_PKT_SIZE]); // göndericilik 
int  channel_pop(uint8_t pkt_out[CHANNEL_PKT_SIZE]);    //alıcılık
int  channel_count(void); // kuyrukta paket sayısı

