# Mini Report — HQC-Secured Vehicle Telemetry

## 1. Goal
The goal is to demonstrate a small end-to-end telemetry pipeline with educational cryptography.
We generate speed, fuel, and timestamp, protect them, send over a mock channel, verify, and print.

## 2. Method
- Packet: 10-byte flat format with CRC-16 CCITT.
- Crypto: mock HQC-KEM for a shared key + toy AEAD for confidentiality and integrity.
- Channel: small FIFO buffer to move packets between sender and receiver.

## 3. Flow (Text)
Generate data → Pack fields → AEAD encrypt + tag → Send via channel → Receive → Verify tag → Decrypt → Parse → Print.

## 4. Sample Output
See `README.md` for example runs and how to reproduce a clean execution.

## 5. Limitations
Cryptography is educational and not secure for production.
Vehicle physics is very simple; results are only illustrative.

## 6. Future Work
- Replace toy crypto with a real library.
- Add unit tests in `tests/`.
- Extend vehicle model and packet fields.
