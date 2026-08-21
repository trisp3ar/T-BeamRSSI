# T-Beam RSSI

RSSI monitor firmware for the LilyGO TTGO T-Beam T22, built with the Arduino framework.

Each device periodically broadcasts a LoRa `Ping` and records nearby nodes that respond. The main screen shows the link status, latest RSSI, local node ID, discovered peers, and battery level. Press the button briefly to change peer pages. Hold it for 700 ms to open the menu.

## Purpose

This firmware was designed for LoRa coverage testing, troubleshooting, and collecting empirical data from real-world scenarios in order to compare and improve simulation models.

## Menu

- **Node ID**: View or change the 16-bit node ID. Each device should use a unique ID; `0xFFFF` is reserved for broadcast. Short press increments the ID and long press saves it.
- **TX Power**: Select `10`, `14`, or `20 dBm`. Short press cycles the values and long press saves the selection.
- **Run RSSI Sweep**: Send 10 broadcast pings to check nearby nodes and their signal strength.
- **Factory Reset**: Clear saved settings, generate a new node ID, and restore TX power to `14 dBm`.
- **Exit**: Save the current settings and return to the main screen.

In the menu, short press moves to the next item and long press selects or confirms it.