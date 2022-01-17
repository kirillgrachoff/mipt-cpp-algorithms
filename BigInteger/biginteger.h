#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

namespace BigIntegerHelpers {

void delete_zeros(std::vector<int>& a) {
    while (!a.empty() && a.back() == 0) a.pop_back();
}

void to_base_10(std::vector<int>& a) {
    int accumulator = 0;
    for (size_t idx = 0; accumulator || idx < a.size(); ++idx) {
        if (idx >= a.size()) a.push_back(0);
        accumulator += a[idx];
        a[idx] = accumulator % 10;
        accumulator /= 10;
    }
}

void sub_from(std::vector<int>& a, const std::vector<int>& b, size_t pos) {
    for (size_t i = 0; i + pos < a.size(); ++i) {
        if (i < b.size()) a[i + pos] -= b[i];
        if (a[i + pos] < 0) {
            a[i + pos] += 10;
            --a[i + pos + 1];
        }
    }
}

void abs_add(std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() < b.size()) a.resize(b.size(), 0);
    for (size_t i = 0; i < b.size(); ++i) a[i] += b[i];
    to_base_10(a);
    delete_zeros(a);
}

void abs_sub(std::vector<int>& a, const std::vector<int>& b) {
    sub_from(a, b, 0);
    delete_zeros(a);
}

int compare_to(const std::vector<int>& a, const std::vector<int>& b, size_t pos) {
    if (a.size() < b.size() + pos) return -1;
    if (a.size() > b.size() + pos) return 1;
    for (size_t i = b.size(); i--;) {
        if (a[i + pos] < b[i]) return -1;
        if (a[i + pos] > b[i]) return 1;
    }
    return 0;
}

std::vector<int> divide(std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> ans(a.size(), 0);
    for (size_t i = ans.size(); i--;) {
        for (int j = 0; j < 9; ++j) {
            delete_zeros(a);
            if (BigIntegerHelpers::compare_to(a, b, i) >= 0) {
                BigIntegerHelpers::sub_from(a, b, i);
                ++ans[i];
            } else break;
        }
    }
    return ans;
}

void normalize(std::vector<int>& a) {
    to_base_10(a);
    delete_zeros(a);
}
}

namespace fft {
using std::vector;
constexpr int base = 998244353;
constexpr int forward_root = 15311432;
constexpr int max_len = 1 << 23;

int mul(int a, int b) {
    return static_cast<int>(
            static_cast<long long>(a) * static_cast<long long>(b) % base
            );
}

int add(int a, int b) {
    a += b;
    if (a >= base) a -= base;
    if (a < 0) a += base;
    return a;
}

int mpow(int a, int p) {
    int ans = 1;
    while (p) {
        if (p & 1) ans = mul(ans, a);
        p /= 2;
        a = mul(a, a);
    }
    return ans;
}

int rev(int n) {
    return mpow(n, base - 2);
}

int rev_bits(int n, int w) {
    int ans = 0;
    while (w--) {
        ans *= 2;
        ans += n % 2;
        n /= 2;
    }
    return ans;
}

void fft_common(vector<int>& a, int root) {
    using std::swap;
    int n = a.size();
    int k = 1;
    while ((1 << k) < n) ++k;
    for (int i = 1; i < n; ++i) {
        int ri = rev_bits(i, k);
        if (i < ri) {
            swap(a[i], a[ri]);
        }
    }
    for (int len = 2; len <= n; len *= 2) {
        int wlen = root;
        for (int i = len; i < max_len; i *= 2) wlen = mul(wlen, wlen); // w_n -> w_{n/2}
        for (int i = 0; i < n; i += len) { // n / len
            int w = 1;
            for (int j = 0; j < len / 2; ++j, w = mul(w, wlen)) {
                int u = a[i + j];
                int v = a[i + j + len / 2];
                v = mul(v, w);
                a[i + j] = add(u, v);
                a[i + j + len / 2] = add(u, -v);
            }
        }
    }
}

void to_pow_2(vector<int>& a) {
    if (__builtin_popcountll(a.size()) != 1) {
        size_t k = 1;
        while (k < a.size()) k *= 2;
        a.resize(k, 0);
    }
}

void fft(vector<int>& a) {
    to_pow_2(a);
    fft_common(a, forward_root);
}

void fft_rev(vector<int>& a) {
    to_pow_2(a);
    fft_common(a, rev(forward_root));
    int n_1 = rev(a.size());
    for (auto& it : a) it = mul(it, n_1);
}

void inplace_multiply(vector<int>& a, vector<int>& b) { /// b is 'constant'
    size_t n = 1;
    while (n < a.size() + b.size()) n *= 2;
    a.resize(n, 0);
    b.resize(n, 0);
    fft(a);
    fft(b);
    for (size_t i = 0; i < n; i++) {
        a[i] = mul(a[i], b[i]);
    }
    fft_rev(a);
    fft_rev(b);
    BigIntegerHelpers::normalize(a);
    BigIntegerHelpers::delete_zeros(b);
}
}

