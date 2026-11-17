#include "BigNum.h"
#include "RSA.h"
#include "CryptoRandom.h"
#include <iostream>
#include <chrono>
#include <string>

void runBigNumTests(int numTests = 5, int maxDigits = 40) {
    std::cout << "\n======================================================\n";
    std::cout << "  RUNNING " << numTests << " RANDOM TESTS FOR BigNum (up to " << maxDigits << " digits)\n";
    std::cout << "  Random numbers generated via CSPRNG (BCryptGenRandom)\n";
    std::cout << "======================================================\n\n";

    bool allPassed = true;

    for (int t = 1; t <= numTests; ++t) {
        int digitsA = CryptoRandom::getInt(10, maxDigits);
        int digitsB = CryptoRandom::getInt(10, maxDigits);
        int digitsMod = CryptoRandom::getInt(10, maxDigits);

        BigNum a = BigNum::generateRandom(digitsA);
        BigNum b = BigNum::generateRandom(digitsB);
        BigNum m = BigNum::generateRandom(digitsMod);

        std::cout << "--- Test " << t << " ---\n";
        std::cout << "a (" << digitsA << " digits): " << a << "\n";
        std::cout << "b (" << digitsB << " digits): " << b << "\n";
        std::cout << "m (" << digitsMod << " digits): " << m << "\n\n";

        // add
        BigNum sum = a + b;
        BigNum sum2 = b + a;
        std::cout << "a + b = " << sum << "\n";
        if (sum != sum2) {
            std::cout << "[FAIL] Addition commutativity violated!\n";
            allPassed = false;
        }

        // sub
        if (a >= b) {
            BigNum sub = a - b;
            std::cout << "a - b = " << sub << "\n";
            if ((sub + b) != a) {
                std::cout << "[FAIL] Subtraction identity violated: (a - b) + b != a\n";
                allPassed = false;
            }
        } else {
            BigNum sub = b - a;
            std::cout << "b - a = " << sub << "\n";
            if ((sub + a) != b) {
                std::cout << "[FAIL] Subtraction identity violated: (b - a) + a != b\n";
                allPassed = false;
            }
        }

        // mul
        BigNum prod = a * b;
        BigNum prod2 = b * a;
        std::cout << "a * b = " << prod << "\n";
        if (prod != prod2) {
            std::cout << "[FAIL] Multiplication commutativity violated!\n";
            allPassed = false;
        }

        // sqr
        BigNum aSqr = a.sqr();
        BigNum aSqr2 = a * a;
        if (aSqr != aSqr2) {
            std::cout << "[FAIL] Squaring inconsistency: a.sqr() != a * a\n";
            allPassed = false;
        }

        // div mod
        BigNum q, r;
        BigNum::divMod(a, m, q, r);
        std::cout << "a / m = " << q << "\n";
        std::cout << "a % m = " << r << "\n";
        if ((q * m + r) != a) {
            std::cout << "[FAIL] Division identity violated: (q * m + r) != a\n";
            allPassed = false;
        }
        if (r >= m) {
            std::cout << "[FAIL] Remainder property violated: r >= m\n";
            allPassed = false;
        }

        // mod
        BigNum lhs = ((a % m) + (b % m)) % m;
        BigNum rhs = (a + b) % m;
        if (lhs != rhs) {
            std::cout << "[FAIL] Modular addition property violated!\n";
            allPassed = false;
        }

        BigNum lhsMul = ((a % m) * (b % m)) % m;
        BigNum rhsMul = (a * b) % m;
        if (lhsMul != rhsMul) {
            std::cout << "[FAIL] Modular multiplication property violated!\n";
            allPassed = false;
        }

        std::cout << "\n";
    }

    // modInverse
    BigNum testA("65537");
    BigNum testM("1000000007");
    BigNum inv = BigNum::modInverse(testA, testM);
    BigNum check = (testA * inv) % testM;
    std::cout << "Testing modInverse: (" << testA << " * " << inv << ") % " << testM << " = " << check << "\n";
    if (!check.isOne()) {
        std::cout << "[FAIL] modInverse test failed!\n";
        allPassed = false;
    }

    if (allPassed) {
        std::cout << "=== All BigNum tests PASSED successfully! ===\n\n";
    } else {
        std::cout << "=== Some tests FAILED! ===\n\n";
    }
}

