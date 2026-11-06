/*
 * SPDX-FileCopyrightText: Donovan Worrell
 * SPDX-License-Identifier: Apache-2.0
 *
 * Runs the display card GPU core on a Cortex-M33 (QEMU mps2-an505) against
 * the independently computed vectors from gen_gpu_vectors.py, and prints one
 * line per case over semihosting. Exit status 0 only if every case passes.
 * Coordinates must be within +/-2 LSB (16.16); the measured difference is
 * printed so a pass is never silent about how close it was.
 */

#include <stdint.h>
#include <stdio.h>

#include "gpu_core.h"
#include "gpu_vectors.h"

static int32_t rd32(const uint8_t *p) {
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static int check(const char *name, const uint8_t *request, uint16_t length, const uint8_t *expected) {
    uint8_t reply[ATARIX_GPU_REPLY_LENGTH];
    int32_t worst = 0;
    int ok = 1;
    int i;
    const atarix_gpu_status_t status = atarix_gpu_transform(request, length, reply);

    if ((uint32_t)rd32(reply) != (uint32_t)status || (uint32_t)status != (uint32_t)rd32(expected)) {
        ok = 0;
    }
    for (i = 0; i < 3; ++i) {
        int32_t diff = rd32(reply + 4 + 4 * i) - rd32(expected + 4 + 4 * i);
        if (diff < 0) diff = -diff;
        if (diff > worst) worst = diff;
        if (diff > 2) ok = 0;
    }
    printf("%s %s status=%ld x=%ld y=%ld z=%ld max_diff_lsb=%ld\n",
           ok ? "PASS" : "FAIL", name, (long)status,
           (long)rd32(reply + 4), (long)rd32(reply + 8), (long)rd32(reply + 12), (long)worst);
    return ok;
}

int main(void) {
    int passed = 0;
    int total = 0;

    total++; passed += check("g1_rotate90_translate", gpu_req_g1, sizeof gpu_req_g1, gpu_exp_g1);
#ifdef NEGATIVE_CONTROL
    {
        /* Expect x 3 LSB away from the true value: the check must fail. */
        static uint8_t wrong[ATARIX_GPU_REPLY_LENGTH];
        unsigned i;
        for (i = 0; i < sizeof wrong; ++i) wrong[i] = gpu_exp_g2[i];
        wrong[4] = (uint8_t)(wrong[4] + 3u);
        total++; passed += check("g2_general_expected_off_by_3", gpu_req_g2, sizeof gpu_req_g2, wrong);
    }
#else
    total++; passed += check("g2_general", gpu_req_g2, sizeof gpu_req_g2, gpu_exp_g2);
#endif
    total++; passed += check("bad_operation", gpu_req_badop, sizeof gpu_req_badop, gpu_exp_badop);
    total++; passed += check("bad_length", gpu_req_g1, 40, gpu_exp_badlen);
    total++; passed += check("zero_axis", gpu_req_zeroaxis, sizeof gpu_req_zeroaxis, gpu_exp_zeroaxis);
    total++; passed += check("out_of_range", gpu_req_range, sizeof gpu_req_range, gpu_exp_range);

    printf("M33 GPU CORE: %d/%d %s\n", passed, total, passed == total ? "PASS" : "FAIL");
    return passed == total ? 0 : 1;
}
