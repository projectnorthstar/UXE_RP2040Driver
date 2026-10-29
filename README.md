# NS UXE Driver — RP2040 PlatformIO USB HID

Arduino-Pico / Earle Philhower core, TinyUSB HID, product name **NS UXE Driver**.

Build with `pio run`. Uses the original display power sequencing and GPIO assignments.

To upload the pre-compiled binaries, plug your UXE in while holding the BootSEL button, then run copmile-for-release.ps1

Otherwise, use vscode/vscodium, install the platformIO extension, then run compile-and-upload.ps1 (or use the extension to compile, up to you!)

The USB declares two devices:
- HID (For app hardware control)
- Serial (For debugging)

With Serial you have 3 commands (send with \n break end):
- BOOTLOADER 
-- Reboots the device in bootloader mode
- USBDEBUG
-- Runn a quick diagnostic on the USB Enumerator
- SAVE
-- Performs a test save of the current settings

NOTE: Changing the refresh rate of the display requires the device to be replugged in

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

Status fields in response data (payload offsets 4–13): ADC raw LE16, board temperature (x10 celcius) LE16, brightness u8, fan mode u8, fan on u8, refresh u8, stream interval LE16. Unsolicited telemetry uses command byte 0x80. 

## License

This project is licensed under the **MIT License**.

Copyright (c) 2026 Brewed Engagement Extended Reality Labs &  Shenzhen Sudo Electronics Technology Limited
* www.beerlabs.com.au
* www.sudotronic.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND**, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.