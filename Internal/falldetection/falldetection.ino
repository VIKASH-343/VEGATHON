/* =====================================================================
 * Fall_Detection.ino
 *
 * Board: ARIES IoT / Vega IoT v2 (THEJAS32, VEGA ET1031 RISC-V)
 * Sensor: on-board BMI088, SPI (Bolder Flight BMI088 library, VEGA port)
 *
 * Algorithm: Bourke3 (velocity + impact + posture) + Chen orientation
 * gate, per Bagalà et al. 2012, PLoS ONE 7(5):e37062 -- best-performing
 * algorithm of 13 benchmarked against REAL (not simulated) falls.
 * Logic lives in FallDetector.h/.cpp (see comment there for why).
 *
 * Required Library: VEGA_BMI088 (confirmed from your BMI088Test.ino)
 * =====================================================================*/

#include <SPI.h>
#include "BMI088.h"
#include "FallDetector.h"

SPIClass SPI(2);

/* accel object -- same wiring as your working BMI088Test.ino */
Bmi088Accel accel(SPI, 0xFF);
/* gyro object -- not used by the fall-detection logic itself (the
 * algorithms in the paper are accelerometer-only), kept here only if
 * you want gyro data for future extensions/logging. */
Bmi088Gyro gyro(SPI, 21);

#define SAMPLE_RATE_HZ    100u
#define SAMPLE_PERIOD_MS  (1000u / SAMPLE_RATE_HZ)
#define ALARM_PIN         13   // TODO: set to whatever GPIO drives your buzzer/LED

static fall_detector_t fd;
static uint32_t next_sample_ms = 0;

void setup()
{
  delay(2000);
  int status;

  Serial.begin(115200);
  while (!Serial) {}

  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);

  status = accel.begin();
  if (status < 0) {
    Serial.println("Accel Initialization Error");
    Serial.println(status);
    while (1) {}
  }

  /* IMPORTANT: use +/-6g, not +/-2g or +/-3g -- Bagalà et al. found that
   * lower ranges clip/saturate on real fall impacts, which would corrupt
   * the SV signal right when it matters most (UFT = 2.8g). */
  accel.setRange(Bmi088Accel::RANGE_6G);
  accel.setOdr(Bmi088Accel::ODR_100HZ_BW_19HZ);

  status = gyro.begin();
  if (status < 0) {
    Serial.println("Gyro Initialization Error");
    Serial.println(status);
    while (1) {}
  }

  fall_detector_init(&fd);
  next_sample_ms = millis();

  Serial.println("Fall detector ready.");
}

void loop()
{
#if defined(VEGA_ARIES_IOT)
  uint32_t now = millis();
  if (now >= next_sample_ms) {

    accel.readSensor();

    vec3f_t a;
    a.x = accel.getAccelX_mss();
    a.y = accel.getAccelY_mss();
    a.z = accel.getAccelZ_mss();

    if (fall_detector_update(&fd, &a, now)) {
      digitalWrite(ALARM_PIN, HIGH);
      Serial.println("FALL DETECTED");
      // TODO: latch/hold alarm, fire radio alert (LoRa/BLE/Wi-Fi), etc.
      // digitalWrite(ALARM_PIN, LOW) once acknowledged/reset externally.
    }

    /* Optional: print live SV for calibration/debugging.
    Serial.print(a.x); Serial.print("\t");
    Serial.print(a.y); Serial.print("\t");
    Serial.println(a.z);
    */

    next_sample_ms += SAMPLE_PERIOD_MS;
  }
#else
  Serial.println("Choose the correct board!");
#endif
}
