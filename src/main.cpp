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

void runFermatTests() {
    std::cout << "\n======================================================\n";
    std::cout << "  FERMAT PRIMALITY TEST VERIFICATION\n";
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
        {"1009", true, "4-digit prime"},
        {"1000000007", true, "Known 10-digit prime"},
        {"1000000009", true, "Known 10-digit prime"},
        {"1000000011", false, "Composite (3 * 333333337)"},
        {"275604541", true, "Large prime"},
        {"10000000000000000051", true, "20-digit prime"}
    };

    for (const auto& tc : tests) {
        BigNum n(tc.numStr);
        bool res = isPrimeFermat(n, 25);
        std::cout << "Number: " << n << " (" << tc.note << ")\n";
        std::cout << "  Fermat result: " << (res ? "PRIME" : "COMPOSITE")
                  << " | Expected: " << (tc.expectedPrime ? "PRIME" : "COMPOSITE")
                  << " -> " << (res == tc.expectedPrime ? "[OK]" : "[MISMATCH]") << "\n";
    }
    std::cout << "\n";
}

void runRSADemo(int primeDigits = 25) {
    std::cout << "\n======================================================\n";
    std::cout << "  RSA KEY GENERATION & PARAMETERS (" << primeDigits << " digits per prime)\n";
    std::cout << "  Using CSPRNG (BCryptGenRandom) and Fermat's Little Theorem\n";
    std::cout << "======================================================\n\n";

    std::cout << "1. Generating large primes p and q using Fermat's Little Theorem...\n";
    auto tStart = std::chrono::high_resolution_clock::now();

    RSAKeyPair keys = generateRSAKeys(primeDigits);

    auto tEnd = std::chrono::high_resolution_clock::now();
    double genTimeMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    std::cout << "   p (" << keys.p.digitCount() << " digits) = " << keys.p << "\n";
    std::cout << "   q (" << keys.q.digitCount() << " digits) = " << keys.q << "\n";
    std::cout << "   Generation time: " << genTimeMs << " ms\n\n";

    std::cout << "2. Computed RSA parameters:\n";
    std::cout << "   Modulus n (p * q) [" << keys.n.digitCount() << " digits]:\n   " << keys.n << "\n\n";
    std::cout << "   Euler totient phi(n) = (p - 1) * (q - 1):\n   " << keys.phi << "\n\n";
    std::cout << "   Public exponent e:\n   " << keys.e << "\n\n";
    std::cout << "   Private exponent d = e^(-1) mod phi(n):\n   " << keys.d << "\n\n";

    // Перевірка d * e mod phi == 1
    BigNum checkDE = (keys.d * keys.e) % keys.phi;
    std::cout << "3. Key validity verification:\n";
    std::cout << "   (d * e) mod phi(n) = " << checkDE
              << " -> " << (checkDE.isOne() ? "[VALID: Keys generated correctly]" : "[INVALID]") << "\n\n";
}

int main() {
    while (true) {
        std::cout << "\nMENU:\n";
        std::cout << "1. Run BigNum arithmetic tests\n";
        std::cout << "2. Test numbers for primality (Fermat's Little Theorem)\n";
        std::cout << "3. Generate Large Prime Number (specify digits)\n";
        std::cout << "4. Generate and compute RSA keys (p, q, n, phi, e, d)\n";
        std::cout << "0. Exit\n";
        std::cout << "Select option (0-4): ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            break;
        }

        try {
            if (choice == 1) {
                runBigNumTests(5, 40);
            } else if (choice == 2) {
                std::cout << "Enter a large number to test for primality: ";
                std::string numStr;
                std::cin >> numStr;
                BigNum n(numStr);
                std::cout << "Testing " << n << " with 30 Fermat rounds (CSPRNG random bases)...\n";
                auto t1 = std::chrono::high_resolution_clock::now();
                bool isPrime = isPrimeFermat(n, 30);
                auto t2 = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double, std::milli>(t2 - t1).count();
                std::cout << "Result: " << (isPrime ? "PRIME (likely prime)" : "COMPOSITE") << "\n";
                std::cout << "Elapsed time: " << elapsed << " ms\n";
            } else if (choice == 3) {
                std::cout << "Enter desired number of decimal digits (e.g. 20, 30, 40): ";
                int digits = 25;
                std::cin >> digits;
                if (digits <= 0) digits = 25;
                std::cout << "Generating large prime with " << digits << " digits via BCryptGenRandom...\n";
                int candidates = 0;
                auto t1 = std::chrono::high_resolution_clock::now();
                BigNum prime = generateLargePrime(digits, 25, &candidates);
                auto t2 = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double, std::milli>(t2 - t1).count();

                std::cout << "Found Prime (" << prime.digitCount() << " digits):\n" << prime << "\n";
                std::cout << "Candidates tested: " << candidates << "\n";
                std::cout << "Elapsed time: " << elapsed << " ms\n";
            } else if (choice == 4) {
                std::cout << "Enter prime digit length for p and q (e.g. 20 for ~40 digit modulus): ";
                int digits = 20;
                std::cin >> digits;
                if (digits <= 0) digits = 20;
                runRSADemo(digits);
            } else if (choice == 0) {
                std::cout << "Exiting. Goodbye!\n";
                break;
            } else {
                std::cout << "Invalid choice. Please select 0-4.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "\n[ERROR] " << e.what() << "\n";
        }
    }

    return 0;
}
