#ifndef RSA_H
#define RSA_H

#include "BigNum.h"

struct RSAKeyPair {
    BigNum p;   // first prime
    BigNum q;   // second prime 
    BigNum n;   // modulus n = p * q
    BigNum phi; // euler func phi(n) = (p - 1) * (q - 1)
    BigNum e;   // public exponent
    BigNum d;   // private exponent
};

bool isPrimeFermat(const BigNum& n, int rounds = 25);

BigNum generateLargePrime(int digits, int fermatRounds = 25, int* candidateCount = nullptr);

RSAKeyPair generateRSAKeys(int primeDigits, const BigNum& preferredE = BigNum(65537));

#endif // RSA_H