class BigInteger {
    mutable std::vector<int> num_;
    mutable int sign_ = 1;

public:

    BigInteger() = default;

    BigInteger(int v) {
        if (v < 0) {
            sign_ = -1;
            v *= -1;
        }
        while (v > 0) {
            num_.push_back(v % 10);
            v /= 10;
        }
    }

    BigInteger(std::string s) { // s contains only '0'...'9' and '-'
        num_.reserve(s.size());
        reverse(s.begin(), s.end());
        if (s.back() == '-') {
            sign_ = -1;
            s.pop_back();
        }
        for (auto& it : s) {
            num_.push_back(it - '0');
        }
        BigIntegerHelpers::delete_zeros(num_);
        if (num_.empty()) sign_ = 1;
    }

    BigInteger(const BigInteger& that) {
        num_ = that.num_;
        sign_ = that.sign_;
    }

    int sign() const {
        return this->sign_;
    }

    void swap(BigInteger&);

    BigInteger& operator=(const BigInteger&) &;

    friend bool operator==(const BigInteger&, const BigInteger&);

    friend bool operator<(const BigInteger&, const BigInteger&);

    BigInteger& operator-=(const BigInteger&) &;

    BigInteger& operator+=(const BigInteger&) &;

    BigInteger& operator*=(const BigInteger&) &;

    BigInteger& operator/=(const BigInteger&) &;
    BigInteger& operator%=(const BigInteger&) &;

    BigInteger operator-() const;

    BigInteger& operator++() &;
    BigInteger operator++(int) &;

    BigInteger& operator--() &;
    BigInteger operator--(int) &;

    explicit operator std::string() const;

    std::string toString() const {
        return static_cast<std::string>(*this);
    }

    explicit operator bool() const;

    void mulPow10(int);

    explicit operator long long() {
        int ans = 0;
        for (auto it = num_.rbegin(); it != num_.rend(); ++it) {
            ans *= 10;
            ans += *it;
        }
        return ans * sign_;
    }
};

void BigInteger::mulPow10(int pw) {
    std::vector<int> nnum_(num_.size() + pw, 0);
    std::copy(num_.begin(), num_.end(), nnum_.begin() + pw);
    num_.swap(nnum_);
}

void BigInteger::swap(BigInteger& that) {
    using std::swap;
    swap(num_, that.num_);
    swap(sign_, that.sign_);
}

BigInteger& BigInteger::operator=(const BigInteger& that) & {
    BigInteger tmp = that;
    swap(tmp);
    return *this;
}

bool operator==(const BigInteger& x, const BigInteger& y) {
    const std::vector<int>& a = x.num_;
    const std::vector<int>& b = y.num_;
    if (a.empty()) {
        return b.empty();
    }
    if (x.sign() != y.sign()) return false;
    return a == b;
}

bool operator<(const BigInteger& x, const BigInteger& y) {
    if (x == y) return false;
    const std::vector<int>& a = x.num_;
    const std::vector<int>& b = y.num_;
    if (y.sign() < x.sign()) return false;
    if (x.sign() < y.sign()) return true;
    return BigIntegerHelpers::compare_to(a, b, 0) * x.sign() < 0;
}

