#ifndef RESIDUE_H
#define RESIDUE_H

#include <vector>

using ull = unsigned long long;

template <bool v>
struct static_assert_f;

template <>
struct static_assert_f<true> {
    static const bool value = true;
};

template <ull a, ull b>
struct mid {
    static const ull value = (a + b) / 2u;
};

template <ull a, ull b>
const ull mid_v = mid<a, b>::value;

template <ull a>
struct sqr {
    static const ull value = a * a;
};

template <ull a>
const ull sqr_v = sqr<a>::value;

template <ull res, ull l = 1, ull r = res>
struct ct_sqrt;

template <ull res, ull r>
struct ct_sqrt<res, r, r> {
    static const ull value = r;
};

template <ull res, ull l, ull r>
struct ct_sqrt {
    static const ull value = ct_sqrt<res,
                                     (sqr_v<mid_v<r, l>> >= res ? l : mid_v<r, l> + 1),
                                     (sqr_v<mid_v<r, l>> >= res ? mid_v<r, l> : r)>::value;
};

template <ull N>
const ull ct_sqrt_v = ct_sqrt<N>::value;

template <>
const ull ct_sqrt_v<0> = 0;

template <>
const ull ct_sqrt_v<1> = 1;

#ifdef STRICT_NO_STL

template <ull a, ull b>
const ull max_v = a > b ? a : b;

template <ull a, ull b>
const ull min_v = a < b ? a : b;

#else

template <ull a, ull b>
const ull max_v = std::max(a, b);

template <ull a, ull b>
const ull min_v = std::min(a, b);

#endif

template <ull N>
struct to_odd {
    static const ull value = N - (N & 1) + 1;
};

template <ull N>
const ull to_odd_v = to_odd<N>::value;

template <ull N, ull K = to_odd_v<ct_sqrt_v<N>>>
struct less_divisor {
    static const ull value = min_v<N % K == 0 ? K : N, less_divisor<N, K - 2>::value>;
};

template <ull N>
struct less_divisor<N, 1> {
    static const ull value = N % 2 == 0 ? 2 : N;
};

template <ull N>
const ull less_divisor_v = less_divisor<N>::value;

#ifdef STRICT_NO_STL

template <ull N, ull K>
struct is_prime_function {
    static const bool value = (N % K || N == K) && is_prime_function<N, K - 2>::value;
};

template <ull N>
struct is_prime_function<N, 1> {
    static const bool value = true;
};

template <ull N>
struct is_prime_function<N, 0> {
    static const bool value = false;
};

template <ull N>
struct is_prime {
    static const bool value = (N == 2 || N % 2) && is_prime_function<N, to_odd_v<ct_sqrt_v<N>>>::value;
};

#else

template <ull N>
struct is_prime {
    static const bool value = less_divisor_v<N> == N;
};

#endif

template <>
struct is_prime<1> {
    static const bool value = false;
};

template<>
struct is_prime<2> {
    static const bool value = true;
};

template <ull N>
const bool is_prime_v = is_prime<N>::value;

template <ull N, ull K>
struct is_power_of_odd_prime {
    static const bool value = (N % K == 0) && is_power_of_odd_prime<N / K, K>::value;
};

template <ull K>
struct is_power_of_odd_prime<1, K> {
    static const bool value = is_prime_v<K> && K != 2;
};

template <ull K>
struct is_power_of_odd_prime<0, K> {
    static const bool value = false;
};

template <ull N, ull K>
const bool is_power_of_odd_prime_v = is_power_of_odd_prime<N, K>::value;

template <ull N>
struct is_power_odd_prime {
    static const bool value = (N % 2)
            && (is_prime_v<N> || is_power_of_odd_prime_v<N, less_divisor_v<N>>);
};

template <>
struct is_power_odd_prime<1> {
    static const bool value = false;
};

template <>
struct is_power_odd_prime<2> {
    static const bool value = false;
};

template <ull N>
const bool is_power_odd_prime_v = is_power_odd_prime<N>::value;

template <ull N>
struct has_primitive_root {
    static const bool value = is_power_odd_prime_v<N> || (N % 2 == 0 && is_power_odd_prime_v<N / 2>);
};

template<>
struct has_primitive_root<1> {
    static const bool value = false;
};

template <>
struct has_primitive_root<2> {
    static const bool value = true;
};

template <>
struct has_primitive_root<4> {
    static const bool value = true;
};

template <ull N>
const bool has_primitive_root_v = has_primitive_root<N>::value;

template <typename A, typename B>
struct same_type;

template <typename A>
struct same_type<A, A> {};

