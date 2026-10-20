#include "BigNum.h"
#include "CryptoRandom.h"

void BigNum::remove_zeros() {
    while (num.size() > 1 && num.back() == 0) {
        num.pop_back();
    }
    if (num.empty()) {
        num.push_back(0);
    }
}

BigNum::BigNum() {
    num.push_back(0);
}

BigNum::BigNum(int val) : BigNum(static_cast<long long>(val)) {}

BigNum::BigNum(long long val) {
    if (val < 0) {
        throw std::invalid_argument("Error: BigNum currently supports only non-negative integers.");
    }
    if (val == 0) {
        num.push_back(0);
    } else {
        while (val > 0) {
            num.push_back(static_cast<int>(val % 10));
            val /= 10;
        }
    }
}

BigNum::BigNum(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Error: Input string is empty.");
    }
    size_t start = 0;
    while (start < str.size() && isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    size_t end = str.size();
    while (end > start && isspace(static_cast<unsigned char>(str[end - 1]))) {
        end--;
    }
    if (start >= end) {
        throw std::invalid_argument("Error: Input string is empty or contains only spaces.");
    }

    for (int i = static_cast<int>(end) - 1; i >= static_cast<int>(start); i--) {
        if (!isdigit(static_cast<unsigned char>(str[i]))) {
            throw std::invalid_argument("Error: Input string contains non-digit characters.");
        }
        num.push_back(str[i] - '0');
    }
    remove_zeros();
}

bool BigNum::isZero() const {
    return num.size() == 1 && num[0] == 0;
}

bool BigNum::isOne() const {
    return num.size() == 1 && num[0] == 1;
}

bool BigNum::isOdd() const {
    return !num.empty() && (num[0] % 2 != 0);
}

bool BigNum::isEven() const {
    return !num.empty() && (num[0] % 2 == 0);
}

size_t BigNum::digitCount() const {
    return num.size();
}

bool BigNum::operator<(const BigNum& other) const {
    if (num.size() != other.num.size())
        return num.size() < other.num.size();
    for (int i = static_cast<int>(num.size()) - 1; i >= 0; i--) {
        if (num[i] != other.num[i])
            return num[i] < other.num[i];
    }
    return false;
}

bool BigNum::operator==(const BigNum& other) const {
    return num == other.num;
}

bool BigNum::operator!=(const BigNum& other) const {
    return !(*this == other);
}

bool BigNum::operator<=(const BigNum& other) const {
    return *this < other || *this == other;
}

bool BigNum::operator>(const BigNum& other) const {
    return !(*this <= other);
}

bool BigNum::operator>=(const BigNum& other) const {
    return !(*this < other);
}

BigNum BigNum::operator+(const BigNum& other) const {
    BigNum res;
    res.num.clear();
    int carry = 0;
    int n = static_cast<int>((std::max)(num.size(), other.num.size()));
    for (int i = 0; i < n || carry; i++) {
        int sum = carry;
        if (i < static_cast<int>(num.size())) sum += num[i];
        if (i < static_cast<int>(other.num.size())) sum += other.num[i];
        res.num.push_back(sum % 10);
        carry = sum / 10;
    }
    res.remove_zeros();
    return res;
}

BigNum BigNum::operator-(const BigNum& other) const {
    if (*this < other) {
        throw std::invalid_argument("Error: Subtraction would produce a negative result.");
    }
    BigNum res;
    res.num.clear();
    int carry = 0;
    for (size_t i = 0; i < num.size(); i++) {
        int sub = num[i] - carry - (i < other.num.size() ? other.num[i] : 0);
        if (sub < 0) {
            sub += 10;
            carry = 1;
        } else {
            carry = 0;
        }
        res.num.push_back(sub);
    }
    res.remove_zeros();
    return res;
}

BigNum BigNum::operator*(const BigNum& other) const {
    if (isZero() || other.isZero()) return BigNum(0);
    BigNum res;
    res.num.assign(num.size() + other.num.size(), 0);
    for (size_t i = 0; i < num.size(); ++i) {
        long long carry = 0;
        for (size_t j = 0; j < other.num.size() || carry; ++j) {
            long long cur = res.num[i + j] + num[i] * 1LL * (j < other.num.size() ? other.num[j] : 0) + carry;
            res.num[i + j] = static_cast<int>(cur % 10);
            carry = cur / 10;
        }
    }
    res.remove_zeros();
    return res;
}

void BigNum::divMod(const BigNum& a, const BigNum& b, BigNum& q, BigNum& r) {
    if (b.isZero()) {
        throw std::domain_error("Error: Division by zero.");
    }
    if (a < b) {
        q = BigNum(0);
        r = a;
        return;
    }

    q.num.clear();
    r = BigNum(0);

    for (int i = static_cast<int>(a.num.size()) - 1; i >= 0; --i) {
        if (!r.isZero()) {
            r.num.insert(r.num.begin(), 0);
        }
        r.num[0] += a.num[i];
        r.remove_zeros();

        int count = 0;
        while (r >= b) {
            r = r - b;
            count++;
        }
        q.num.push_back(count);
    }
    std::reverse(q.num.begin(), q.num.end());
    q.remove_zeros();
}

