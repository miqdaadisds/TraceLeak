/**
 * TraceLeak Native SIMD Acceleration Subsystem
 * Ministry of Defence / WESEE (SIH 2026 - PS 237)
 * High-speed 8x8 Block 2D-DCT Quantization Index Modulation (QIM)
 */
#ifndef TRACELEAK_DCT_SIMD_H
#define TRACELEAK_DCT_SIMD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DCT_BLOCK_SIZE 8
#define DCT_BLOCK_AREA 64

/**
 * Computes forward 2D-DCT on an 8x8 floating point spatial block.
 * Uses separable 1D-DCT row-column decomposition.
 */
void traceleak_dct_forward_8x8(const double in[64], double out[64]);

/**
 * Computes inverse 2D-DCT on an 8x8 frequency coefficient block.
 */
void traceleak_dct_inverse_8x8(const double in[64], double out[64]);

/**
 * Quantization Index Modulation (QIM) embedding into mid-frequency coefficients.
 * @param block 64-element DCT coefficient array
 * @param bit The binary watermark bit to embed (0 or 1)
 * @param delta Quantization step size (e.g. 16.0 for robustness vs visual fidelity)
 */
void traceleak_qim_embed_bit(double block[64], int bit, double delta);

/**
 * Extracts embedded watermark bit from mid-frequency coefficient.
 */
int traceleak_qim_extract_bit(const double block[64], double delta);

/**
 * Constant-time secure memory zeroization.
 * Prevents compiler optimization from stripping key erasure.
 */
void traceleak_secure_zeroize(void *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* TRACELEAK_DCT_SIMD_H */
