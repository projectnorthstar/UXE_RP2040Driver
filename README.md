# NS UXE Driver — RP2040 PlatformIO USB HID

Arduino-Pico / Earle Philhower core, TinyUSB HID, product name **NS UXE Driver**.

Build with `pio run`. Uses the original display power sequencing and GPIO assignments.

## HID
Vendor-defined usage page 0xFF00. 64-byte HID reports including report ID: **output 0x01** and **input 0x02**, each containing 63 bytes after report ID. The output payload is `[command, sequence, payload_length, data...]`. Response payload is `[command, sequence, status, data_length, data...]`; status 0=OK, 1=unknown command, 2=bad length, 3=bad value. Commands:

| Command | Payload | Action |
|---|---|---|
| 0x01 | 1 byte brightness (0–255) | PWM GPIO0 |
| 0x02 | none | ADC/status |
| 0x03 | 1 byte: 0=auto, 1=off, 2=on | Fan GPIO20/21 |
| 0x04 | 1 byte: 0=90Hz, 1=60Hz | GPIO15 |
| 0x05 | 2-byte LE interval ms, 0=off | Telemetry streaming |
| 0x06 | none | Full status |

Status fields in response data (payload offsets 4–13): ADC raw LE16, sensor millivolts LE16, brightness u8, fan mode u8, fan on u8, refresh u8, stream interval LE16. Unsolicited telemetry uses command byte 0x80. Sensor measurement is **millivolts, not Celsius**.
