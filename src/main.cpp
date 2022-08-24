

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <string.h>

extern "C"
{
#include "TMC-API/tmc/ic/TMC2160/TMC2160.h"
}

#include "autoreboot.h"

extern "C"
{
  void tmc2160_readWriteArray(uint8_t channel, uint8_t *data, size_t length)
  {
    uint8_t inputBuffer[length] = {0};
    spi_write_read_blocking(spi1, data, inputBuffer, length);

    memcpy(data, inputBuffer, length);
  }
}

int main()
{
  stdio_init_all();

  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_init(spi1, 1000000);
  gpio_set_function(12, GPIO_FUNC_SPI);
  gpio_set_function(10, GPIO_FUNC_SPI);
  gpio_set_function(11, GPIO_FUNC_SPI);
  gpio_set_function(9, GPIO_FUNC_SPI);
  spi_set_format(spi1, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

  // Enable this for developping, it will reboot the board on USB when pressing only the USB_BOOT button
  autoreboot_init();

  const int GPIO_STEPPER_0_5A = 3;
  const int GPIO_STEPPER_1_0A = 2;
  const int GPIO_STEPPER_2_0A = 1;
  const int GPIO_STEPPER_4_0A = 0;

  gpio_init(GPIO_STEPPER_0_5A);
  gpio_set_dir(GPIO_STEPPER_0_5A, GPIO_IN);
  gpio_init(GPIO_STEPPER_1_0A);
  gpio_set_dir(GPIO_STEPPER_1_0A, GPIO_IN);
  gpio_init(GPIO_STEPPER_2_0A);
  gpio_set_dir(GPIO_STEPPER_2_0A, GPIO_IN);
  gpio_init(GPIO_STEPPER_4_0A);
  gpio_set_dir(GPIO_STEPPER_4_0A, GPIO_IN);
  sleep_ms(1000);

  TMC2160TypeDef tmc2160;
  uint8_t channel = 0;
  ConfigurationTypeDef config;
  int32_t registerResetState;

  tmc2160_init(&tmc2160, channel, &config, &registerResetState);
  tmc2160_reset(&tmc2160);
  while (tmc2160.config->state != CONFIG_READY)
  {
    printf("Initializing TMC2160 registers\n");
    tmc2160_periodicJob(&tmc2160, 0);
    sleep_ms(10);
  }

  while (true)
  {
    int32_t status = tmc2160_readInt(&tmc2160, TMC2160_IOIN___OUTPUT);
    printf("Status: %d\n", status);
    sleep_ms(1000);

    printf("GPIO: %d %d %d %d \n", gpio_get(GPIO_STEPPER_0_5A), gpio_get(GPIO_STEPPER_1_0A), gpio_get(GPIO_STEPPER_2_0A), gpio_get(GPIO_STEPPER_4_0A));
  }
}