void runPrimalityTests() {
    std::cout << "\n======================================================\n";
    std::cout << "  PRIMALITY TEST VERIFICATION (FERMAT vs MILLER-RABIN)\n";
    std::cout << "======================================================\n\n";

    struct TestCase {
        std::string numStr;
        bool expectedPrime;
        std::string note;
    };

    TestCase tests[] = {
        {"2", true, "Smallest prime"},
        {"3", true, "Small prime"},
        {"4", false, "Even composite"},
        {"17", true, "Fermat prime"},
        {"561", false, "Carmichael number (3 * 11 * 17)"},
        {"216821881", false, "Carmichael number (331 * 661 * 991, all factors > 307)"},
        {"1009", true, "4-digit prime"},
        {"1000000007", true, "Known 10-digit prime"},
        {"1000000009", true, "Known 10-digit prime"},
        {"1000000011", false, "Composite (3 * 333333337)"},
        {"275604541", true, "Large prime"},
        {"10000000000000000051", true, "20-digit prime"}
    };

    for (const auto& tc : tests) {
        BigNum n(tc.numStr);
        bool resFermat = isPrimeFermat(n, 25);
        bool resMR = isPrimeMillerRabin(n, 25);
        std::cout << "Number: " << n << " (" << tc.note << ")\n";
        std::cout << "  Fermat:       " << (resFermat ? "PRIME" : "COMPOSITE")
                  << " -> " << (resFermat == tc.expectedPrime ? "[OK]" : "[MISMATCH]") << "\n";
        std::cout << "  Miller-Rabin: " << (resMR ? "PRIME" : "COMPOSITE")
                  << " -> " << (resMR == tc.expectedPrime ? "[OK]" : "[MISMATCH]") << "\n";
    }
    std::cout << "\n";
}

void printRSAKeys(const RSAKeyPair& keys) {
    std::cout << "Computed RSA parameters:\n";
    std::cout << "  p (" << keys.p.digitCount() << " digits) = " << keys.p << "\n";
    std::cout << "  q (" << keys.q.digitCount() << " digits) = " << keys.q << "\n";
    std::cout << "  Modulus n = p * q (" << keys.n.digitCount() << " digits):\n    " << keys.n << "\n";
    std::cout << "  Euler totient phi(n) = (p - 1) * (q - 1):\n    " << keys.phi << "\n\n";

    std::cout << "--- PUBLIC KEY (Open part: e, n) ---\n";
    std::cout << "  e = " << keys.e << "\n";
    std::cout << "  n = " << keys.n << "\n\n";

    std::cout << "--- PRIVATE KEY (Secret part: d, n) ---\n";
    std::cout << "  d = " << keys.d << "\n";
    std::cout << "  n = " << keys.n << "\n\n";

    BigNum checkDE = (keys.d * keys.e) % keys.phi;
    std::cout << "Key validity verification:\n";
    std::cout << "  (d * e) mod phi(n) = " << checkDE
              << " -> " << (checkDE.isOne() ? "[VALID]" : "[INVALID]") << "\n\n";
}

RSAKeyPair runRSADemo(int primeDigits = 25) {
    if (primeDigits < 2) {
        throw std::invalid_argument("Prime digit length must be at least 2 so that n > 256.");
    }

    std::cout << "\n======================================================\n";
    std::cout << "  RSA KEY GENERATION & PARAMETERS (" << primeDigits << " digits per prime)\n";
    std::cout << "  Using CSPRNG (BCryptGenRandom) and Miller-Rabin Test\n";
    std::cout << "======================================================\n\n";

    std::cout << "1. Generating large primes p and q using Miller-Rabin test...\n";
    auto tStart = std::chrono::high_resolution_clock::now();

    RSAKeyPair keys = generateRSAKeys(primeDigits);

    auto tEnd = std::chrono::high_resolution_clock::now();
    double genTimeMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
    std::cout << "   Generation time: " << genTimeMs << " ms\n\n";

    printRSAKeys(keys);
    return keys;
}

