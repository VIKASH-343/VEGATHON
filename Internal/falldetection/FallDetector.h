/* =====================================================================
 * FallDetector.h
 *
 * Bourke3 (velocity+impact+posture) fall detection, per Bagalà et al.
 * 2012, PLoS ONE 7(5):e37062 -- best-performing algorithm of the 13
 * benchmarked against real-world falls (SE~83%, SP~97%).
 * Secondary gate: Chen et al.'s orientation-change check.
 *
 * This logic lives in a .h/.cpp pair (NOT the .ino) deliberately: the
 * Arduino IDE auto-generates function prototypes for everything in the
 * .ino file and inserts them near the top of the translation unit,
 * before any typedefs that appear later in that same file. That breaks
 * as soon as a function signature uses a custom struct type. Regular
 * .h/.cpp files added to a sketch are compiled normally by the
 * underlying gcc/g++ toolchain and are NOT touched by that
 * auto-prototyping step, so the same struct-based code compiles fine
 * here.
 * =====================================================================*/
#ifndef FALL_DETECTOR_H
#define FALL_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

/* ---- thresholds from the paper's description of Bourke3 ----
 * NOTE: the BMI088 Arduino library (Bolder Flight / VEGA port) returns
 * acceleration in m/s^2, not g, so thresholds are defined directly in
 * m/s^2 here (G_MPS2 * g-value from the paper) -- no unit conversion
 * needed at the call site. */
#define G_MPS2                   9.80665f
#define LFT_MSS                  (0.65f * G_MPS2)   /* ~6.37 m/s^2 */
#define UFT_MSS                  (2.80f * G_MPS2)   /* ~27.46 m/s^2 */
#define FALLING_EDGE_MAX_MS      600u
#define RISING_EDGE_MAX_MS       350u
#define VERTICAL_VELOCITY_TH     0.70f   /* m/s, already SI */
#define POSTURE_CHECK_START_MS   1000u
#define POSTURE_CHECK_END_MS     3000u
#define POSTURE_ANGLE_TH_DEG     60.0f
#define POSTURE_DUTY_FRACTION    0.75f
#define CHEN_ORIENTATION_TH_DEG  20.0f
#define BASELINE_ALPHA           0.001f

typedef struct { float x, y, z; } vec3f_t;

typedef enum {
    STATE_IDLE = 0,
    STATE_TROUGH,
    STATE_IMPACT_SEEN,
    STATE_POSTURE_WAIT
} fall_state_t;

typedef struct {
    fall_state_t state;
    uint32_t  t_lft_cross_ms;
    uint32_t  t_uft_cross_ms;
    float     velocity_mps;
    uint32_t  t_last_sample_ms;
    uint32_t  posture_samples_total;
    uint32_t  posture_samples_lying;
    vec3f_t   orientation_pre;
    vec3f_t   orientation_post;
    bool      have_pre_orientation;
    float     resting_sv_baseline;  /* m/s^2, informational only */
} fall_detector_t;

#ifdef __cplusplus
extern "C" {
#endif

void  fall_detector_init(fall_detector_t *fd);

/* Returns true exactly on the sample where a fall is confirmed
 * (posture + orientation gates both passed). Call once per
 * accelerometer sample. */
bool  fall_detector_update(fall_detector_t *fd, const vec3f_t *accel_g,
                            uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* FALL_DETECTOR_H */
