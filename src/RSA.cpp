#include "RSA.h"
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

bool checkSmallPrimes(const BigNum& n, bool& definiteResult) {
    if (n <= BigNum(1)) {
        definiteResult = false;
        return true;
    }
    if (n == BigNum(2) || n == BigNum(3)) {
        definiteResult = true;
        return true;
    }
    if (n.isEven()) {
        definiteResult = false;
        return true;
    }

    static const int smallPrimes[] = {
        3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
        151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227,
        229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307
    };
    for (int p : smallPrimes) {
        BigNum bp(p);
        if (n == bp) {
            definiteResult = true;
            return true;
        }
        if ((n % bp).isZero()) {
            definiteResult = false;
            return true;
        }
    }
    return false;
}

} // namespace

bool isPrimeFermat(const BigNum& n, int rounds) {
    bool definite = false;
    if (checkSmallPrimes(n, definite)) {
        return definite;
    }

    BigNum nMinus1 = n - BigNum(1);
    BigNum nMinus2 = n - BigNum(2);

    for (int r = 0; r < rounds; ++r) {
        BigNum a = BigNum::generateRandomRange(BigNum(2), nMinus2);

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

bool isPrimeMillerRabin(const BigNum& n, int rounds) {
    bool definite = false;
    if (checkSmallPrimes(n, definite)) {
        return definite;
    }

    // Write n - 1 = d * 2^s with d odd
    BigNum nMinus1 = n - BigNum(1);
    BigNum nMinus2 = n - BigNum(2);
    BigNum d = nMinus1;
    int s = 0;
    while (d.isEven()) {
        d = d.div2();
        s++;
    }

    for (int r = 0; r < rounds; ++r) {
        BigNum a = BigNum::generateRandomRange(BigNum(2), nMinus2);
        BigNum x = BigNum::modPow(a, d, n);

        if (x.isOne() || x == nMinus1) {
            continue;
        }

        bool isWitness = true;
        for (int i = 1; i < s; ++i) {
            x = x.sqr() % n;
            if (x == nMinus1) {
                isWitness = false;
                break;
            }
            if (x.isOne()) {
                return false;
            }
        }

        if (isWitness) {
            return false;
        }
    }

    return true;
}

BigNum generateLargePrime(int digits, int rounds, int* candidateCount) {
    int count = 0;
    while (true) {
        BigNum candidate = BigNum::generateRandomOdd(digits);
        count++;
        if (isPrimeMillerRabin(candidate, rounds)) {
            if (candidateCount) *candidateCount = count;
            return candidate;
        }

        for (int step = 0; step < 50; ++step) {
            candidate = candidate + BigNum(2);
            count++;
            if (isPrimeMillerRabin(candidate, rounds)) {
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

size_t getRSABlockSize(const BigNum& n) {
    if (n <= BigNum(256)) {
        throw std::invalid_argument("Error: RSA modulus n must be greater than 256 to encrypt byte blocks.");
    }
    size_t blockSize = 0;
    BigNum limit(256);
    while (limit <= n) {
        blockSize++;
        limit = limit * BigNum(256);
    }
    return blockSize;
}

BigNum rsaEncryptBlock(const BigNum& m, const BigNum& e, const BigNum& n) {
    if (m >= n) {
        throw std::invalid_argument("Error: Plaintext block must be strictly less than modulus n.");
    }
    return BigNum::modPow(m, e, n);
}

BigNum rsaDecryptBlock(const BigNum& c, const BigNum& d, const BigNum& n) {
    if (c >= n) {
        throw std::invalid_argument("Error: Ciphertext block must be strictly less than modulus n.");
    }
    return BigNum::modPow(c, d, n);
}

void encryptFileRSA(const std::string& inputPath,
                    const std::string& outputPath,
                    const BigNum& e,
                    const BigNum& n,
                    size_t* outBlockCount,
                    size_t* outFileSize) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Error: Cannot open input file for encryption: " + inputPath);
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::ofstream out(outputPath, std::ios::out | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Error: Cannot open output file for encrypted data: " + outputPath);
    }

    size_t blockSize = getRSABlockSize(n);
    out << data.size() << "\n";

    size_t blockCount = 0;
    for (size_t i = 0; i < data.size(); i += blockSize) {
        size_t chunkLen = (std::min)(blockSize, data.size() - i);
        BigNum m(0);
        for (size_t j = 0; j < chunkLen; ++j) {
            m = m * BigNum(256) + BigNum(static_cast<int>(data[i + j]));
        }
        BigNum c = rsaEncryptBlock(m, e, n);
        out << c.toString() << "\n";
        blockCount++;
    }

    out.close();
    if (!out) {
        throw std::runtime_error("Error: Failed while writing encrypted file: " + outputPath);
    }

    if (outBlockCount) *outBlockCount = blockCount;
    if (outFileSize) *outFileSize = data.size();
}

void decryptFileRSA(const std::string& inputPath,
                    const std::string& outputPath,
                    const BigNum& d,
                    const BigNum& n,
                    size_t* outBlockCount,
                    size_t* outFileSize) {
    std::ifstream in(inputPath);
    if (!in) {
        throw std::runtime_error("Error: Cannot open encrypted file for decryption: " + inputPath);
    }

    std::string headerLine;
    if (!std::getline(in, headerLine)) {
        throw std::runtime_error("Error: Encrypted file is empty or missing size header: " + inputPath);
    }

    size_t originalSize = 0;
    try {
        originalSize = static_cast<size_t>(std::stoull(headerLine));
    } catch (const std::exception&) {
        throw std::runtime_error("Error: Invalid size header in encrypted file: " + inputPath);
    }

    size_t blockSize = getRSABlockSize(n);
    std::vector<uint8_t> decrypted;
    decrypted.reserve(originalSize);

    size_t blockCount = 0;
    std::string line;
    while (decrypted.size() < originalSize && std::getline(in, line)) {
        if (line.empty()) continue;

        BigNum c(line);
        BigNum m = rsaDecryptBlock(c, d, n);

        size_t chunkLen = (std::min)(blockSize, originalSize - decrypted.size());
        std::vector<uint8_t> chunk(chunkLen, 0);
        BigNum temp = m;
        for (size_t j = chunkLen; j > 0; --j) {
            BigNum q, r;
            BigNum::divMod(temp, BigNum(256), q, r);
            chunk[j - 1] = static_cast<uint8_t>(std::stoi(r.toString()));
            temp = q;
        }
        decrypted.insert(decrypted.end(), chunk.begin(), chunk.end());
        blockCount++;
    }
    in.close();

    if (decrypted.size() != originalSize) {
        throw std::runtime_error("Error: Encrypted file truncated or corrupted (decoded " +
                                 std::to_string(decrypted.size()) + " of " +
                                 std::to_string(originalSize) + " bytes).");
    }

    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Error: Cannot open output file for decrypted data: " + outputPath);
    }

    if (!decrypted.empty()) {
        out.write(reinterpret_cast<const char*>(decrypted.data()), static_cast<std::streamsize>(decrypted.size()));
    }
    out.close();
    if (!out) {
        throw std::runtime_error("Error: Failed while writing decrypted file: " + outputPath);
    }

    if (outBlockCount) *outBlockCount = blockCount;
    if (outFileSize) *outFileSize = decrypted.size();
}

bool verifyFilesMatch(const std::string& pathA,
                      const std::string& pathB,
                      size_t* outSizeA,
                      size_t* outSizeB) {
    std::ifstream fileA(pathA, std::ios::binary);
    if (!fileA) {
        throw std::runtime_error("Error: Cannot open file for comparison: " + pathA);
    }
    std::ifstream fileB(pathB, std::ios::binary);
    if (!fileB) {
        throw std::runtime_error("Error: Cannot open file for comparison: " + pathB);
    }

    std::vector<uint8_t> bufA((std::istreambuf_iterator<char>(fileA)), std::istreambuf_iterator<char>());
    std::vector<uint8_t> bufB((std::istreambuf_iterator<char>(fileB)), std::istreambuf_iterator<char>());

    if (outSizeA) *outSizeA = bufA.size();
    if (outSizeB) *outSizeB = bufB.size();

    return bufA == bufB;
}
