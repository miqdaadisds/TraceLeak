#include "dct_simd.h"
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

void traceleak_secure_zeroize(void *ptr, size_t len) {
    if (!ptr || len == 0) return;

#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#elif defined(__STDC_LIB_EXT1__)
    memset_s(ptr, len, 0, len);
#else
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
#endif
}
