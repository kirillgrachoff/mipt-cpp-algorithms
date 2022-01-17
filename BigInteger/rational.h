#ifndef RATIONAL_H
#define RATIONAL_H

#include "biginteger.h"

namespace RationalHelpers {
BigInteger gcd(BigInteger a, BigInteger b) {
    while (a != 0) {
        b %= a;
        a.swap(b);
    }
    return b;
}

BigInteger lcm(const BigInteger& a, const BigInteger& b) {
    return a / gcd(a, b) * b;
}
}

class Rational {
    BigInteger num = 0;
    BigInteger den = 1;

    int sign() const;

    void normalize() {
        using RationalHelpers::gcd;
        BigInteger g = gcd(num, den);
        if (g.sign() < 0) g *= -1;
        this->num /= g;
        this->den /= g;
        if (den.sign() < 0) {
            den *= -1;
            num *= -1;
        }
    }

    BigInteger withPrecision(size_t) const;

public:
    Rational(): num(0), den(1) {}

    Rational(const BigInteger& num, const BigInteger& den): num(num), den(den) {
        normalize();
    }

    Rational(const BigInteger& num): num(num), den(1) {} // implicit

    Rational(int n): num(BigInteger(n)), den(1) {} // implicit

    Rational(const Rational& that): Rational(that.num, that.den) {}

    Rational& operator=(const Rational&) &;

    void swap(Rational&) &;

    Rational& operator+=(const Rational&) &;

    Rational& operator-=(const Rational&) &;

    Rational& operator/=(const Rational&) &;

    Rational& operator*=(const Rational&) &;

    friend bool operator<(const Rational&, const Rational&);
    friend bool operator==(const Rational&, const Rational&);

    std::string toString() const;

    std::string asDecimal(size_t) const;

    explicit operator double() const {
        BigInteger ans = withPrecision(10);
        double doubleans = static_cast<double>(static_cast<long long>(ans));
        doubleans /= 10000000000.0;
        return doubleans;
    }

    Rational operator-() {
        Rational ans = *this;
        ans *= -1_bi;
        return ans;
    }
};

int Rational::sign() const {
    return num.sign();
}

void Rational::swap(Rational& that) & {
    num.swap(that.num);
    den.swap(that.den);
}

Rational& Rational::operator=(const Rational& that) & {
    if (this == &that) return *this;
    Rational tmp = that;
    swap(tmp);
    return *this;
}

Rational& Rational::operator+=(const Rational& that) & {
    BigInteger lm = RationalHelpers::lcm(den, that.den);
    num = num * (lm / den) + (that.num * lm / that.den);
    den = lm;
    normalize();
    return *this;
}

Rational& Rational::operator-=(const Rational& that) & {
    BigInteger lm = RationalHelpers::lcm(den, that.den);
    num = num * (lm / den) - (that.num * lm / that.den);
    den = lm;
    normalize();
    return *this;
}

Rational& Rational::operator*=(const Rational& that) & {
    num *= that.num;
    den *= that.den;
    normalize();
    return *this;
}

Rational& Rational::operator/=(const Rational& that) & {
    num *= that.den;
    den *= that.num;
    normalize();
    return *this;
}

BigInteger Rational::withPrecision(size_t precision) const {
    BigInteger a = num;
    BigInteger b = den;
    a.mulPow10(precision);
    BigInteger ans = a / b;
    return ans;
}

std::string Rational::asDecimal(size_t precision = 0) const {
    BigInteger ans = withPrecision(precision);
    int sign = ans.sign();
    if (sign < 0) {
        ans *= -1;
    }
    std::string sans = static_cast<std::string>(ans);
    if (sans.size() < precision) {
        std::string addZeros(precision - sans.size(), '0');
        sans = addZeros + sans;
    }
    std::string trunc = std::string(sans.begin(), sans.end() - precision);
    if (trunc.empty()) trunc = "0";
    if (trunc == "-") trunc = "-0";
    std::string rem = std::string(sans.end() - precision, sans.end());
    return (sign < 0? "-" : "") + trunc + '.' + rem;
}

std::string Rational::toString() const {
    if (den == 1) return static_cast<std::string>(num);
    return static_cast<std::string>(num) + '/' + static_cast<std::string>(den);
}

std::ostream& operator<<(std::ostream& out, const Rational& r) {
    out << r.toString();
    return out;
}

std::istream& operator>>(std::istream& in, Rational& r) {
    int n;
    in >> n;
    r = n;
    return in;
}

Rational operator+(const Rational& lhs, const Rational& rhs) {
    Rational ans = lhs;
    ans += rhs;
    return ans;
}

Rational operator-(const Rational& lhs, const Rational& rhs) {
    Rational ans = lhs;
    ans -= rhs;
    return ans;
}

Rational operator*(const Rational& lhs, const Rational& rhs) {
    Rational ans = lhs;
    ans *= rhs;
    return ans;
}

Rational operator/(const Rational& lhs, const Rational& rhs) {
    Rational ans = lhs;
    ans /= rhs;
    return ans;
}

bool operator==(const Rational& lhs, const Rational& rhs) {
    return lhs.num == rhs.num;
}

bool operator!=(const Rational& lhs, const Rational& rhs) {
    return !(lhs == rhs);
}

bool operator<(const Rational& lhs, const Rational& rhs) {
    return lhs.num * rhs.den < rhs.num * lhs.den;
}

bool operator>(const Rational& lhs, const Rational& rhs) {
    return rhs < lhs;
}

bool operator<=(const Rational& lhs, const Rational& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const Rational& lhs, const Rational& rhs) {
    return !(lhs < rhs);
}

#endif
