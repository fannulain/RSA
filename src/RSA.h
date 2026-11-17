#ifndef RSA_H
#define RSA_H

#include "BigNum.h"
#include <string>
#include <cstddef>

struct RSAKeyPair {
    BigNum p;   // first prime
    BigNum q;   // second prime 
    BigNum n;   // modulus n = p * q
    BigNum phi; // euler func phi(n) = (p - 1) * (q - 1)
    BigNum e;   // public exponent
    BigNum d;   // private exponent
};

bool isPrimeFermat(const BigNum& n, int rounds = 25);
bool isPrimeMillerRabin(const BigNum& n, int rounds = 25);

BigNum generateLargePrime(int digits, int rounds = 25, int* candidateCount = nullptr);

RSAKeyPair generateRSAKeys(int primeDigits, const BigNum& preferredE = BigNum(65537));

size_t getRSABlockSize(const BigNum& n);
BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n);
BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n);

void encryptFileRSA(const std::string& inputPath,
                    const std::string& outputPath,
                    const BigNum& e,
                    const BigNum& n,
                    size_t* outBlockCount = nullptr,
                    size_t* outFileSize = nullptr);

void decryptFileRSA(const std::string& inputPath,
                    const std::string& outputPath,
                    const BigNum& d,
                    const BigNum& n,
                    size_t* outBlockCount = nullptr,
                    size_t* outFileSize = nullptr);

bool verifyFilesMatch(const std::string& pathA,
                      const std::string& pathB,
                      size_t* outSizeA = nullptr,
                      size_t* outSizeB = nullptr);

#endif // RSA_H
