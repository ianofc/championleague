#include "MannaNAUTA_LSM6DS.h"

#include <Wire.h>

static const uint8_t LSM6DS_REG_FUNC_CFG_ACCESS = 0x01;
static const uint8_t LSM6DS_REG_INT1_CTRL = 0x0D;
static const uint8_t LSM6DS_REG_INT2_CTRL = 0x0E;
static const uint8_t LSM6DS_REG_WHOAMI = 0x0F;
static const uint8_t LSM6DS_REG_CTRL1_XL = 0x10;
static const uint8_t LSM6DS_REG_CTRL2_G = 0x11;
static const uint8_t LSM6DS_REG_CTRL3_C = 0x12;
static const uint8_t LSM6DS_REG_CTRL8_XL = 0x17;
static const uint8_t LSM6DS_REG_CTRL10_C = 0x19;
static const uint8_t LSM6DS_REG_WAKEUP_SRC = 0x1B;
static const uint8_t LSM6DS_REG_STATUS = 0x1E;
static const uint8_t LSM6DS_REG_OUT_TEMP_L = 0x20;
static const uint8_t LSM6DS_REG_OUTX_L_G = 0x22;
static const uint8_t LSM6DS_REG_OUTX_L_A = 0x28;
static const uint8_t LSM6DS_REG_STEP_COUNTER_L = 0x4B;
static const uint8_t LSM6DS_REG_TAP_CFG = 0x58;
static const uint8_t LSM6DS_REG_WAKEUP_THS = 0x5B;
static const uint8_t LSM6DS_REG_WAKEUP_DUR = 0x5C;
static const uint8_t LSM6DS_REG_MD1_CFG = 0x5E;

static const uint8_t LSM6DS_CHIP_ID_69 = 0x69;
static const uint8_t LSM6DS_CHIP_ID_6A = 0x6A;
static const uint8_t LSM6DS_CHIP_ID_6B = 0x6B;
static const uint8_t LSM6DS_CHIP_ID_6C = 0x6C;

MannaNAUTA_LSM6DS::MannaNAUTA_LSM6DS()
    : _wire(nullptr), _i2cAddr(MANNANAUTA_LSM6DS_I2C_ADDR_DEFAULT),
      _chipID(0), _connected(false), _variant(MANNANAUTA_LSM6DS_AUTO),
      _accelRange(LSM6DS_ACCEL_RANGE_4_G),
      _gyroRange(LSM6DS_GYRO_RANGE_2000_DPS),
      _accelDataRate(LSM6DS_RATE_104_HZ),
      _gyroDataRate(LSM6DS_RATE_104_HZ), _temperatureSensitivity(256.0f),
      _boardOrientationCorrection(true) {}

bool MannaNAUTA_LSM6DS::begin() {
  return begin(MANNANAUTA_LSM6DS_DEFAULT_SDA, MANNANAUTA_LSM6DS_DEFAULT_SCL);
}

bool MannaNAUTA_LSM6DS::begin(int sdaPin, int sclPin) {
  return begin(sdaPin, sclPin, MANNANAUTA_LSM6DS_I2C_ADDR_DEFAULT, &Wire,
               MANNANAUTA_LSM6DS_AUTO);
}

bool MannaNAUTA_LSM6DS::begin(int sdaPin, int sclPin, uint8_t i2cAddr,
                              TwoWire *wire,
                              mannanauta_lsm6ds_variant_t variant) {
  _wire = wire ? wire : &Wire;
  _i2cAddr = i2cAddr;

#if defined(ARDUINO_ARCH_ESP32)
  _wire->begin(sdaPin, sclPin);
#else
  (void)sdaPin;
  (void)sclPin;
  _wire->begin();
#endif

  return initDevice(variant);
}

bool MannaNAUTA_LSM6DS::beginWithWire(uint8_t i2cAddr, TwoWire *wire,
                                      mannanauta_lsm6ds_variant_t variant) {
  _wire = wire ? wire : &Wire;
  _i2cAddr = i2cAddr;
  return initDevice(variant);
}