template <typename T>
T gcd(T a, T b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

template <unsigned N>
class Residue {
    ull value;

public:
    Residue(): value(0) {}

    explicit Residue(long long n) {
        long long mod = N;
        n %= mod;
        n += mod;
        n %= mod;
        value = n;
    }

    Residue<N>& operator=(long long v) {
        Residue<N> r(v);
        swap(r);
        return *this;
    }

    explicit operator int() const {
        return value;
    }

    void swap(Residue<N>& v) {
        std::swap(value, v.value);
    }

    Residue<N> pow(ull p) const {
        Residue<N> ans(1);
        Residue<N> a = *this;
        while (p) {
            if (p & 1) ans *= a;
            p /= 2;
            a *= a;
        }
        return ans;
    }

    Residue<N> getInverse() const {
        static_assert_f<is_prime_v<N>>();
        return Residue<N>(pow(N - 2));
    }

    Residue<N>& operator+=(const Residue<N>& that) {
        value += that.value;
        if (value >= N) value -= N;
        return *this;
    }

    Residue<N>& operator-=(const Residue<N>& that) {
        if (that.value == 0) return *this;
        value += N - that.value;
        if (value >= N) value -= N;
        return *this;
    }

    Residue<N>& operator*=(const Residue<N>& that) {
        value *= that.value;
        value %= N;
        return *this;
    }

    Residue<N>& operator/=(const Residue<N>& that) {
        *this *= that.getInverse();
        return *this;
    }

    ull order() const;

    // order but Residue
    Residue<N> reOrder() const;

    static Residue<N> getPrimitiveRoot();

    Residue<N>& operator++() {
        value += 1;
        if (value >= N) value -= N;
        return *this;
    }

    bool operator==(const Residue<N>& v) const {
        return value == v.value;
    }
};

std::vector<ull> divisors(ull n) {
    std::vector<ull> ans;
    ans.push_back(1);
    for (ull d = 2; d * d <= n; ++d) {
        if (n % d == 0) {
            n /= d;
            ans.push_back(d);
        }
    }
    if (n != 1) ans.push_back(n);
    return ans;
}

unsigned phi(ull n) {
    std::vector<ull> d = divisors(n);
    if (d.size() == 1) return 0;
    ull ans = 1;
    for (size_t i = 1; i < d.size(); ++i) {
        if (d[i] == d[i - 1]) {
            ans *= d[i];
        } else {
            ans *= d[i] - 1;
        }
    }
    return ans;
}

template <unsigned N>
bool isPrimitiveRoot(Residue<N> v) {
    ull p = phi(N);
    if (v.pow(p) != 1) return false;
    auto div = divisors(p);
    for (auto& d : div) {
        if (d == 1) continue;
        if (v.pow(p / d) == 1) return false;
    }
    return true;
}

template <unsigned N>
Residue<N> Residue<N>::getPrimitiveRoot() {
    static_assert_f<has_primitive_root_v<N>>();
    static Residue<N> answer(0);
    if (answer != 0) return answer;
    for (answer = Residue<N>(2); answer != N; ++answer) {
        if (isPrimitiveRoot(answer)) return answer;
    }
    throw std::runtime_error("internal error\nplease submit a full bug report to\n\texample@example.com");
}

template <>
Residue<2> Residue<2>::getPrimitiveRoot() {
    return Residue<2>(1);
}

template <>
Residue<4> Residue<4>::getPrimitiveRoot() {
    return Residue<4>(3);
}

template <unsigned N>
ull Residue<N>::order() const {
    if (gcd<ull>(value, N) != 1) return 0;
    ull p = phi(N);
    ull ans = p;
    for (ull d = 2; d * d <= 2 * ans; ++d) {
        while (ans % d == 0) {
            if (pow(ans / d) == 1) ans /= d;
            else break;
        }
    }
    return ans;
}

template <unsigned N>
Residue<N> Residue<N>::reOrder() const {
    return Residue<N>(order());
}

template <unsigned N>
Residue<N> operator+(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> ans = a;
    ans += b;
    return ans;
}

template <unsigned N>
Residue<N> operator-(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> ans = a;
    ans -= b;
    return ans;
}

template <unsigned N>
Residue<N> operator*(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> ans = a;
    ans *= b;
    return ans;
}

template <unsigned N>
Residue<N> operator/(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> ans = a;
    ans /= b;
    return ans;
}

template <unsigned N>
bool operator!=(const Residue<N>& a, const Residue<N>& b) {
    return !(a == b);
}

template <unsigned N>
bool operator==(const Residue<N>& a, int b) {
    return a == static_cast<Residue<N>>(b);
}

template <unsigned N>
bool operator==(int b, const Residue<N>& a) {
    return a == static_cast<Residue<N>>(b);
}

template <unsigned N>
bool operator!=(const Residue<N>& a, int b) {
    return a != static_cast<Residue<N>>(b);
}

template <unsigned N>
bool operator!=(int b, const Residue<N>& a) {
    return a != static_cast<Residue<N>>(b);
}

template <unsigned N>
std::ostream& operator<<(std::ostream& out, const Residue<N>& v) {
    out << static_cast<int>(v);
    return out;
}

template <unsigned N>
std::istream& operator>>(std::istream& in, Residue<N>& v) {
    long long x;
    in >> x;
    v = x;
    return in;
}

#endif
