#ifndef BIGNUM_H
#define BIGNUM_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

class BigNum {
private:
    std::vector<int> num;

    void remove_zeros();

public:
    BigNum();
    BigNum(int val);
    BigNum(long long val);
    BigNum(const std::string& str);

    bool isZero() const;
    bool isOne() const;
    bool isOdd() const;
    bool isEven() const;
    size_t digitCount() const;

    bool operator<(const BigNum& other) const;
    bool operator==(const BigNum& other) const;
    bool operator!=(const BigNum& other) const;
    bool operator<=(const BigNum& other) const;
    bool operator>(const BigNum& other) const;
    bool operator>=(const BigNum& other) const;

    BigNum operator+(const BigNum& other) const;
    BigNum operator-(const BigNum& other) const;
    BigNum operator*(const BigNum& other) const;
    BigNum operator/(const BigNum& other) const;
    BigNum operator%(const BigNum& other) const;

    BigNum sqr() const;
    BigNum div2() const;

    static void divMod(const BigNum& a, const BigNum& b, BigNum& q, BigNum& r);

    static BigNum gcd(BigNum a, BigNum b);
    static BigNum modPow(BigNum base, BigNum exp, const BigNum& mod);
    static BigNum modInverse(const BigNum& a, const BigNum& m);

    static BigNum generateRandom(int numDigits);
    static BigNum generateRandomOdd(int numDigits);
    static BigNum generateRandomRange(const BigNum& minVal, const BigNum& maxVal);

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const BigNum& b);
    friend std::istream& operator>>(std::istream& is, BigNum& b);
};

#endif // BIGNUM_H
