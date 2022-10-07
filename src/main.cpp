

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <string.h>

extern "C"
{
#include "TMC-API/tmc/ic/TMC2160/TMC2160.h"
}

#include "autoreboot.h"

const int GPIO_STEPPER_0_5A = 3;
const int GPIO_STEPPER_1_0A = 2;
const int GPIO_STEPPER_2_0A = 1;
const int GPIO_STEPPER_4_0A = 0;

const int GPIO_X_STEPPER_SPI_CS = 14;
const int GPIO_Y_STEPPER_SPI_CS = 15;
const int GPIO_Z_STEPPER_SPI_CS = 13;
const int GPIO_A_STEPPER_SPI_CS = 9;


extern "C"
{
  void set_spi_cs(int channel, int value){
    if(channel == 0){
      gpio_put(GPIO_X_STEPPER_SPI_CS, value);
    } else if(channel == 1){
      gpio_put(GPIO_Y_STEPPER_SPI_CS, value);
    } else if(channel == 2){
      gpio_put(GPIO_Z_STEPPER_SPI_CS, value);
    } else if(channel == 3){
      gpio_put(GPIO_A_STEPPER_SPI_CS, value);
    }
  }

  void tmc2160_readWriteArray(uint8_t channel, uint8_t *data, size_t length)
  {
    uint8_t inputBuffer[length] = {0};
    set_spi_cs(channel, 0);
    spi_write_read_blocking(spi1, data, inputBuffer, length);
    set_spi_cs(channel, 1);
    memcpy(data, inputBuffer, length);
  }
}


void initPeripherals() {

  // Enable SPI 0 at 1 MHz and connect to GPIOs
  spi_init(spi1, 1000000);
  gpio_set_function(12, GPIO_FUNC_SPI);
  gpio_set_function(10, GPIO_FUNC_SPI);
  gpio_set_function(11, GPIO_FUNC_SPI);
  spi_set_format(spi1, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

  gpio_init(GPIO_X_STEPPER_SPI_CS);
  gpio_set_dir(GPIO_X_STEPPER_SPI_CS, GPIO_OUT);
  gpio_init(GPIO_Y_STEPPER_SPI_CS);
  gpio_set_dir(GPIO_Y_STEPPER_SPI_CS, GPIO_OUT);
  gpio_init(GPIO_Z_STEPPER_SPI_CS);
  gpio_set_dir(GPIO_Z_STEPPER_SPI_CS, GPIO_OUT);
  gpio_init(GPIO_A_STEPPER_SPI_CS);
  gpio_set_dir(GPIO_A_STEPPER_SPI_CS, GPIO_OUT);

  gpio_init(GPIO_STEPPER_0_5A);
  gpio_set_dir(GPIO_STEPPER_0_5A, GPIO_IN);
  gpio_init(GPIO_STEPPER_1_0A);
  gpio_set_dir(GPIO_STEPPER_1_0A, GPIO_IN);
  gpio_init(GPIO_STEPPER_2_0A);
  gpio_set_dir(GPIO_STEPPER_2_0A, GPIO_IN);
  gpio_init(GPIO_STEPPER_4_0A);
  gpio_set_dir(GPIO_STEPPER_4_0A, GPIO_IN);
}


float getConfiguredCurrent(){
      
  float current = 0.0;

  if (gpio_get(GPIO_STEPPER_0_5A) == 0) {
    current += 0.5;
  }
  if (gpio_get(GPIO_STEPPER_1_0A) == 0) {
    current += 1.0;
  }
  if (gpio_get(GPIO_STEPPER_2_0A) == 0) {
    current += 2.0;
  }
  if (gpio_get(GPIO_STEPPER_4_0A) == 0) {
    current += 4.0;
  }

  if (current > 4.0) {
    current = 4.0;
  }
  return current; 
}

void setStepperCurrent(TMC2160TypeDef& stepperDriver, float current) {

  const float maxCurrent = 7.5;
  float fractionOfMaxCurrent = current / maxCurrent;

  if (fractionOfMaxCurrent > 0.5) {
    
    int runSetting = 31*fractionOfMaxCurrent;
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_IHOLD_IRUN, TMC2160_IRUN_MASK, TMC2160_IRUN_SHIFT, runSetting);
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_IHOLD_IRUN, TMC2160_IHOLD_MASK, TMC2160_IHOLD_SHIFT, runSetting);
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_GLOBAL_SCALER, TMC2160_GLOBAL_SCALER_MASK, TMC2160_GLOBAL_SCALER_SHIFT, 256);

    printf("Set IRUN, IHOLD to %d and GLOBAL SCALER to %d\r\n", runSetting, 256);
  
  } else {

    int runSetting = 2*31*fractionOfMaxCurrent;
    
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_IHOLD_IRUN, TMC2160_IRUN_MASK, TMC2160_IRUN_SHIFT, runSetting);   
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_IHOLD_IRUN, TMC2160_IHOLD_MASK, TMC2160_IHOLD_SHIFT, runSetting);
    TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_GLOBAL_SCALER, TMC2160_GLOBAL_SCALER_MASK, TMC2160_GLOBAL_SCALER_SHIFT, 128);

    printf("Set IRUN, IHOLD to %d and GLOBAL SCALER to %d\r\n", runSetting, 128);

  }
}

