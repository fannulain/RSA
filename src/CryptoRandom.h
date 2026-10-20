#ifndef CRYPTO_RANDOM_H
#define CRYPTO_RANDOM_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <bcrypt.h>
#include <cstdint>
#include <stdexcept>
#include <string>

#pragma comment(lib, "bcrypt.lib")

class CryptoRandom {
public:
    static void getBytes(void* buffer, size_t length) {
        NTSTATUS status = BCryptGenRandom(
            NULL,
            static_cast<PUCHAR>(buffer),
            static_cast<ULONG>(length),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );
        if (status != 0) {
            throw std::runtime_error("Error: BCryptGenRandom failed with status " + std::to_string(status));
        }
    }

    static uint32_t getUInt32() {
        uint32_t val = 0;
        getBytes(&val, sizeof(val));
        return val;
    }

    static int getInt(int minVal, int maxVal) {
        if (minVal > maxVal) {
            throw std::invalid_argument("Error: minVal cannot be greater than maxVal.");
        }
        if (minVal == maxVal) {
            return minVal;
        }

        uint64_t range = static_cast<uint64_t>(maxVal) - static_cast<uint64_t>(minVal) + 1;
        uint64_t limit = 0x100000000ULL - (0x100000000ULL % range);

        uint32_t val = 0;
        do {
            val = getUInt32();
        } while (static_cast<uint64_t>(val) >= limit);

        return minVal + static_cast<int>(val % range);
    }

    static int getDigit(int minDigit = 0, int maxDigit = 9) {
        return getInt(minDigit, maxDigit);
    }
};

#endif // CRYPTO_RANDOM_H