bool MannaNAUTA_LSM6DS::initDevice(mannanauta_lsm6ds_variant_t requestedVariant) {
  _connected = false;
  _variant = MANNANAUTA_LSM6DS_AUTO;
  _chipID = chipID();

  if (_chipID != LSM6DS_CHIP_ID_69 && _chipID != LSM6DS_CHIP_ID_6A &&
      _chipID != LSM6DS_CHIP_ID_6B && _chipID != LSM6DS_CHIP_ID_6C) {
    return false;
  }

  mannanauta_lsm6ds_variant_t detected = detectVariant(_chipID);
  if (requestedVariant != MANNANAUTA_LSM6DS_AUTO &&
      requestedVariant != detected) {
    if (!((requestedVariant == MANNANAUTA_LSM6DSOX ||
           requestedVariant == MANNANAUTA_LSM6DSO32) &&
          detected == MANNANAUTA_LSM6DSOX) &&
        !((requestedVariant == MANNANAUTA_LSM6DS3 ||
           requestedVariant == MANNANAUTA_LSM6DS33) &&
          detected == MANNANAUTA_LSM6DS3) &&
        !((requestedVariant == MANNANAUTA_LSM6DS3TRC ||
           requestedVariant == MANNANAUTA_LSM6DSL) &&
          detected == MANNANAUTA_LSM6DSL)) {
      return false;
    }
    _variant = requestedVariant;
  } else {
    _variant = detected;
  }

  _temperatureSensitivity = (_chipID == LSM6DS_CHIP_ID_69) ? 16.0f : 256.0f;

  reset();
  delay(10);

  writeRegister(LSM6DS_REG_CTRL3_C, 0x44);
  setAccelDataRate(_accelDataRate);
  setAccelRange(_accelRange);
  setGyroDataRate(_gyroDataRate);
  setGyroRange(_gyroRange);

  _connected = true;
  return true;
}

bool MannaNAUTA_LSM6DS::getEvent(mannanauta_lsm6ds_vector_t *accel,
                                 mannanauta_lsm6ds_vector_t *gyro,
                                 mannanauta_lsm6ds_temperature_t *temp) {
  if (!_connected) {
    return false;
  }

  uint32_t now = millis();

  if (accel) {
    float x, y, z;
    if (!readAcceleration(x, y, z)) {
      return false;
    }
    accel->x = x * 9.80665f;
    accel->y = y * 9.80665f;
    accel->z = z * 9.80665f;
    accel->timestamp = now;
  }

  if (gyro) {
    float x, y, z;
    if (!readGyroscope(x, y, z)) {
      return false;
    }
    gyro->x = x * DEG_TO_RAD;
    gyro->y = y * DEG_TO_RAD;
    gyro->z = z * DEG_TO_RAD;
    gyro->timestamp = now;
  }

  if (temp) {
    temp->temperature = readTemperature();
    temp->timestamp = now;
  }

  return true;
}

int MannaNAUTA_LSM6DS::readAcceleration(float &x, float &y, float &z) {
  int16_t rawX, rawY, rawZ;
  if (!readRawAccel(rawX, rawY, rawZ)) {
    x = y = z = NAN;
    return 0;
  }

  float scale = accelRangeG() / 32768.0f;
  float sensorX = rawX * scale;
  float sensorY = rawY * scale;
  float sensorZ = rawZ * scale;

  if (_boardOrientationCorrection) {
    x = -sensorY;
    y = -sensorX;
    z = sensorZ;
  } else {
    x = sensorX;
    y = sensorY;
    z = sensorZ;
  }

  return 1;
}

float MannaNAUTA_LSM6DS::accelerationSampleRate(void) {
  return dataRateHz(_accelDataRate);
}

int MannaNAUTA_LSM6DS::accelerationAvailable(void) {
  return (status() & 0x01) ? 1 : 0;
}

int MannaNAUTA_LSM6DS::readGyroscope(float &x, float &y, float &z) {
  int16_t rawX, rawY, rawZ;
  if (!readRawGyro(rawX, rawY, rawZ)) {
    x = y = z = NAN;
    return 0;
  }

  float scale = gyroRangeDps() / 32768.0f;
  float sensorX = rawX * scale;
  float sensorY = rawY * scale;
  float sensorZ = rawZ * scale;

  if (_boardOrientationCorrection) {
    x = -sensorY;
    y = -sensorX;
    z = sensorZ;
  } else {
    x = sensorX;
    y = sensorY;
    z = sensorZ;
  }

  return 1;
}