BigNum BigNum::operator/(const BigNum& other) const {
    BigNum q, r;
    divMod(*this, other, q, r);
    return q;
}

BigNum BigNum::operator%(const BigNum& other) const {
    BigNum q, r;
    divMod(*this, other, q, r);
    return r;
}

BigNum BigNum::sqr() const {
    return (*this) * (*this);
}

BigNum BigNum::div2() const {
    BigNum res;
    res.num.clear();
    int carry = 0;
    for (int i = static_cast<int>(num.size()) - 1; i >= 0; i--) {
        int current = num[i] + carry * 10;
        res.num.push_back(current / 2);
        carry = current % 2;
    }
    std::reverse(res.num.begin(), res.num.end());
    res.remove_zeros();
    return res;
}

BigNum BigNum::gcd(BigNum a, BigNum b) {
    while (!b.isZero()) {
        BigNum rem = a % b;
        a = b;
        b = rem;
    }
    return a;
}

BigNum BigNum::modPow(BigNum base, BigNum exp, const BigNum& mod) {
    if (mod.isZero()) {
        throw std::domain_error("Error: Modulus cannot be zero.");
    }
    if (mod.isOne()) {
        return BigNum(0);
    }
    BigNum res(1);
    base = base % mod;
    while (!exp.isZero()) {
        if (exp.isOdd()) {
            res = (res * base) % mod;
        }
        base = base.sqr() % mod;
        exp = exp.div2();
    }
    return res;
}

BigNum BigNum::modInverse(const BigNum& a, const BigNum& m) {
    if (m <= BigNum(1)) {
        throw std::invalid_argument("Error: Modulus must be greater than 1.");
    }
    BigNum r0 = m, r1 = a % m;
    BigNum t0(0), t1(1);

    while (!r1.isZero()) {
        BigNum q = r0 / r1;
        BigNum r2 = r0 - q * r1;

        BigNum q_t1 = (q % m) * t1 % m;
        BigNum t2;
        if (t0 >= q_t1) {
            t2 = (t0 - q_t1) % m;
        } else {
            t2 = (m - ((q_t1 - t0) % m)) % m;
        }

        r0 = r1;
        r1 = r2;
        t0 = t1;
        t1 = t2;
    }

    if (!r0.isOne()) {
        throw std::runtime_error("Error: Modular inverse does not exist (numbers are not coprime).");
    }
    return t0;
}

BigNum BigNum::generateRandom(int numDigits) {
    if (numDigits <= 0) {
        throw std::invalid_argument("Error: Number of digits must be positive.");
    }
    std::string s;
    s.reserve(numDigits);
    s += static_cast<char>('0' + CryptoRandom::getDigit(1, 9));
    for (int i = 1; i < numDigits; ++i) {
        s += static_cast<char>('0' + CryptoRandom::getDigit(0, 9));
    }
    return BigNum(s);
}

BigNum BigNum::generateRandomOdd(int numDigits) {
    if (numDigits <= 0) {
        throw std::invalid_argument("Error: Number of digits must be positive.");
    }
    static const int oddDigits[] = {1, 3, 7, 9};
    std::string s;
    s.reserve(numDigits);
    s += static_cast<char>('0' + CryptoRandom::getDigit(1, 9));
    for (int i = 1; i < numDigits - 1; ++i) {
        s += static_cast<char>('0' + CryptoRandom::getDigit(0, 9));
    }
    if (numDigits > 1) {
        int lastIdx = CryptoRandom::getInt(0, 3);
        s += static_cast<char>('0' + oddDigits[lastIdx]);
    } else {
        static const int singleOdds[] = {3, 5, 7};
        int idx = CryptoRandom::getInt(0, 2);
        s = std::to_string(singleOdds[idx]);
    }
    return BigNum(s);
}

BigNum BigNum::generateRandomRange(const BigNum& minVal, const BigNum& maxVal) {
    if (minVal > maxVal) {
        throw std::invalid_argument("Error: minVal cannot be greater than maxVal.");
    }
    if (minVal == maxVal) {
        return minVal;
    }
    BigNum diff = maxVal - minVal;
    size_t digits = diff.digitCount();

    while (true) {
        size_t len = static_cast<size_t>(CryptoRandom::getInt(1, static_cast<int>(digits)));
        std::string s;
        s.reserve(len);
        s += static_cast<char>('0' + CryptoRandom::getDigit(1, 9));
        for (size_t i = 1; i < len; ++i) {
            s += static_cast<char>('0' + CryptoRandom::getDigit(0, 9));
        }
        BigNum candidate(s);
        if (candidate <= diff) {
            return minVal + candidate;
        }
    }
}

std::string BigNum::toString() const {
    std::string s;
    s.reserve(num.size());
    for (int i = static_cast<int>(num.size()) - 1; i >= 0; --i) {
        s += static_cast<char>('0' + num[i]);
    }
    return s;
}

std::ostream& operator<<(std::ostream& os, const BigNum& b) {
    os << b.toString();
    return os;
}

std::istream& operator>>(std::istream& is, BigNum& b) {
    std::string s;
    if (is >> s) {
        b = BigNum(s);
    }
    return is;
}
