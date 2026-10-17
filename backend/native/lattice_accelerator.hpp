#ifndef TRACELEAK_LATTICE_ACCELERATOR_HPP
#define TRACELEAK_LATTICE_ACCELERATOR_HPP

#include <vector>
#include <cstdint>
#include <array>

namespace traceleak {

constexpr int KYBER_N = 256;
constexpr int KYBER_Q = 3329;

class LatticeAccelerator {
public:
    static void ntt(std::array<int16_t, KYBER_N>& poly);
    static void inv_ntt(std::array<int16_t, KYBER_N>& poly);
    static void poly_mul_pointwise(
        const std::array<int16_t, KYBER_N>& a,
        const std::array<int16_t, KYBER_N>& b,
        std::array<int16_t, KYBER_N>& out
    );
};

} // namespace traceleak

#endif // TRACELEAK_LATTICE_ACCELERATOR_HPP