float MannaNAUTA_LSM6DS::gyroscopeSampleRate(void) {
  return dataRateHz(_gyroDataRate);
}

int MannaNAUTA_LSM6DS::gyroscopeAvailable(void) {
  return (status() & 0x02) ? 1 : 0;
}

float MannaNAUTA_LSM6DS::readTemperature(void) {
  int16_t raw = readRawTemperature();
  return 25.0f + ((float)raw / _temperatureSensitivity);
}

bool MannaNAUTA_LSM6DS::temperatureAvailable(void) {
  return (status() & 0x04) != 0;
}

void MannaNAUTA_LSM6DS::setBoardOrientationCorrection(bool enabled) {
  _boardOrientationCorrection = enabled;
}

bool MannaNAUTA_LSM6DS::boardOrientationCorrectionEnabled(void) const {
  return _boardOrientationCorrection;
}

lsm6ds_data_rate_t MannaNAUTA_LSM6DS::getAccelDataRate(void) {
  return _accelDataRate;
}

void MannaNAUTA_LSM6DS::setAccelDataRate(lsm6ds_data_rate_t dataRate) {
  _accelDataRate = dataRate;
  writeMaskedRegister(LSM6DS_REG_CTRL1_XL, 0xF0, ((uint8_t)dataRate) << 4);
}

lsm6ds_accel_range_t MannaNAUTA_LSM6DS::getAccelRange(void) {
  return _accelRange;
}

void MannaNAUTA_LSM6DS::setAccelRange(lsm6ds_accel_range_t range) {
  _accelRange = range;
  writeMaskedRegister(LSM6DS_REG_CTRL1_XL, 0x0C, ((uint8_t)range) << 2);
}

lsm6ds_data_rate_t MannaNAUTA_LSM6DS::getGyroDataRate(void) {
  return _gyroDataRate;
}

void MannaNAUTA_LSM6DS::setGyroDataRate(lsm6ds_data_rate_t dataRate) {
  _gyroDataRate = dataRate;
  writeMaskedRegister(LSM6DS_REG_CTRL2_G, 0xF0, ((uint8_t)dataRate) << 4);
}

lsm6ds_gyro_range_t MannaNAUTA_LSM6DS::getGyroRange(void) {
  return _gyroRange;
}

void MannaNAUTA_LSM6DS::setGyroRange(lsm6ds_gyro_range_t range) {
  _gyroRange = range;
  writeMaskedRegister(LSM6DS_REG_CTRL2_G, 0x0F, (uint8_t)range);
}

void MannaNAUTA_LSM6DS::reset(void) {
  writeMaskedRegister(LSM6DS_REG_CTRL3_C, 0x01, 0x01);
  delay(2);
}

void MannaNAUTA_LSM6DS::configIntOutputs(bool activeLow, bool openDrain) {
  uint8_t value = 0;
  if (activeLow) {
    value |= 0x20;
  }
  if (openDrain) {
    value |= 0x10;
  }
  writeMaskedRegister(LSM6DS_REG_CTRL3_C, 0x30, value);
}

void MannaNAUTA_LSM6DS::configInt1(bool drdyTemp, bool drdyGyro,
                                   bool drdyAccel, bool stepDetect,
                                   bool wakeup) {
  uint8_t value = 0;
  if (drdyAccel) {
    value |= 0x01;
  }
  if (drdyGyro) {
    value |= 0x02;
  }
  if (drdyTemp) {
    value |= 0x04;
  }
  if (stepDetect) {
    value |= 0x80;
  }
  writeRegister(LSM6DS_REG_INT1_CTRL, value);

  if (wakeup) {
    writeMaskedRegister(LSM6DS_REG_MD1_CFG, 0x20, 0x20);
  }
}

void MannaNAUTA_LSM6DS::configInt2(bool drdyTemp, bool drdyGyro,
                                   bool drdyAccel) {
  uint8_t value = 0;
  if (drdyAccel) {
    value |= 0x01;
  }
  if (drdyGyro) {
    value |= 0x02;
  }
  if (drdyTemp) {
    value |= 0x04;
  }
  writeRegister(LSM6DS_REG_INT2_CTRL, value);
}

