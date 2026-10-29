// Permanent flash storage for the display driver's settings

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
extern "C" {
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include <hardware/watchdog.h>
uint8_t uxe_get_brightness(void), uxe_get_refresh(void), uxe_get_fan_mode(void);
void uxe_settings_apply(uint8_t, uint8_t, uint8_t);
}

void rebootRP2040()
{
    watchdog_reboot(0, 0, 0);
    while (true) {
        tight_loop_contents();
    }
}

#ifndef PICO_FLASH_SIZE_BYTES
#error "PICO_FLASH_SIZE_BYTES must be defined by the board configuration"
#endif
static constexpr uint32_t kOffset = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
static constexpr uint32_t kMagic = 0x31584555u;
static constexpr uint8_t kVersion = 1;
static constexpr uint32_t kDelay = 1500;
struct __attribute__((packed)) Record {
  uint32_t magic;
  uint8_t version, brightness, refresh, fan_mode;
  uint32_t checksum;
};
static_assert(sizeof(Record)==12, "Unexpected record layout");
static bool dirty=false;
static bool rebootAfterSave=false;
static uint32_t lastChange=0;
static int nextPage=0;
static uint32_t hash(const Record &r) {
  uint32_t h=2166136261u;
  const uint8_t *p=reinterpret_cast<const uint8_t*>(&r);
  for(size_t i=0;i<offsetof(Record,checksum);++i) h=(h^p[i])*16777619u;
  return h;
}
static bool valid(const Record &r) {
  return r.magic==kMagic && r.version==kVersion && r.refresh<=1 &&
         r.fan_mode<=2 && r.checksum==hash(r);
}
static const uint8_t* pageAddress(int index) {
  return reinterpret_cast<const uint8_t*>(XIP_BASE+kOffset+index*FLASH_PAGE_SIZE);
}
extern "C" void uxe_settings_load(void) {
  Record latest{};
  bool found=false;
  nextPage=FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE;
  for(int i=0;i<FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE;i++) {
    Record r{};
    memcpy(&r,pageAddress(i),sizeof(r));
    if(r.magic==0xFFFFFFFFu) {
      if(nextPage==FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE) nextPage=i;
    } else if(valid(r)) {latest=r;found=true;}
  }
  if(found) {
    uxe_settings_apply(latest.brightness,latest.refresh,latest.fan_mode);
    Serial.println("UXE: settings restored");
  } else {
    uxe_settings_apply(200,0,0);
    Serial.println("UXE: defaults loaded");
  }
  dirty=false;
}
extern "C" void uxe_settings_changed(void) {
  dirty=true;
  lastChange=millis();
}

extern "C" void uxe_settings_changeReboot(void) {
  dirty=true;
  rebootAfterSave=true;
  lastChange=millis();
}
extern "C" void uxe_settings_poll(void) {
  if(!dirty || (uint32_t)(millis()-lastChange)<kDelay) return;
  Record r{};
  r.magic=kMagic; r.version=kVersion;
  r.brightness=uxe_get_brightness();
  r.refresh=uxe_get_refresh(); r.fan_mode=uxe_get_fan_mode();
  r.checksum=hash(r);
  alignas(4) uint8_t page[FLASH_PAGE_SIZE];
  memset(page,0xFF,sizeof(page));
  memcpy(page,&r,sizeof(r));

  // Flash writes MUST NOT be enabled unless the sector is reserved.
#ifndef UXE_SETTINGS_FLASH_RESERVED
  Serial.println("UXE: save skipped: reserve last flash sector and define UXE_SETTINGS_FLASH_RESERVED");
  dirty=false;
  return;
#else
  // Only suitable for single-core firmware with no concurrent flash access.
  // The RP2040 executes code from XIP flash; interrupts must be disabled.
  const bool erase=(nextPage>=FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE);
  const uint32_t offset=kOffset+(erase?0:nextPage*FLASH_PAGE_SIZE);
  uint32_t irq=save_and_disable_interrupts();
  if(erase) flash_range_erase(kOffset,FLASH_SECTOR_SIZE);
  flash_range_program(offset,page,FLASH_PAGE_SIZE);
  restore_interrupts(irq);
  nextPage=erase?1:nextPage+1;
  Record verify{};
  memcpy(&verify,reinterpret_cast<const void*>(XIP_BASE+offset),sizeof(verify));
  if(valid(verify) && verify.checksum==r.checksum) {
    dirty=false;
    Serial.println("UXE: settings saved");
    if(rebootAfterSave){
      rebootAfterSave = false;    
      Serial.println("UXE: Display Refresh Changed, rebooting");
      rebootRP2040();
    }
  } else {
    lastChange=millis();
    Serial.println("UXE: flash verification failed");
  }
#endif
}
