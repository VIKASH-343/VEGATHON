#include <Wire.h>

TwoWire Wire(8);

// Temperature calibration
uint16_t par_t1;
int16_t  par_t2;
int8_t   par_t3;
double   t_fine;

// Pressure calibration
uint16_t par_p1;
int16_t  par_p2, par_p4, par_p5, par_p8, par_p9;
int8_t   par_p3, par_p6, par_p7;
uint8_t  par_p10;

// Humidity calibration
uint16_t par_h1, par_h2;
int8_t   par_h3, par_h4, par_h5, par_h7;
uint8_t  par_h6;

// Gas calibration
int8_t  par_g1;
int16_t par_g2;
int8_t  par_g3;
uint8_t res_heat_range;
int8_t  res_heat_val;
int8_t  range_sw_err;

uint8_t readReg8(uint8_t reg) {
  Wire.beginTransmission(0x76);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(0x76, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

void writeReg8(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(0x76);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint16_t readReg16LE(uint8_t reg) {
  uint8_t lsb = readReg8(reg);
  uint8_t msb = readReg8(reg + 1);
  return ((uint16_t)msb << 8) | lsb;
}

void readCalibration() {
  // Temperature
  par_t1 = readReg16LE(0xE9);
  par_t2 = (int16_t)readReg16LE(0x8A);
  par_t3 = (int8_t)readReg8(0x8C);

  // Pressure
  par_p1 = readReg16LE(0x8E);
  par_p2 = (int16_t)readReg16LE(0x90);
  par_p3 = (int8_t)readReg8(0x92);
  par_p4 = (int16_t)readReg16LE(0x94);
  par_p5 = (int16_t)readReg16LE(0x96);
  par_p6 = (int8_t)readReg8(0x99);
  par_p7 = (int8_t)readReg8(0x98);
  par_p8 = (int16_t)readReg16LE(0x9C);
  par_p9 = (int16_t)readReg16LE(0x9E);
  par_p10 = readReg8(0xA0);

  // Humidity
  uint8_t h1_lsb = readReg8(0xE2);
  par_h1 = ((uint16_t)readReg8(0xE3) << 4) | (h1_lsb & 0x0F);
  par_h2 = ((uint16_t)readReg8(0xE1) << 4) | (h1_lsb >> 4);
  par_h3 = (int8_t)readReg8(0xE4);
  par_h4 = (int8_t)readReg8(0xE5);
  par_h5 = (int8_t)readReg8(0xE6);
  par_h6 = readReg8(0xE7);
  par_h7 = (int8_t)readReg8(0xE8);

  // Gas
  par_g1 = (int8_t)readReg8(0xED);
  par_g2 = (int16_t)readReg16LE(0xEB);
  par_g3 = (int8_t)readReg8(0xEE);
  res_heat_range = (readReg8(0x02) & 0x30) >> 4;
  res_heat_val = (int8_t)readReg8(0x00);
  range_sw_err = ((int8_t)readReg8(0x04) & 0xF0) >> 4;
}

double calcTemperature(int32_t temp_adc) {
  double var1 = (((double)temp_adc / 16384.0) - ((double)par_t1 / 1024.0)) * (double)par_t2;
  double var2 = ((((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0)) *
                 (((double)temp_adc / 131072.0) - ((double)par_t1 / 8192.0))) * (double)par_t3 * 16.0;
  t_fine = var1 + var2;
  return t_fine / 5120.0;
}

double calcPressure(int32_t pres_adc) {
  double var1, var2, var3, pressure;
  var1 = (t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * ((double)par_p6 / 131072.0);
  var2 = var2 + (var1 * (double)par_p5 * 2.0);
  var2 = (var2 / 4.0) + ((double)par_p4 * 65536.0);
  var1 = (((double)par_p3 * var1 * var1 / 16384.0) + ((double)par_p2 * var1)) / 524288.0;
  var1 = (1.0 + (var1 / 32768.0)) * (double)par_p1;
  pressure = 1048576.0 - (double)pres_adc;
  pressure = ((pressure - (var2 / 4096.0)) * 6250.0) / var1;
  var1 = ((double)par_p9 * pressure * pressure) / 2147483648.0;
  var2 = pressure * ((double)par_p8 / 32768.0);
  var3 = (pressure / 256.0) * (pressure / 256.0) * (pressure / 256.0) * ((double)par_p10 / 131072.0);
  pressure = pressure + (var1 + var2 + var3 + ((double)par_p7 * 128.0)) / 16.0;
  return pressure;
}

double calcHumidity(int32_t hum_adc) {
  double temp_comp = t_fine / 5120.0;
  double var1 = (double)hum_adc - (((double)par_h1 * 16.0) + (((double)par_h3 / 2.0) * temp_comp));
  double var2 = var1 * (((double)par_h2 / 262144.0) *
                (1.0 + (((double)par_h4 / 16384.0) * temp_comp) +
                (((double)par_h5 / 1048576.0) * temp_comp * temp_comp)));
  double var3 = (double)par_h6 / 16384.0;
  double var4 = (double)par_h7 / 2097152.0;
  double humidity = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);
  if (humidity > 100.0) humidity = 100.0;
  if (humidity < 0.0) humidity = 0.0;
  return humidity;
}

void setGasHeater(uint16_t target_temp, uint16_t ambient_temp) {
  double var1 = ((double)par_g1 / 16.0) + 49.0;
  double var2 = (((double)par_g2 / 32768.0) * 0.0005) + 0.00235;
  double var3 = (double)par_g3 / 1024.0;
  double var4 = var1 * (1.0 + (var2 * (double)target_temp));
  double var5 = var4 + (var3 * (double)ambient_temp);
  uint8_t res_heat = (uint8_t)(3.4 *
      ((var5 * (4.0 / (4.0 + (double)res_heat_range)) *
       (1.0 / (1.0 + ((double)res_heat_val * 0.002)))) - 25));
  writeReg8(0x5A, res_heat);
  writeReg8(0x64, 0x59);
}

double calcGasResistance(uint16_t gas_adc, uint8_t gas_range) {
  static const double const_array1[16] = {1,1,1,1,1,0.99,1,0.992,1,1,0.998,0.995,1,0.99,1,1};
  static const double const_array2[16] = {8000000,4000000,2000000,1000000,499500.4995,248262.1648,125000,63004.03226,
                                           31281.28128,15625,7812.5,3906.25,1953.125,976.5625,488.28125,244.140625};
  double var1 = (1340.0 + (5.0 * (double)range_sw_err)) * const_array1[gas_range];
  double gas_res = var1 * const_array2[gas_range] / (double)(gas_adc - 512 + var1);
  return gas_res;
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("BME680 All-in-One Reader");

  Wire.begin();
  readCalibration();
  Serial.println("Calibration data loaded!");

  setGasHeater(320, 25);
  writeReg8(0x71, 0x10);   // ctrl_gas_1: run_gas=1
  writeReg8(0x72, 0x01);   // ctrl_hum
  writeReg8(0x75, 0x00);   // config: filter off
}

void loop() {
  writeReg8(0x74, 0x25);   // trigger forced-mode measurement
  delay(300);

  uint8_t p_msb = readReg8(0x1F);
  uint8_t p_lsb = readReg8(0x20);
  uint8_t p_xlsb = readReg8(0x21);
  int32_t pres_adc = ((uint32_t)p_msb << 12) | ((uint32_t)p_lsb << 4) | (p_xlsb >> 4);

  uint8_t t_msb = readReg8(0x22);
  uint8_t t_lsb = readReg8(0x23);
  uint8_t t_xlsb = readReg8(0x24);
  int32_t temp_adc = ((uint32_t)t_msb << 12) | ((uint32_t)t_lsb << 4) | (t_xlsb >> 4);

  uint8_t h_msb = readReg8(0x25);
  uint8_t h_lsb = readReg8(0x26);
  int32_t hum_adc = ((uint32_t)h_msb << 8) | h_lsb;

  uint8_t gas_msb = readReg8(0x2A);
  uint8_t gas_lsb = readReg8(0x2B);
  uint16_t gas_adc = ((uint16_t)gas_msb << 2) | (gas_lsb >> 6);
  uint8_t gas_range = gas_lsb & 0x0F;
  bool gas_valid = (gas_lsb & 0x20) != 0;

  double temperature = calcTemperature(temp_adc);   // must run first (sets t_fine)
  double pressure = calcPressure(pres_adc);
  double humidity = calcHumidity(hum_adc);

  Serial.println("---------------------------");
  Serial.print("Temperature = "); Serial.print(temperature); Serial.println(" *C");
  Serial.print("Pressure = "); Serial.print(pressure / 100.0); Serial.println(" hPa");
  Serial.print("Humidity = "); Serial.print(humidity); Serial.println(" %");

  if (gas_valid) {
    double gasRes = calcGasResistance(gas_adc, gas_range);
    Serial.print("Gas Resistance = "); Serial.print(gasRes); Serial.println(" Ohms");
  } else {
    Serial.println("Gas reading not valid yet");
  }

  delay(2000);
}
// this is the one code for the gas,humidity,pressure,emperature