void MannaNAUTA_LSM6DS::highPassFilter(bool enabled,
                                       lsm6ds_hp_filter_t filter) {
  uint8_t value = enabled ? (0x80 | ((uint8_t)filter & 0x03)) : 0;
  writeMaskedRegister(LSM6DS_REG_CTRL8_XL, 0x83, value);
}

void MannaNAUTA_LSM6DS::enableWakeup(bool enable, uint8_t duration,
                                     uint8_t thresh) {
  if (enable) {
    writeRegister(LSM6DS_REG_WAKEUP_THS, thresh & 0x3F);
    writeRegister(LSM6DS_REG_WAKEUP_DUR, duration & 0x0F);
    writeMaskedRegister(LSM6DS_REG_TAP_CFG, 0x80, 0x80);
    writeMaskedRegister(LSM6DS_REG_MD1_CFG, 0x20, 0x20);
  } else {
    writeMaskedRegister(LSM6DS_REG_MD1_CFG, 0x20, 0x00);
  }
}

bool MannaNAUTA_LSM6DS::awake(void) {
  return (readRegister(LSM6DS_REG_WAKEUP_SRC) & 0x08) != 0;
}

bool MannaNAUTA_LSM6DS::shake(void) {
  return (readRegister(LSM6DS_REG_WAKEUP_SRC) & 0x08) != 0;
}

void MannaNAUTA_LSM6DS::enablePedometer(bool enable) {
  if (enable) {
    writeMaskedRegister(LSM6DS_REG_CTRL10_C, 0x3C, 0x3C);
    writeMaskedRegister(LSM6DS_REG_TAP_CFG, 0x40, 0x40);
  } else {
    writeMaskedRegister(LSM6DS_REG_CTRL10_C, 0x3C, 0x00);
  }
}

void MannaNAUTA_LSM6DS::resetPedometer(void) {
  writeMaskedRegister(LSM6DS_REG_CTRL10_C, 0x02, 0x02);
  delay(1);
  writeMaskedRegister(LSM6DS_REG_CTRL10_C, 0x02, 0x00);
}

uint16_t MannaNAUTA_LSM6DS::readPedometer(void) {
  uint8_t buffer[2];
  if (!readBytes(LSM6DS_REG_STEP_COUNTER_L, buffer, 2)) {
    return 0;
  }
  return (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
}

uint8_t MannaNAUTA_LSM6DS::chipID(void) {
  return readRegister(LSM6DS_REG_WHOAMI);
}

bool MannaNAUTA_LSM6DS::connected(void) const { return _connected; }

mannanauta_lsm6ds_variant_t MannaNAUTA_LSM6DS::variant(void) const {
  return _variant;
}

const char *MannaNAUTA_LSM6DS::variantName(void) const {
  switch (_variant) {
  case MANNANAUTA_LSM6DSOX:
    return "LSM6DSOX or LSM6DSO32";
  case MANNANAUTA_LSM6DSO32:
    return "LSM6DSO32";
  case MANNANAUTA_LSM6DS3:
    return "LSM6DS3 or LSM6DS33";
  case MANNANAUTA_LSM6DS33:
    return "LSM6DS33";
  case MANNANAUTA_LSM6DS3TRC:
    return "LSM6DS3TR-C";
  case MANNANAUTA_LSM6DSL:
    return "LSM6DSL or LSM6DS3TR-C";
  case MANNANAUTA_ISM330DHCX:
    return "ISM330DHCX";
  case MANNANAUTA_LSM6DS_AUTO:
  default:
    return "not detected";
  }
}

bool MannaNAUTA_LSM6DS::readBytes(uint8_t reg, uint8_t *buffer, uint8_t len) {
  if (!_wire) {
    return false;
  }

  _wire->beginTransmission(_i2cAddr);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) {
    return false;
  }

  uint8_t received = _wire->requestFrom((int)_i2cAddr, (int)len);
  if (received != len) {
    return false;
  }

  for (uint8_t i = 0; i < len; i++) {
    buffer[i] = _wire->read();
  }

  return true;
}

uint8_t MannaNAUTA_LSM6DS::readRegister(uint8_t reg) {
  uint8_t value = 0;
  readBytes(reg, &value, 1);
  return value;
}

bool MannaNAUTA_LSM6DS::writeRegister(uint8_t reg, uint8_t value) {
  if (!_wire) {
    return false;
  }

  _wire->beginTransmission(_i2cAddr);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}

