# HQC-Secured Vehicle Telemetry (Educational)

This mini project simulates a vehicle producing simple telemetry (speed, fuel, timestamp).  
Data is encoded into a 10-byte flat packet with CRC-16, protected by a toy AEAD, and sent over a mock channel.  
On the receiver, the packet is verified and printed.

> **Security note:** The cryptography is educational and **not** for production.

## Build & Run
```bash
make
./HQC_rt_secure --steps 50 --dt 0.005 | tee /tmp/run.log
src/ C sources (channel, crypto, packet, main)
include/ headers
docs/ mini-report
tests/ simple smoke script
Makefile build rules

