#include "RSA.h"

bool isPrimeFermat(const BigNum& n, int rounds) {
    if (n <= BigNum(1)) return false;
    if (n == BigNum(2) || n == BigNum(3)) return true;
    if (n.isEven()) return false;

    static const int smallPrimes[] = {
        3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
        151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227,
        229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307
    };
    for (int p : smallPrimes) {
        BigNum bp(p);
        if (n == bp) return true;
        if ((n % bp).isZero()) return false;
    }

    std::random_device rd;
    std::mt19937 rng(rd());

    BigNum nMinus1 = n - BigNum(1);
    BigNum nMinus2 = n - BigNum(2);

    for (int r = 0; r < rounds; ++r) {
        BigNum a = BigNum::generateRandomRange(BigNum(2), nMinus2, rng);

        if (BigNum::gcd(a, n) > BigNum(1)) {
            return false;
        }

        BigNum res = BigNum::modPow(a, nMinus1, n);
        if (!res.isOne()) {
            return false;
        }
    }

    return true;
}

BigNum generateLargePrime(int digits, int fermatRounds, int* candidateCount) {
    std::random_device rd;
    std::mt19937 rng(rd());

    int count = 0;
    while (true) {
        BigNum candidate = BigNum::generateRandomOdd(digits, rng);
        count++;
        if (isPrimeFermat(candidate, fermatRounds)) {
            if (candidateCount) *candidateCount = count;
            return candidate;
        }

        for (int step = 0; step < 50; ++step) {
            candidate = candidate + BigNum(2);
            count++;
            if (isPrimeFermat(candidate, fermatRounds)) {
                if (candidateCount) *candidateCount = count;
                return candidate;
            }
        }
    }
}

RSAKeyPair generateRSAKeys(int primeDigits, const BigNum& preferredE) {
    RSAKeyPair keys;

    keys.p = generateLargePrime(primeDigits);
    do {
        keys.q = generateLargePrime(primeDigits);
    } while (keys.p == keys.q);

    keys.n = keys.p * keys.q;

    BigNum pMinus1 = keys.p - BigNum(1);
    BigNum qMinus1 = keys.q - BigNum(1);
    keys.phi = pMinus1 * qMinus1;

    if (preferredE < keys.phi && BigNum::gcd(preferredE, keys.phi).isOne()) {
        keys.e = preferredE;
    } else {
        BigNum curE(3);
        while (true) {
            if (BigNum::gcd(curE, keys.phi).isOne()) {
                keys.e = curE;
                break;
            }
            curE = curE + BigNum(2);
        }
    }

    keys.d = BigNum::modInverse(keys.e, keys.phi);

    return keys;
}