bool MannaNAUTA_LSM6DS::writeMaskedRegister(uint8_t reg, uint8_t mask,
                                            uint8_t value) {
  uint8_t current = readRegister(reg);
  current &= ~mask;
  current |= value & mask;
  return writeRegister(reg, current);
}

bool MannaNAUTA_LSM6DS::readRawAccel(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t buffer[6];
  if (!readBytes(LSM6DS_REG_OUTX_L_A, buffer, 6)) {
    return false;
  }

  x = (int16_t)((uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8));
  y = (int16_t)((uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8));
  z = (int16_t)((uint16_t)buffer[4] | ((uint16_t)buffer[5] << 8));
  return true;
}

bool MannaNAUTA_LSM6DS::readRawGyro(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t buffer[6];
  if (!readBytes(LSM6DS_REG_OUTX_L_G, buffer, 6)) {
    return false;
  }

  x = (int16_t)((uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8));
  y = (int16_t)((uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8));
  z = (int16_t)((uint16_t)buffer[4] | ((uint16_t)buffer[5] << 8));
  return true;
}

int16_t MannaNAUTA_LSM6DS::readRawTemperature(void) {
  uint8_t buffer[2];
  if (!readBytes(LSM6DS_REG_OUT_TEMP_L, buffer, 2)) {
    return 0;
  }
  return (int16_t)((uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8));
}

uint8_t MannaNAUTA_LSM6DS::status(void) {
  return readRegister(LSM6DS_REG_STATUS);
}

float MannaNAUTA_LSM6DS::accelRangeG(void) const {
  switch (_accelRange) {
  case LSM6DS_ACCEL_RANGE_16_G:
    return 16.0f;
  case LSM6DS_ACCEL_RANGE_4_G:
    return 4.0f;
  case LSM6DS_ACCEL_RANGE_8_G:
    return 8.0f;
  case LSM6DS_ACCEL_RANGE_2_G:
  default:
    return 2.0f;
  }
}

float MannaNAUTA_LSM6DS::gyroRangeDps(void) const {
  switch (_gyroRange) {
  case LSM6DS_GYRO_RANGE_125_DPS:
    return 125.0f;
  case LSM6DS_GYRO_RANGE_500_DPS:
    return 500.0f;
  case LSM6DS_GYRO_RANGE_1000_DPS:
    return 1000.0f;
  case LSM6DS_GYRO_RANGE_2000_DPS:
    return 2000.0f;
  case ISM330DHCX_GYRO_RANGE_4000_DPS:
    return 4000.0f;
  case LSM6DS_GYRO_RANGE_250_DPS:
  default:
    return 250.0f;
  }
}

float MannaNAUTA_LSM6DS::dataRateHz(lsm6ds_data_rate_t dataRate) const {
  switch (dataRate) {
  case LSM6DS_RATE_12_5_HZ:
    return 12.5f;
  case LSM6DS_RATE_26_HZ:
    return 26.0f;
  case LSM6DS_RATE_52_HZ:
    return 52.0f;
  case LSM6DS_RATE_104_HZ:
    return 104.0f;
  case LSM6DS_RATE_208_HZ:
    return 208.0f;
  case LSM6DS_RATE_416_HZ:
    return 416.0f;
  case LSM6DS_RATE_833_HZ:
    return 833.0f;
  case LSM6DS_RATE_1_66K_HZ:
    return 1660.0f;
  case LSM6DS_RATE_3_33K_HZ:
    return 3330.0f;
  case LSM6DS_RATE_6_66K_HZ:
    return 6660.0f;
  case LSM6DS_RATE_SHUTDOWN:
  default:
    return 0.0f;
  }
}

mannanauta_lsm6ds_variant_t
MannaNAUTA_LSM6DS::detectVariant(uint8_t id) const {
  switch (id) {
  case LSM6DS_CHIP_ID_69:
    return MANNANAUTA_LSM6DS3;
  case LSM6DS_CHIP_ID_6A:
    return MANNANAUTA_LSM6DSL;
  case LSM6DS_CHIP_ID_6B:
    return MANNANAUTA_ISM330DHCX;
  case LSM6DS_CHIP_ID_6C:
    return MANNANAUTA_LSM6DSOX;
  default:
    return MANNANAUTA_LSM6DS_AUTO;
  }
}
