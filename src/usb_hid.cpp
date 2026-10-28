#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <string.h>
extern "C" {
uint16_t uxe_get_adc(void), uxe_get_mv(void), uxe_get_stream_interval(void);
uint8_t uxe_get_brightness(void), uxe_get_refresh(void), uxe_get_fan_mode(void), uxe_get_fan_on(void);
void uxe_set_brightness(uint8_t), uxe_set_refresh(uint8_t), uxe_set_fan_mode(uint8_t), uxe_set_stream_interval(uint16_t);
}
// Vendor-defined HID, 63 data bytes + one report ID = 64 bytes on wire.
static const uint8_t descriptor[] = {
  0x06,0x00,0xFF, 0x09,0x01, 0xA1,0x01,
  0x15,0x00, 0x26,0xFF,0x00, 0x75,0x08, 0x95,0x3F,
  0x85,0x01, 0x09,0x01, 0x91,0x02,
  0x85,0x02, 0x09,0x02, 0x81,0x02, 0xC0
};
Adafruit_USBD_HID hid;
static volatile bool pending=false;
static uint8_t incoming[63];
static uint8_t incoming_len=0;
static uint8_t outgoing[63];
static bool response_pending=false;
static uint32_t last_stream=0;

static void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t len) {
  if(report_id!=1 || report_type!=HID_REPORT_TYPE_OUTPUT || !buffer || pending) return;
  if(len>63)len=63;
  memcpy(incoming,buffer,len); incoming_len=(uint8_t)len;
  pending=true;
}

static uint16_t get_report(uint8_t, hid_report_type_t, uint8_t*, uint16_t){return 0;}
void uxe_hid_begin()
{
    // USB device identification
    TinyUSBDevice.setManufacturerDescriptor("NorthStar Org");
    TinyUSBDevice.setProductDescriptor("NorthStar UXE Driver");

    // Register HID descriptor and callbacks
    hid.setPollInterval(2);

    hid.setReportDescriptor(
        descriptor,
        sizeof(descriptor)
    );

    hid.setReportCallback(
        get_report,
        set_report
    );

    // Register HID interface with TinyUSB
    hid.begin();
}
void uxe_debug_poll(){

    Serial.println("NS UXE USB Diagnostics");

    Serial.print("TinyUSB mounted: ");
    Serial.println(TinyUSBDevice.mounted() ? "YES" : "NO");

    Serial.print("HID ready: ");
    Serial.println(hid.ready() ? "YES" : "NO");
}
static void status_fields(uint8_t* b) {
  uint16_t adc=uxe_get_adc(), mv=uxe_get_mv(), interval=uxe_get_stream_interval();
  b[4]=adc&255; b[5]=adc>>8; b[6]=mv&255; b[7]=mv>>8;
  b[8]=uxe_get_brightness(); b[9]=uxe_get_fan_mode();
  b[10]=uxe_get_fan_on(); b[11]=uxe_get_refresh();
  b[12]=interval&255; b[13]=interval>>8;
}
void uxe_hid_poll() {
  if(pending && !response_pending) {
    uint8_t b[63]={0};
    uint8_t len=incoming_len;
    memcpy(b,incoming,len);
    pending=false;
    uint8_t cmd=len?b[0]:0, seq=len>1?b[1]:0, n=len>2?b[2]:0;
    memset(outgoing,0,sizeof(outgoing));
    outgoing[0]=cmd; outgoing[1]=seq;
    uint8_t status=0;
    if(len<3 || n>60 || n>(uint8_t)(len-3))status=2;
    else switch(cmd) {
      case 1: if(n!=1)status=2;else uxe_set_brightness(b[3]);break;
      case 2: if(n!=0)status=2;break;
      case 3: if(n!=1)status=2;else if(b[3]>2)status=3;else uxe_set_fan_mode(b[3]);break;
      case 4: if(n!=1)status=2;else if(b[3]>1)status=3;else uxe_set_refresh(b[3]);break;
      case 5: if(n!=2)status=2;else uxe_set_stream_interval((uint16_t)b[3]|((uint16_t)b[4]<<8));break;
      case 6: if(n!=0)status=2;break;
      default: status=1;break;
    }
    outgoing[2]=status;
    outgoing[3]=(status==0 && (cmd==2 || cmd==6))?10:0;
    if(outgoing[3])status_fields(outgoing);
    response_pending=true;
  }
  if(response_pending) {
    if(hid.ready() && hid.sendReport(2,outgoing,sizeof(outgoing))) response_pending=false;
    return;
  }
  uint16_t interval=uxe_get_stream_interval();
  uint32_t now=millis();
  if(interval && (uint32_t)(now-last_stream)>=interval && hid.ready()) {
    uint8_t b[63]={0}; b[0]=0x80; b[3]=10; status_fields(b);
    if(hid.sendReport(2,b,sizeof(b)))last_stream=now;
  }
}