BigInteger& BigInteger::operator-=(const BigInteger& that) & {
    if (*this == that) {
        sign_ = 1;
        num_.clear();
        return *this;
    }
    if (sign() == that.sign()) {
        int nsign_ = sign_;
        const int thatsign_ = that.sign_;
        sign_ = that.sign_ = 1;
        if (*this < that) {
            nsign_ *= -1;
            std::vector<int> tmp = that.num_;
            BigIntegerHelpers::abs_sub(tmp, num_);
            num_ = tmp;
        } else if (that < *this) {
            BigIntegerHelpers::abs_sub(num_, that.num_);
        } else {
            this->num_.clear();
            nsign_ = 1;
        }
        that.sign_ = thatsign_;
        sign_ = nsign_;
    } else {
        that.sign_ *= -1;
        *this += that;
        that.sign_ *= -1;
    }
    BigIntegerHelpers::delete_zeros(num_);
    return *this;
}

BigInteger& BigInteger::operator+=(const BigInteger& that) & {
    if (sign() == that.sign()) {
        BigIntegerHelpers::abs_add(num_, that.num_);
    } else {
        that.sign_ *= -1;
        *this -= that;
        that.sign_ *= -1;
    }
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& that) & {
    if (*this == 0) return *this;
    if (that == 0) {
        num_.clear();
        sign_ = 1;
        return *this;
    }
    sign_ *= that.sign_;
    fft::inplace_multiply(this->num_, that.num_);
    BigIntegerHelpers::delete_zeros(num_);
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& that) & {
    if (that == 0) {
        throw std::runtime_error("division by zero");
    }
    num_ = BigIntegerHelpers::divide(num_, that.num_);
    BigIntegerHelpers::delete_zeros(num_);
    sign_ *= that.sign_;
    if (num_.empty()) sign_ = 1;
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& that) & {
    if (that == 0) {
        throw std::runtime_error("division by zero");
    }
    sign_ *= that.sign_;
    BigIntegerHelpers::divide(num_, that.num_);
    if (num_.empty()) {
        sign_ = 1;
    }
    return *this;
}

BigInteger BigInteger::operator-() const {
    BigInteger ans = *this;
    if (ans) ans.sign_ *= -1;
    return ans;
}

BigInteger::operator std::string() const {
    std::string ans;
    for (auto& it : num_) {
        ans += '0' + it;
    }
    if (ans.empty()) ans += '0';
    else if (sign() < 0) ans += '-';
    std::reverse(ans.begin(), ans.end());
    return ans;
}

BigInteger::operator bool() const {
    return !num_.empty();
}

BigInteger operator*(const BigInteger& lhs, const BigInteger& rhs) {
    BigInteger ans = lhs;
    ans *= rhs;
    return ans;
}

BigInteger operator+(const BigInteger& lhs, const BigInteger& rhs) {
    BigInteger ans = lhs;
    ans += rhs;
    return ans;
}

BigInteger operator-(const BigInteger& lhs, const BigInteger& rhs) {
    BigInteger ans = lhs;
    ans -= rhs;
    return ans;
}

BigInteger operator/(const BigInteger& lhs, const BigInteger& rhs) {
    BigInteger ans = lhs;
    ans /= rhs;
    return ans;
}

BigInteger operator%(const BigInteger& lhs, const BigInteger& rhs) {
    BigInteger ans = lhs;
    ans %= rhs;
    return ans;
}

BigInteger& BigInteger::operator++() & {
    *this += 1;
    return *this;
}

BigInteger BigInteger::operator++(int) & {
    BigInteger ans = *this;
    ++(*this);
    return ans;
}

BigInteger& BigInteger::operator--() & {
    *this -= 1;
    return *this;
}

BigInteger BigInteger::operator--(int) & {
    BigInteger ans = *this;
    --(*this);
    return ans;
}

BigInteger operator""_bi(const char* s) {
    return BigInteger(static_cast<std::string>(s));
}

bool operator!=(const BigInteger& lhs, const BigInteger& rhs) {
    return !(lhs == rhs);
}

bool operator>(const BigInteger& lhs, const BigInteger& rhs) {
    return rhs < lhs;
}

bool operator<=(const BigInteger& lhs, const BigInteger& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const BigInteger& lhs, const BigInteger& rhs) {
    return !(lhs < rhs);
}

std::ostream& operator<<(std::ostream& out, const BigInteger& v) {
    out << static_cast<std::string>(v);
    return out;
}

std::istream& operator>>(std::istream& in, BigInteger& a) {
    std::string v;
    in >> v;
    a = BigInteger(v);
    return in;
}

#endif

