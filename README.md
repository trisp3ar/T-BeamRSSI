# T-Beam RSSI

RSSI monitor firmware for the LilyGO TTGO T-Beam T22, built with the Arduino framework.

Each device periodically broadcasts a LoRa `Ping` and records nearby nodes that respond. The main screen shows the link status, latest RSSI, local node ID, discovered peers, and battery level. Press the button briefly to change peer pages. Hold it for 700 ms to open the menu.

## Purpose

This firmware was designed for LoRa coverage testing, troubleshooting, and collecting empirical data from real-world scenarios in order to compare and improve simulation models.

## Menu

- **Node ID**: View or change the 16-bit node ID. Each device should use a unique ID; `0xFFFF` is reserved for broadcast. Short press increments the ID and long press saves it.
- **TX Power**: Select `10`, `14`, or `20 dBm`. Short press cycles the values and long press saves the selection.
- **Encryption**: Choose communication mode. `AES-256` encrypts all LoRa payloads, while `Plaintext` sends and receives unencrypted payloads.
- **Run RSSI Sweep**: Send 10 broadcast pings to check nearby nodes and their signal strength.
- **Factory Reset**: Clear saved settings, generate a new node ID, and restore TX power to `14 dBm`.
- **Exit**: Save the current settings and return to the main screen.

In the menu, short press moves to the next item and long press selects or confirms it.

## LoRa AES-256 Encryption

All LoRa payloads are encrypted with AES-256 before transmission and decrypted on reception.

- Encryption key file: `data/LoRa-Key.json`
- JSON field used by firmware: `aes256_key_hex`
- Format: 64 hex characters (32 bytes)

Example:

```json
{
	"aes256_key_hex": "9f4c2a7e3b1d8c50a6e9f2147bd83c11e52a9d70c4b6f1382d7e5a91bc03f6d4"
}
```

After changing the key, upload filesystem contents so `/LoRa-Key.json` is updated on the device:

```bash
pio run -t uploadfs
```

Important: every node in your LoRa network must use the same key.