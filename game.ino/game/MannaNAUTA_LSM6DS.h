#ifndef MANNANAUTA_LSM6DS_H
#define MANNANAUTA_LSM6DS_H

#include <Arduino.h>

class TwoWire;

#define MANNANAUTA_LSM6DS_DEFAULT_SDA 5
#define MANNANAUTA_LSM6DS_DEFAULT_SCL 4
#define MANNANAUTA_LSM6DS_I2C_ADDR_DEFAULT 0x6A

#ifndef SDA_PIN
#define SDA_PIN MANNANAUTA_LSM6DS_DEFAULT_SDA
#endif
#ifndef SCL_PIN
#define SCL_PIN MANNANAUTA_LSM6DS_DEFAULT_SCL
#endif

#define LSM6DS_I2CADDR_DEFAULT MANNANAUTA_LSM6DS_I2C_ADDR_DEFAULT

typedef enum {
  LSM6DS_RATE_SHUTDOWN = 0x00,
  LSM6DS_RATE_12_5_HZ = 0x01,
  LSM6DS_RATE_26_HZ = 0x02,
  LSM6DS_RATE_52_HZ = 0x03,
  LSM6DS_RATE_104_HZ = 0x04,
  LSM6DS_RATE_208_HZ = 0x05,
  LSM6DS_RATE_416_HZ = 0x06,
  LSM6DS_RATE_833_HZ = 0x07,
  LSM6DS_RATE_1_66K_HZ = 0x08,
  LSM6DS_RATE_3_33K_HZ = 0x09,
  LSM6DS_RATE_6_66K_HZ = 0x0A
} lsm6ds_data_rate_t;

typedef enum {
  LSM6DS_ACCEL_RANGE_2_G = 0x00,
  LSM6DS_ACCEL_RANGE_16_G = 0x01,
  LSM6DS_ACCEL_RANGE_4_G = 0x02,
  LSM6DS_ACCEL_RANGE_8_G = 0x03
} lsm6ds_accel_range_t;

typedef enum {
  LSM6DS_GYRO_RANGE_125_DPS = 0x02,
  LSM6DS_GYRO_RANGE_250_DPS = 0x00,
  LSM6DS_GYRO_RANGE_500_DPS = 0x04,
  LSM6DS_GYRO_RANGE_1000_DPS = 0x08,
  LSM6DS_GYRO_RANGE_2000_DPS = 0x0C,
  ISM330DHCX_GYRO_RANGE_4000_DPS = 0x01
} lsm6ds_gyro_range_t;

typedef enum {
  LSM6DS_HPF_ODR_DIV_50 = 0,
  LSM6DS_HPF_ODR_DIV_100 = 1,
  LSM6DS_HPF_ODR_DIV_9 = 2,
  LSM6DS_HPF_ODR_DIV_400 = 3
} lsm6ds_hp_filter_t;

typedef enum {
  MANNANAUTA_LSM6DS_AUTO = 0,
  MANNANAUTA_LSM6DSOX,
  MANNANAUTA_LSM6DSO32,
  MANNANAUTA_LSM6DS3,
  MANNANAUTA_LSM6DS33,
  MANNANAUTA_LSM6DS3TRC,
  MANNANAUTA_LSM6DSL,
  MANNANAUTA_ISM330DHCX
} mannanauta_lsm6ds_variant_t;

typedef struct {
  float x;
  float y;
  float z;
  uint32_t timestamp;
} mannanauta_lsm6ds_vector_t;

typedef struct {
  float temperature;
  uint32_t timestamp;
} mannanauta_lsm6ds_temperature_t;

class MannaNAUTA_LSM6DS {
public:
  MannaNAUTA_LSM6DS();

  bool begin();
  bool begin(int sdaPin, int sclPin);
  bool begin(int sdaPin, int sclPin, uint8_t i2cAddr,
             TwoWire *wire = nullptr,
             mannanauta_lsm6ds_variant_t variant = MANNANAUTA_LSM6DS_AUTO);
  bool beginWithWire(uint8_t i2cAddr = MANNANAUTA_LSM6DS_I2C_ADDR_DEFAULT,
                     TwoWire *wire = nullptr,
                     mannanauta_lsm6ds_variant_t variant =
                         MANNANAUTA_LSM6DS_AUTO);

  bool getEvent(mannanauta_lsm6ds_vector_t *accel,
                mannanauta_lsm6ds_vector_t *gyro,
                mannanauta_lsm6ds_temperature_t *temp);

  int readAcceleration(float &x, float &y, float &z);
  float accelerationSampleRate(void);
  int accelerationAvailable(void);

  int readGyroscope(float &x, float &y, float &z);
  float gyroscopeSampleRate(void);
  int gyroscopeAvailable(void);

  float readTemperature(void);
  bool temperatureAvailable(void);

  void setBoardOrientationCorrection(bool enabled);
  bool boardOrientationCorrectionEnabled(void) const;

  lsm6ds_data_rate_t getAccelDataRate(void);
  void setAccelDataRate(lsm6ds_data_rate_t dataRate);

  lsm6ds_accel_range_t getAccelRange(void);
  void setAccelRange(lsm6ds_accel_range_t range);

  lsm6ds_data_rate_t getGyroDataRate(void);
  void setGyroDataRate(lsm6ds_data_rate_t dataRate);

  lsm6ds_gyro_range_t getGyroRange(void);
  void setGyroRange(lsm6ds_gyro_range_t range);

  void reset(void);
  void configIntOutputs(bool activeLow, bool openDrain);
  void configInt1(bool drdyTemp, bool drdyGyro, bool drdyAccel,
                  bool stepDetect = false, bool wakeup = false);
  void configInt2(bool drdyTemp, bool drdyGyro, bool drdyAccel);
  void highPassFilter(bool enabled, lsm6ds_hp_filter_t filter);

  void enableWakeup(bool enable, uint8_t duration = 0, uint8_t thresh = 20);
  bool awake(void);
  bool shake(void);

  void enablePedometer(bool enable);
  void resetPedometer(void);
  uint16_t readPedometer(void);

  uint8_t chipID(void);
  bool connected(void) const;
  mannanauta_lsm6ds_variant_t variant(void) const;
  const char *variantName(void) const;

private:
  bool initDevice(mannanauta_lsm6ds_variant_t requestedVariant);
  bool readBytes(uint8_t reg, uint8_t *buffer, uint8_t len);
  uint8_t readRegister(uint8_t reg);
  bool writeRegister(uint8_t reg, uint8_t value);
  bool writeMaskedRegister(uint8_t reg, uint8_t mask, uint8_t value);
  bool readRawAccel(int16_t &x, int16_t &y, int16_t &z);
  bool readRawGyro(int16_t &x, int16_t &y, int16_t &z);
  int16_t readRawTemperature(void);
  uint8_t status(void);
  float accelRangeG(void) const;
  float gyroRangeDps(void) const;
  float dataRateHz(lsm6ds_data_rate_t dataRate) const;
  mannanauta_lsm6ds_variant_t detectVariant(uint8_t id) const;

  TwoWire *_wire;
  uint8_t _i2cAddr;
  uint8_t _chipID;
  bool _connected;
  mannanauta_lsm6ds_variant_t _variant;
  lsm6ds_accel_range_t _accelRange;
  lsm6ds_gyro_range_t _gyroRange;
  lsm6ds_data_rate_t _accelDataRate;
  lsm6ds_data_rate_t _gyroDataRate;
  float _temperatureSensitivity;
  bool _boardOrientationCorrection;
};

#endif
