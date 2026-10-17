#include "dct_simd.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* 2D-DCT forward 8x8 implementation */
void traceleak_dct_forward_8x8(const double in[64], double out[64]) {
    double temp[64];
    const double c0 = 1.0 / sqrt(2.0);

    /* 1D-DCT on rows */
    for (int i = 0; i < 8; ++i) {
        for (int u = 0; u < 8; ++u) {
            double sum = 0.0;
            double alpha = (u == 0) ? c0 : 1.0;
            for (int x = 0; x < 8; ++x) {
                sum += in[i * 8 + x] * cos((2.0 * x + 1.0) * u * M_PI / 16.0);
            }
            temp[i * 8 + u] = 0.5 * alpha * sum;
        }
    }

    /* 1D-DCT on columns */
    for (int j = 0; j < 8; ++j) {
        for (int v = 0; v < 8; ++v) {
            double sum = 0.0;
            double alpha = (v == 0) ? c0 : 1.0;
            for (int y = 0; y < 8; ++y) {
                sum += temp[y * 8 + j] * cos((2.0 * y + 1.0) * v * M_PI / 16.0);
            }
            out[v * 8 + j] = 0.5 * alpha * sum;
        }
    }
}

/* 2D-DCT inverse 8x8 implementation */
void traceleak_dct_inverse_8x8(const double in[64], double out[64]) {
    double temp[64];
    const double c0 = 1.0 / sqrt(2.0);

    for (int i = 0; i < 8; ++i) {
        for (int x = 0; x < 8; ++x) {
            double sum = 0.0;
            for (int u = 0; u < 8; ++u) {
                double alpha = (u == 0) ? c0 : 1.0;
                sum += alpha * in[i * 8 + u] * cos((2.0 * x + 1.0) * u * M_PI / 16.0);
            }
            temp[i * 8 + x] = 0.5 * sum;
        }
    }

    for (int j = 0; j < 8; ++j) {
        for (int y = 0; y < 8; ++y) {
            double sum = 0.0;
            for (int v = 0; v < 8; ++v) {
                double alpha = (v == 0) ? c0 : 1.0;
                sum += alpha * temp[v * 8 + j] * cos((2.0 * y + 1.0) * v * M_PI / 16.0);
            }
            out[y * 8 + j] = 0.5 * sum;
        }
    }
}

/* QIM Embedding in Mid-Frequency Coefficient (4, 3) -> index 35 */
void traceleak_qim_embed_bit(double block[64], int bit, double delta) {
    int target_idx = 35; // Robust mid-frequency AC coefficient
    double val = block[target_idx];
    double q0 = round(val / delta) * delta;
    double q1 = (round((val - delta / 2.0) / delta) * delta) + delta / 2.0;

    block[target_idx] = (bit == 0) ? q0 : q1;
}

/* QIM Extraction from Mid-Frequency Coefficient */
int traceleak_qim_extract_bit(const double block[64], double delta) {
    int target_idx = 35;
    double val = block[target_idx];
    double q0 = round(val / delta) * delta;
    double q1 = (round((val - delta / 2.0) / delta) * delta) + delta / 2.0;

    return (fabs(val - q0) < fabs(val - q1)) ? 0 : 1;
}
