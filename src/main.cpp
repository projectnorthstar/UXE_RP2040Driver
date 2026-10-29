#include <Arduino.h>
extern "C" void uxe_initialize(void);
extern "C" void uxe_poll(void);
void uxe_hid_begin(void);
void uxe_hid_poll(void);
void uxe_debug_poll(void);
extern "C" void uxe_settings_load(void);
extern "C" void uxe_settings_poll(void);
extern "C" void uxe_settings_changed(void);
#include <pico/bootrom.h>

void enterBootloader()
{
    reset_usb_boot(0, 0);
}
void setup()
{
    Serial.begin(115200);

    uxe_hid_begin();

    Serial.println("NS UXE Driver initialized");
    
    uxe_settings_load();
    uxe_initialize();
}

void loop()
{
    uxe_poll();
    uxe_hid_poll();
    
    uxe_settings_poll();
    static String serialBuffer = "";
    while (Serial.available() > 0)
    {
        char c = (char)Serial.read();

        if (c == '\n' || c == '\r')
        {
            serialBuffer.trim();

            if (serialBuffer.length() > 0)
            {
                if (serialBuffer == "USBDEBUG")
                {
                    Serial.println("Running USB diagnostics...");
                    uxe_debug_poll();
                }else if (serialBuffer == "SAVE"){
                    Serial.println("Running Test Save...");
                    uxe_settings_changed();
                }else if (serialBuffer == "BOOTLOADER"){                    
                    Serial.println("Entering Bootloader ...");
                    enterBootloader();
                }
                else
                {
                    Serial.print("Unknown command: ");
                    Serial.println(serialBuffer);
                }

                serialBuffer = "";
            }
        }
        else if (serialBuffer.length() < 128)
        {
            serialBuffer += c;
        }
    }
}
