#ifndef AUTOREBOOT_H
#define AUTOREBOOT_H

#include <hardware/structs/ioqspi.h>

#include "autoreboot.h"
#include <cstdio>
#include <hardware/gpio.h>
#include <hardware/regs/psm.h>
#include <hardware/structs/sio.h>
#include <hardware/sync.h>
#include <pico/bootrom.h>
#include <pico/critical_section.h>
#include <pico/stdlib.h>

class CriticalSection {
  critical_section_t cs;

public:
  CriticalSection() {
    critical_section_init(&cs);
    critical_section_enter_blocking(&cs);
  }
  CriticalSection(unsigned int locknum) {
    critical_section_init_with_lock_num(&cs, locknum);
    critical_section_enter_blocking(&cs);
  }
  ~CriticalSection() {
    critical_section_exit(&cs);
    critical_section_deinit(&cs);
  }
};
namespace BootselReset {

    const uint CS_PIN_INDEX = 1;
    
    void set_chip_select_override(const gpio_override override) {
      hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                      override << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                      IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);
    }
    
    bool __no_inline_not_in_flash_func(get_bootsel_button)() {
      CriticalSection critSection;
    
      set_chip_select_override(GPIO_OVERRIDE_LOW);
    
      for (unsigned int i = 0; i < 1000; ++i) {
        __asm("nop");
      };
    
      bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));
    
      set_chip_select_override(GPIO_OVERRIDE_NORMAL);
    
      return button_state;
    }
    void check_for_bootsel_reset() {
      if (get_bootsel_button()) {
        sleep_ms(100);
        if (get_bootsel_button()) {
          reset_usb_boot(0, 0);
        }
      }
    }
    } // namespace BootselReset

    bool timer_callback(repeating_timer_t *rt) {
      if (BootselReset::get_bootsel_button()) {
        reset_usb_boot(0, 0);
      }
      return true;
    }
    repeating_timer_t timer;

    bool autoreboot_init() {
      int hz = 5;
    // negative timeout means exact delay (rather than delay between callbacks)
      if (!add_repeating_timer_us(-1000000 / hz, timer_callback, NULL, &timer)) {
          printf("Failed to add timer\n");
          return false;
      }
      return true;
    }

#endif // AUTOREBOOT_H