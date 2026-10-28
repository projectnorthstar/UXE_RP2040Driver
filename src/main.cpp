#include <Arduino.h>
extern "C" void uxe_initialize(void);
extern "C" void uxe_poll(void);
void uxe_hid_begin(void);
void uxe_hid_poll(void);
void uxe_debug_poll(void);
void setup()
{
    Serial.begin(115200);

    uxe_initialize();
    uxe_hid_begin();

    Serial.println("NS UXE Driver initialized");
}

void loop()
{
    uxe_poll();
    uxe_hid_poll();
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