void configureStepperDriver(TMC2160TypeDef& stepperDriver){
  tmc2160_reset(&stepperDriver);
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_DRV_CONF, TMC2160_OTSELECT_MASK, TMC2160_OTSELECT_SHIFT, 2);

  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_GCONF, TMC2160_EN_PWM_MODE_MASK, TMC2160_EN_PWM_MODE_SHIFT, 1);
  printf("Configure PWM_THRS\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_TPWMTHRS, TMC2160_TPWMTHRS_MASK, TMC2160_TPWMTHRS_SHIFT, 500);
  
  setStepperCurrent(stepperDriver, getConfiguredCurrent());

  printf("Configure TOFF\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_TOFF_MASK, TMC2160_TOFF_SHIFT, 3);
  printf("Configure HSTRT\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_HSTRT_MASK, TMC2160_HSTRT_SHIFT, 4);
  printf("Configure HEND\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_HEND_MASK, TMC2160_HEND_SHIFT, 1);
  printf("Configure TBL\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_TBL_MASK, TMC2160_TBL_SHIFT, 2);
  printf("Configure CHM\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_CHM_MASK, TMC2160_CHM_SHIFT, 0);
  printf("Configure INTPOL\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_INTPOL_MASK, TMC2160_INTPOL_SHIFT, 1);
  printf("Configure MRES\r\n");
  TMC2160_FIELD_WRITE(&stepperDriver, TMC2160_CHOPCONF, TMC2160_MRES_MASK, TMC2160_MRES_SHIFT, 4);

}




void readPins(TMC2160TypeDef& stepperDriver){
    int32_t status = tmc2160_readInt(&stepperDriver, TMC2160_IOIN___OUTPUT);
    int step = (status & TMC2160_STEP_MASK) >> TMC2160_STEP_SHIFT;
    int dir = (status & TMC2160_DIR_MASK) >> TMC2160_DIR_SHIFT;
    int enablen = (status & TMC2160_DRV_ENN_MASK) >> TMC2160_DRV_ENN_SHIFT;
    printf("STEP: %d DIR %d ENABLEn %d\r\n", step, dir, enablen);
}

int main()
{
  stdio_init_all();

  initPeripherals();

  sleep_ms(1000);

  TMC2160TypeDef stepperDriverX;
  ConfigurationTypeDef stepperDriverXConfig;
  int32_t stepperDriverXRegisterResetState;
  tmc2160_init(&stepperDriverX, 0, &stepperDriverXConfig, &stepperDriverXRegisterResetState);
  configureStepperDriver(stepperDriverX);

  TMC2160TypeDef stepperDriverY;
  ConfigurationTypeDef stepperDriverYConfig;
  int32_t stepperDriverYRegisterResetState;
  tmc2160_init(&stepperDriverY, 1, &stepperDriverYConfig, &stepperDriverYRegisterResetState);
  configureStepperDriver(stepperDriverY);

  TMC2160TypeDef stepperDriverZ;
  ConfigurationTypeDef stepperDriverZConfig;
  int32_t stepperDriverZRegisterResetState;
  tmc2160_init(&stepperDriverZ, 2, &stepperDriverZConfig, &stepperDriverZRegisterResetState);
  configureStepperDriver(stepperDriverZ);


  TMC2160TypeDef stepperDriverA;
  ConfigurationTypeDef stepperDriverAConfig;
  int32_t stepperDriverARegisterResetState;
  tmc2160_init(&stepperDriverA, 3, &stepperDriverAConfig, &stepperDriverARegisterResetState);
  configureStepperDriver(stepperDriverA);

  float stepper_current = 0.0;
  while (true)
  {
    sleep_ms(1000);
    printf("Stepper Current: %f\r\n", getConfiguredCurrent());

    if (stepper_current != getConfiguredCurrent()){
      stepper_current = getConfiguredCurrent();
      setStepperCurrent(stepperDriverX, getConfiguredCurrent());
      setStepperCurrent(stepperDriverY, getConfiguredCurrent());
      setStepperCurrent(stepperDriverZ, getConfiguredCurrent());
      setStepperCurrent(stepperDriverA, getConfiguredCurrent());
    }
    else {
      printf("No change in current\r\n");
    }
  }
}