void runLab3Workflow(RSAKeyPair& currentKeys, bool& hasKeys) {
    std::cout << "\n======================================================\n";
    std::cout << "  LAB 3: RSA KEY GENERATION, FILE ENCRYPTION & DECRYPTION\n";
    std::cout << "======================================================\n\n";

    int primeDigits = 20;
    std::cout << "Enter key base length (digits per prime p and q, e.g. 20 for ~40-digit modulus, min 2): ";
    std::cin >> primeDigits;
    if (primeDigits < 2) {
        std::cout << "Using minimum valid length: 2 digits per prime.\n";
        primeDigits = 2;
    }

    currentKeys = runRSADemo(primeDigits);
    hasKeys = true;

    std::string inputPath;
    std::string encryptedPath;
    std::string decryptedPath;

    std::cout << "Enter source file name to encrypt (e.g. input.txt): ";
    std::cin >> inputPath;
    std::cout << "Enter output file name for encrypted data (e.g. encrypted.txt): ";
    std::cin >> encryptedPath;

    size_t encBlocks = 0;
    size_t origSize = 0;
    auto tEncStart = std::chrono::high_resolution_clock::now();
    encryptFileRSA(inputPath, encryptedPath, currentKeys.e, currentKeys.n, &encBlocks, &origSize);
    auto tEncEnd = std::chrono::high_resolution_clock::now();
    double encMs = std::chrono::duration<double, std::milli>(tEncEnd - tEncStart).count();

    std::cout << "\n[ENCRYPTED] " << origSize << " bytes in " << encBlocks << " block(s) "
              << "(block size: " << getRSABlockSize(currentKeys.n) << " bytes) -> "
              << encryptedPath << " (" << encMs << " ms)\n\n";

    std::cout << "Enter output file name for decrypted data (e.g. decrypted.txt): ";
    std::cin >> decryptedPath;

    size_t decBlocks = 0;
    size_t decSize = 0;
    auto tDecStart = std::chrono::high_resolution_clock::now();
    decryptFileRSA(encryptedPath, decryptedPath, currentKeys.d, currentKeys.n, &decBlocks, &decSize);
    auto tDecEnd = std::chrono::high_resolution_clock::now();
    double decMs = std::chrono::duration<double, std::milli>(tDecEnd - tDecStart).count();

    std::cout << "[DECRYPTED] " << decSize << " bytes in " << decBlocks << " block(s) -> "
              << decryptedPath << " (" << decMs << " ms)\n\n";

    size_t sizeA = 0, sizeB = 0;
    bool match = verifyFilesMatch(inputPath, decryptedPath, &sizeA, &sizeB);
    std::cout << "Integrity check (" << inputPath << " vs " << decryptedPath << "):\n";
    std::cout << "  Original size:  " << sizeA << " bytes\n";
    std::cout << "  Decrypted size: " << sizeB << " bytes\n";
    std::cout << "  Result: " << (match ? "[PASS] Decrypted file matches original identically!" : "[FAIL] Files differ!") << "\n";
}

