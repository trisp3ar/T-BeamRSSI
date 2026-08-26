# T-Beam RSSI

RSSI monitor firmware for the LilyGO TTGO T-Beam T22, built with the Arduino framework.

Each device periodically broadcasts a LoRa `Ping` and records nearby nodes that respond. The main screen shows the local node ID, battery level, GPS satellite count (or `NO GPS` before any data arrives), discovered peers with their RSSI and distance, and the device's own GPS location. Press the button briefly to change peer pages. Hold it for 700 ms to open the menu.

## Main Screen

- **Top-left**: local node ID
- **Top-right**: battery state of charge, followed by the satellite icon and satellite count once GPS data has been received, or `NO GPS` if no GPS bytes have been received yet
- **Peer list**: each entry shows the peer's node ID, RSSI in dBm, and (if both the local and peer position are known) the distance to that peer
- **Bottom-left**: the device's own last known GPS position (latitude/longitude with hemisphere letters), or `N- E-` if no position has been recorded
- **Bottom-right**: current peer page

## Purpose

This firmware was designed for LoRa coverage testing, troubleshooting, and collecting empirical data from real-world scenarios in order to compare and improve simulation models.

## Menu

- **Node ID**: View or change the 16-bit node ID. Each device should use a unique ID; `0xFFFF` is reserved for broadcast, node IDs are persistent and generated at random during initial flashing, they don't change when updating the firmware
- **TX Power**: Select `10`, `14`, or `20 dBm`. Short press cycles the values and long press saves the selection.
- **Encryption**: Choose communication mode. `AES-256` encrypts all LoRa payloads, while `Plaintext` sends and receives unencrypted payloads.
- **Run RSSI Sweep**: Send 10 broadcast pings to check nearby nodes and their signal strength.
- **Factory Reset**: Clear saved settings, generate a new node ID, and restore TX power to `14 dBm`.
- **Exit**: Save the current settings and return to the main screen.

In the menu, short press moves to the next item and long press selects or confirms it.

## Message Encryption

LoRa payloads can be encrypted with AES-256 if required.

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

Important: AES is a symmetric encryption method and every node in your LoRa network must use the same key