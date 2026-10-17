#include "lattice_accelerator.hpp"

namespace traceleak {

// Zeta powers for NTT over Kyber ring Z_q[X]/(X^256 + 1)
static const int16_t zetas[128] = {
    -1044,  -758,  -359, -1037,   993,  1293,  -652,  -388,
     1306,   919,  -615,    89,   980,  -773,  -541,  1084,
    -1080,  -976, -1143,  -878,   678,   179,  -353,   826,
      864,  -833,   775,  -772,   456,  -806, -1101,  -988,
      165,   171,   708, -1004, -1008,  -769,   649,   764,
     -783,   -45,   -48,  -854,   961,   704,  -835,  -741,
     -447,   690,  -258,  -965,  -740,  -739,   956,   694,
      417,  -855,   823,   237,  -836,  -450,  -394,   153
};

static inline int16_t montgomery_reduce(int32_t a) {
    int32_t t = (int16_t)a * 62209;
    int16_t res = (a - t * KYBER_Q) >> 16;
    return res;
}

static inline int16_t barrett_reduce(int16_t a) {
    int32_t t = ((int32_t)a * 20159) >> 26;
    return a - t * KYBER_Q;
}

void LatticeAccelerator::ntt(std::array<int16_t, KYBER_N>& poly) {
    int k = 1;
    for (int len = 128; len >= 2; len >>= 1) {
        for (int start = 0; start < 256; start += 2 * len) {
            int16_t zeta = zetas[k++];
            for (int j = start; j < start + len; ++j) {
                int16_t t = montgomery_reduce((int32_t)zeta * poly[j + len]);
                poly[j + len] = poly[j] - t;
                poly[j] = poly[j] + t;
            }
        }
    }
}

void LatticeAccelerator::inv_ntt(std::array<int16_t, KYBER_N>& poly) {
    int k = 127;
    for (int len = 2; len <= 128; len <<= 1) {
        for (int start = 0; start < 256; start += 2 * len) {
            int16_t zeta = -zetas[k--];
            for (int j = start; j < start + len; ++j) {
                int16_t t = poly[j];
                poly[j] = barrett_reduce(t + poly[j + len]);
                poly[j + len] = montgomery_reduce((int32_t)zeta * (poly[j + len] - t));
            }
        }
    }
    // Multiply by 1/256 mod q
    for (int j = 0; j < 256; ++j) {
        poly[j] = montgomery_reduce((int32_t)poly[j] * 3303);
    }
}

void LatticeAccelerator::poly_mul_pointwise(
    const std::array<int16_t, KYBER_N>& a,
    const std::array<int16_t, KYBER_N>& b,
    std::array<int16_t, KYBER_N>& out
) {
    for (int i = 0; i < KYBER_N; ++i) {
        out[i] = montgomery_reduce((int32_t)a[i] * b[i]);
    }
}

} // namespace traceleak