int main() {
    RSAKeyPair currentKeys;
    bool hasKeys = false;

    while (true) {
        std::cout << "\nMENU:\n";
        std::cout << "1. Run BigNum arithmetic tests\n";
        std::cout << "2. Test number for primality (Fermat & Miller-Rabin) + Carmichael suite\n";
        std::cout << "3. Generate Large Prime Number (Miller-Rabin)\n";
        std::cout << "4. Generate and compute RSA keys (p, q, n, phi, e, d)\n";
        std::cout << "5. Lab 3 Full Workflow (Keygen + Encrypt + Decrypt + Verify)\n";
        std::cout << "6. Encrypt file with current RSA public key\n";
        std::cout << "7. Decrypt file with current RSA private key\n";
        std::cout << "0. Exit\n";
        std::cout << "Select option (0-7): ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            break;
        }

        try {
            if (choice == 1) {
                runBigNumTests(5, 40);
            } else if (choice == 2) {
                runPrimalityTests();
                std::cout << "Enter a number to test for primality (or 'skip'): ";
                std::string numStr;
                std::cin >> numStr;
                if (numStr != "skip") {
                    BigNum n(numStr);
                    std::cout << "Testing " << n << " with 30 rounds (Fermat & Miller-Rabin)...\n";
                    auto t1 = std::chrono::high_resolution_clock::now();
                    bool fermatRes = isPrimeFermat(n, 30);
                    bool mrRes = isPrimeMillerRabin(n, 30);
                    auto t2 = std::chrono::high_resolution_clock::now();
                    double elapsed = std::chrono::duration<double, std::milli>(t2 - t1).count();
                    std::cout << "Fermat result:       " << (fermatRes ? "PRIME (likely prime)" : "COMPOSITE") << "\n";
                    std::cout << "Miller-Rabin result: " << (mrRes ? "PRIME (likely prime)" : "COMPOSITE") << "\n";
                    std::cout << "Elapsed time: " << elapsed << " ms\n";
                }
            } else if (choice == 3) {
                std::cout << "Enter desired number of decimal digits (e.g. 20, 30, 40): ";
                int digits = 25;
                std::cin >> digits;
                if (digits <= 0) digits = 25;
                std::cout << "Generating large prime with " << digits << " digits (Miller-Rabin, BCryptGenRandom)...\n";
                int candidates = 0;
                auto t1 = std::chrono::high_resolution_clock::now();
                BigNum prime = generateLargePrime(digits, 25, &candidates);
                auto t2 = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double, std::milli>(t2 - t1).count();

                std::cout << "Found Prime (" << prime.digitCount() << " digits):\n" << prime << "\n";
                std::cout << "Candidates tested: " << candidates << "\n";
                std::cout << "Elapsed time: " << elapsed << " ms\n";
            } else if (choice == 4) {
                std::cout << "Enter prime digit length for p and q (e.g. 20 for ~40 digit modulus, min 2): ";
                int digits = 20;
                std::cin >> digits;
                if (digits < 2) digits = 2;
                currentKeys = runRSADemo(digits);
                hasKeys = true;
            } else if (choice == 5) {
                runLab3Workflow(currentKeys, hasKeys);
            } else if (choice == 6) {
                if (!hasKeys) {
                    std::cout << "No RSA keys generated yet. Generating 20-digit prime keys first...\n";
                    currentKeys = runRSADemo(20);
                    hasKeys = true;
                }
                std::string inPath, outPath;
                std::cout << "Enter input file name to encrypt: ";
                std::cin >> inPath;
                std::cout << "Enter output file name for encrypted data: ";
                std::cin >> outPath;
                size_t blocks = 0, bytes = 0;
                encryptFileRSA(inPath, outPath, currentKeys.e, currentKeys.n, &blocks, &bytes);
                std::cout << "[ENCRYPTED] " << bytes << " bytes in " << blocks << " block(s) -> " << outPath << "\n";
            } else if (choice == 7) {
                if (!hasKeys) {
                    std::cout << "No RSA keys available in this session. Run option 4 or 5 first.\n";
                } else {
                    std::string encPath, decPath, origPath;
                    std::cout << "Enter encrypted file name: ";
                    std::cin >> encPath;
                    std::cout << "Enter output file name for decrypted data: ";
                    std::cin >> decPath;
                    size_t blocks = 0, bytes = 0;
                    decryptFileRSA(encPath, decPath, currentKeys.d, currentKeys.n, &blocks, &bytes);
                    std::cout << "[DECRYPTED] " << bytes << " bytes in " << blocks << " block(s) -> " << decPath << "\n";
                    std::cout << "Enter original file name to verify match (or 'skip'): ";
                    std::cin >> origPath;
                    if (origPath != "skip") {
                        size_t sizeA = 0, sizeB = 0;
                        bool match = verifyFilesMatch(origPath, decPath, &sizeA, &sizeB);
                        std::cout << "Verification: " << (match ? "[PASS] Files match identically!" : "[FAIL] Files differ!")
                                  << " (" << sizeA << " vs " << sizeB << " bytes)\n";
                    }
                }
            } else if (choice == 0) {
                std::cout << "Exiting. Goodbye!\n";
                break;
            } else {
                std::cout << "Invalid choice. Please select 0-7.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "\n[ERROR] " << e.what() << "\n";
        }
    }

    return 0;
}
