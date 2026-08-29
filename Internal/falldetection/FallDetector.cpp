#include "FallDetector.h"
#include <math.h>
#include <string.h>

/* Sum vector magnitude, in whatever unit the input vector is in
 * (m/s^2, matching accel.getAccelX_mss() etc.). Orientation-independent
 * by construction, same as the paper's SV feature. */
static float sum_vector_mss(const vec3f_t *a)
{
    return sqrtf(a->x * a->x + a->y * a->y + a->z * a->z);
}

static float vec3_dot_normalized(const vec3f_t *a, const vec3f_t *b)
{
    float na = sqrtf(a->x*a->x + a->y*a->y + a->z*a->z);
    float nb = sqrtf(b->x*b->x + b->y*b->y + b->z*b->z);
    if (na < 1e-6f || nb < 1e-6f) return 1.0f;
    float dot = (a->x*b->x + a->y*b->y + a->z*b->z) / (na * nb);
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;
    return dot;
}

void fall_detector_init(fall_detector_t *fd)
{
    memset(fd, 0, sizeof(*fd));
    fd->state = STATE_IDLE;
    fd->resting_sv_baseline = G_MPS2; /* ~9.81 m/s^2 at rest */
}

bool fall_detector_update(fall_detector_t *fd, const vec3f_t *accel_g,
                           uint32_t now_ms)
{
    float sv = sum_vector_mss(accel_g);
    bool  fall_confirmed = false;

    fd->resting_sv_baseline += BASELINE_ALPHA * (sv - fd->resting_sv_baseline);

    switch (fd->state) {

    case STATE_IDLE:
        if (sv < LFT_MSS) {
            fd->state = STATE_TROUGH;
            fd->t_lft_cross_ms = now_ms;
            fd->velocity_mps = 0.0f;
            fd->t_last_sample_ms = now_ms;
            fd->orientation_pre = *accel_g;
            fd->have_pre_orientation = true;
        }
        break;

    case STATE_TROUGH: {
        /* Integrate (SV - 1g) to approximate vertical velocity during
         * the fall -- sv is already in m/s^2 so no extra scaling needed. */
        float dt = (now_ms - fd->t_last_sample_ms) / 1000.0f;
        fd->velocity_mps += (sv - G_MPS2) * dt;
        fd->t_last_sample_ms = now_ms;

        uint32_t since_trough = now_ms - fd->t_lft_cross_ms;

        if (sv > UFT_MSS) {
            if (since_trough <= FALLING_EDGE_MAX_MS) {
                fd->t_uft_cross_ms = now_ms;
                fd->state = STATE_IMPACT_SEEN;
            } else {
                fd->state = STATE_IDLE;
            }
        } else if (since_trough > FALLING_EDGE_MAX_MS) {
            fd->state = STATE_IDLE;
        }
        break;
    }

    case STATE_IMPACT_SEEN:
        if (fabsf(fd->velocity_mps) < VERTICAL_VELOCITY_TH) {
            fd->state = STATE_IDLE;
            break;
        }
        fd->posture_samples_total = 0;
        fd->posture_samples_lying = 0;
        fd->state = STATE_POSTURE_WAIT;
        break;

    case STATE_POSTURE_WAIT: {
        uint32_t since_impact = now_ms - fd->t_uft_cross_ms;

        if (since_impact < POSTURE_CHECK_START_MS) {
            break;
        }
        if (since_impact > POSTURE_CHECK_END_MS) {
            bool lying = false;
            if (fd->posture_samples_total > 0) {
                float duty = (float)fd->posture_samples_lying /
                             (float)fd->posture_samples_total;
                lying = (duty >= POSTURE_DUTY_FRACTION);
            }

            bool orientation_changed = true;
            if (fd->have_pre_orientation) {
                fd->orientation_post = *accel_g;
                float cos_angle = vec3_dot_normalized(&fd->orientation_pre,
                                                        &fd->orientation_post);
                float angle_deg = acosf(cos_angle) * 57.29578f;
                orientation_changed = (angle_deg > CHEN_ORIENTATION_TH_DEG);
            }

            if (lying && orientation_changed) {
                fall_confirmed = true;
            }
            fd->state = STATE_IDLE;
            fd->have_pre_orientation = false;
            break;
        }

        vec3f_t vertical_ref = { 0.0f, 0.0f, 1.0f };
        float cos_a = vec3_dot_normalized(accel_g, &vertical_ref);
        float angle_deg = acosf(cos_a) * 57.29578f;

        fd->posture_samples_total++;
        if (angle_deg > POSTURE_ANGLE_TH_DEG) {
            fd->posture_samples_lying++;
        }
        break;
    }
    }

    return fall_confirmed;
}
