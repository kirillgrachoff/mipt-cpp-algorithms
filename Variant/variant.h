#pragma once

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

class bad_variant_access: public std::runtime_error {
public:
    bad_variant_access(): std::runtime_error("bad variant access") {}
    bad_variant_access(const char* str): std::runtime_error(str) {}
};

namespace detail {
template <size_t ans, typename T, typename Head, typename... Tail>
struct position {
    static const size_t value = std::is_same_v<T, Head> ? ans : position<ans + 1, T, Tail...>::value;
};

template <size_t ans, typename T, typename Head>
struct position<ans, T, Head> {
    static const size_t value = std::is_same_v<T, Head> ? ans : ans + 1;
};

template <size_t index, typename Head, typename... Types>
struct type_position {
    using type = typename type_position<index - 1, Types...>::type;
};

template <typename Head, typename... Types>
struct type_position<0, Head, Types...> {
    using type = Head;
};

template <typename T, typename... Types>
struct type_position<static_cast<size_t>(-1), T, Types...> {
    using type = void;
};

template <typename T, typename Head, typename... Args>
struct in_typelist {
    static const bool value = std::is_same_v<T, Head> || in_typelist<T, Args...>::value;
};

template <typename T, typename Head>
struct in_typelist<T, Head> {
    static const bool value = std::is_same_v<T, Head>;
};
}

template <typename T, typename... Types>
const size_t pos_v = detail::position<0, T, Types...>::value;

template <size_t index, typename... Types>
using pos_t = typename detail::type_position<index, Types...>::type;

template <typename... Types>
using head_t = pos_t<0, Types...>;

template <typename T, typename... Types>
const bool in_typelist_v = detail::in_typelist<T, Types...>::value;

namespace detail_best_match {
template <typename T, typename Head, typename... Tail>
struct count_constructible {
    static const size_t value = std::is_constructible_v<Head, T> + count_constructible<T, Tail...>::value;
};

template <typename T, typename Head>
struct count_constructible<T, Head> {
    static const size_t value = std::is_constructible_v<Head, T>;
};

template <typename T, typename... Types>
const size_t count_constructible_v = count_constructible<T, Types...>::value;

template <typename T, typename Head, typename... Tail>
struct first_constructible {
    static const size_t value = std::is_constructible_v<Head, T> ? 0 : 1 + first_constructible<T, Tail...>::value;
};

template <typename T, typename Head>
struct first_constructible<T, Head> {
    static const size_t value = 0;
};

template <typename T, typename... Types>
const size_t first_constructible_v =
        std::is_constructible_v<pos_t<first_constructible<T, Types...>::value, Types...>, T> ?
                first_constructible<T, Types...>::value : static_cast<size_t>(-1);

template <typename T, typename Head, typename... Tail>
struct count_assignable {
    static const size_t value = std::is_assignable_v<Head&, T> + count_assignable<T, Tail...>::value;
};

template <typename T, typename Head>
struct count_assignable<T, Head> {
    static const size_t value = std::is_assignable_v<Head&, T>;
};

template <typename T, typename... Types>
const size_t count_assignable_v = count_assignable<T, Types...>::value;

template <typename T, typename Head, typename... Tail>
struct first_assignable {
    static const size_t value = std::is_assignable_v<Head&, T> ? 0 : 1 + first_assignable<T, Tail...>::value;
};

template <typename T, typename Head>
struct first_assignable<T, Head> {
    static const size_t value = 0;
};

template <typename T, typename... Types>
const size_t first_assignable_v =
        std::is_assignable_v<pos_t<first_assignable<T, Types...>::value, Types...>& , T> ?
        first_assignable<T, Types...>::value : static_cast<size_t>(-1);

template <typename T, typename Head, typename... Tail>
struct count_assignable_rc {
    static const size_t value = std::is_assignable_v<std::remove_const_t<Head>&, T> + count_assignable_rc<T, Tail...>::value;
};

template <typename T, typename Head>
struct count_assignable_rc<T, Head> {
    static const size_t value = std::is_assignable_v<std::remove_const_t<Head>&, T>;
};

template <typename T, typename... Types>
const size_t count_assignable_rc_v = count_assignable_rc<T, Types...>::value;

template <typename T, typename Head, typename... Tail>
struct first_assignable_rc {
    static const size_t value = std::is_assignable_v<std::remove_const_t<Head>&, T> ? 0 : 1 + first_assignable_rc<T, Tail...>::value;
};

template <typename T, typename Head>
struct first_assignable_rc<T, Head> {
    static const size_t value = 0;
};

template <typename T, typename... Types>
const size_t first_assignable_rc_v =
        std::is_assignable_v<std::remove_const<pos_t<first_constructible<T, Types...>::value, Types...>>&, T> ?
        first_assignable_rc<T, Types...>::value : static_cast<size_t>(-1);


template <typename T, typename... Types>
struct best_match_assign {
    using type = pos_t<count_assignable_v<T, Types...> == 1 && count_assignable_rc_v<T, Types...> == 1 ? first_assignable_v<T, Types...> : -1, Types...>;
};

template <typename T, typename... Types>
struct best_match_construct {
    using type = pos_t<count_constructible_v<T, Types...> == 1 ? first_constructible_v<T, Types...> : -1, Types...>;
};
}

template <typename T, typename... Types>
using best_match_assign_t = typename detail_best_match::template best_match_assign<T, Types...>::type;

template <typename T, typename... Types>
using best_match_construct_t = typename detail_best_match::template best_match_construct<T, Types...>::type;


template <typename T>
decltype(auto) reinterpret_ptr(const char* data) {
    return reinterpret_cast<const T*>(data);
}

template <typename T>
decltype(auto) reinterpret_ref(const char* data) {
    return *reinterpret_ptr<T>(data);
}

template <typename T>
decltype(auto) reinterpret_ptr(char* data) {
    return reinterpret_cast<T*>(data);
}

template <typename T>
decltype(auto) reinterpret_ref(char* data) {
    return *reinterpret_ptr<T>(data);
}


template <typename... Types>
class Variant;

template <typename T, typename... Types>
T& get(Variant<Types...>& value);

template <typename T, typename... Types>
const T& get(const Variant<Types...>& value);

template <typename T, typename... Types>
T&& get(Variant<Types...>&& value);

template <typename T, typename... Types>
bool holds_alternative(const Variant<Types...>& value);


template <typename T, typename... Types>
class VariantAlternative {
    using Derived = Variant<Types...>;
    constexpr static size_t position = pos_v<T, Types...>;

    Variant<Types...>* thisPtr() {
        return static_cast<Variant<Types...>*>(this);
    }

    void make_valueless() {
        thisPtr()->number = Variant<Types...>::npos;
    }

public:
    VariantAlternative() {
        if constexpr (position == 0) {
            construct();
        }
    }

    VariantAlternative(const T& value) {
        construct(value);
    }

    VariantAlternative(T&& value) {
        construct(std::move(value));
    }

    VariantAlternative(const Variant<Types...>& value) {
        if (position == value.index()) {
            construct(reinterpret_ref<T>(value.data));
        }
    }

    VariantAlternative(Variant<Types...>&& value) {
        if (position == value.index()) {
            construct(std::move(reinterpret_ref<T>(value.data)));
        }
    }

    template <typename U = T, typename = std::enable_if_t<std::is_assignable_v<T&, const U&>>>
    T& operator=(const T& value) try {
        assign_or_construct(value);
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    template <typename U = T, typename = std::enable_if_t<std::is_assignable_v<T&, U&&>>>
    T& operator=(T&& value) try {
        assign_or_construct(std::move(value));
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    T& operator=(const Derived& value) {
        if (!holds_alternative<T>(value)) return reinterpret_ref<T>(thisPtr()->data);
        assign_or_construct(get<T>(value));
        return get<T>(*thisPtr());
    }

    T& operator=(Derived&& value) {
        if (!holds_alternative<T>(value)) return reinterpret_ref<T>(thisPtr()->data);
        assign_or_construct(std::move(get<T>(value)));
        return get<T>(*thisPtr());
    }

    void destroy() {
        if (holds_alternative<T>(*thisPtr())) {
            get<T>(*thisPtr()).~T();
            thisPtr()->number = Variant<Types...>::npos;
        }
    }

    template <typename... Args>
    T& construct(Args&&... args) try {
        new (std::launder(thisPtr()->data)) T(std::forward<Args>(args)...);
        thisPtr()->number = position;
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    template <typename U, typename... Args>
    T& construct(std::initializer_list<U> li, Args&&... args) try {
        new (std::launder(thisPtr()->data)) T(li, std::forward<Args>(args)...);
        thisPtr()->number = position;
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    template <typename TT>
    T& assign(TT&& value) try {
        get<T>(*thisPtr()) = std::forward<TT>(value);
        thisPtr()->number = position;
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    template <typename TT>
    T& assign_or_construct(TT&& value) {
        if (holds_alternative<T>(*thisPtr())) {
            assign(std::forward<TT>(value));
        } else {
            thisPtr()->destroy();
            construct(std::forward<TT>(value));
        }
        return get<T>(*thisPtr());
    }

    template <typename... Args>
    T& emplace(Args&&... args) try {
        thisPtr()->destroy();
        construct(std::forward<Args>(args)...);
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }

    template <typename U, typename... Args>
    T& emplace(std::initializer_list<U> li, Args&&... args) try {
        thisPtr()->destroy();
        construct(li, std::forward<Args>(args)...);
        return get<T>(*thisPtr());
    } catch (...) {
        make_valueless();
        throw;
    }
};

namespace EnableSpecialFunctions {
template <bool defaultConstructible>
struct DefaultConstructor {};
template <>
struct DefaultConstructor<false> {
    DefaultConstructor() = delete;
};

template <bool copyConstructible>
struct CopyConstructor {};
template <>
struct CopyConstructor<false> {
    CopyConstructor(const CopyConstructor&) = delete;
    CopyConstructor(CopyConstructor&) = delete;
};

template <bool moveConstructible>
struct MoveConstructor {};
template <>
struct MoveConstructor<false> {
    MoveConstructor(MoveConstructor&&) = delete;
    MoveConstructor(const MoveConstructor&&) = delete;
};

template <bool copyAssignable>
struct CopyAssignment {};
template <>
struct CopyAssignment<false> {
    CopyAssignment& operator=(const CopyAssignment&) = delete;
    CopyAssignment& operator=(CopyAssignment&) = delete;
};

template <bool moveAssignable>
struct MoveAssignment {};
template <>
struct MoveAssignment<false> {
    MoveAssignment& operator=(MoveAssignment&&) = delete;
    MoveAssignment& operator=(const MoveAssignment&&) = delete;
};
}

template <typename... Types>
class Variant
        : public VariantAlternative<Types, Types...>... {
    alignas(Types...) char data[std::max({sizeof(Types)...})];
    size_t number;

    Variant<Types...>* thisPtr() {
        return this;
    }

    template <typename TT, typename... TTypes>
    friend class VariantAlternative;

    template <typename TT, typename... TTypes>
    friend TT& get(Variant<TTypes...>& valueThat);

    template <typename TT, typename... TTypes>
    friend const TT& get(const Variant<TTypes...>& value);

    template <typename TT, typename... TTypes>
    friend TT&& get(Variant<TTypes...>&& valueThat);

    void destroy() {
        (::VariantAlternative<Types, Types...>::destroy(), ...);
        number = npos;
    }

public:
    using ::VariantAlternative<Types, Types...>::VariantAlternative...;
    using ::VariantAlternative<Types, Types...>::operator=...;

    Variant() = default;

    Variant(const Variant<Types...>& that): VariantAlternative<Types, Types...>(that)... {}

    Variant(Variant<Types...>&& that): VariantAlternative<Types, Types...>(std::move(that))... {}

    Variant& operator=(const Variant<Types...>& valueThat) {
        (::VariantAlternative<Types, Types...>::operator=(valueThat), ...);
        return *this;
    }

    Variant& operator=(Variant<Types...>&& valueThat) {
        (::VariantAlternative<Types, Types...>::operator=(std::move(valueThat)), ...);
        return *this;
    }

    template <
            typename T,
            typename Selected = best_match_construct_t<T, Types...>,
            typename = std::enable_if_t<!std::is_same_v<const Selected&, const T&> && std::is_constructible_v<Selected, T>>>
    Variant(T&& value) {
        ::VariantAlternative<Selected, Types...>::construct(std::forward<T>(value));
    }

    template <
            typename T,
            typename Selected = best_match_assign_t<T, Types...>,
            typename = std::enable_if_t<!std::is_same_v<const Selected&, const T&> && std::is_assignable_v<Selected&, T> && std::is_constructible_v<Selected, T>>>
    Variant& operator=(T&& value) {
        ::VariantAlternative<Selected, Types...>::assign_or_construct(std::forward<T>(value));
        return *this;
    }

    template <typename T, typename... Args>
    T& emplace(Args&&... args) try {
        static_assert(in_typelist_v<T, Types...>);
        ::VariantAlternative<T, Types...>::emplace(std::forward<Args>(args)...);
        return get<T>(*thisPtr());
    } catch (bad_variant_access&) {
        number = npos;
        throw;
    }

    template <typename T, typename U, typename... Args>
    T& emplace(std::initializer_list<U> list, Args&&... args) try {
        static_assert(in_typelist_v<T, Types...>);
        ::VariantAlternative<T, Types...>::emplace(list, std::forward<Args>(args)...);
        return get<T>(*thisPtr());
    } catch (bad_variant_access&) {
        number = npos;
        throw;
    }

    template <size_t I, typename... Args>
    auto emplace(Args&&... args) -> pos_t<I, Types...>& {
        static_assert(0 <= I && I < sizeof...(Types));
        return emplace<pos_t<I, Types...>>(std::forward<Args>(args)...);
    }

    size_t index() const noexcept {
        return number;
    }

    bool valueless_by_exception() const noexcept {
        return index() == npos;
    }

    constexpr static size_t npos = -1;

    ~Variant() {
        destroy();
    }
};

template <typename T, typename... Types>
bool holds_alternative(const Variant<Types...>& value) {
    static_assert(in_typelist_v<T, Types...>);
    return pos_v<T, Types...> == value.index();
}

template <typename T, typename... Types>
T& get(Variant<Types...>& value) {
    static_assert(in_typelist_v<T, Types...>);
    if (!holds_alternative<T>(value)) {
        throw bad_variant_access((std::string("value is ") + std::to_string(value.index())).c_str());
    }
    return reinterpret_ref<T>(value.data);
}

template <typename T, typename... Types>
const T& get(const Variant<Types...>& value) {
    static_assert(in_typelist_v<T, Types...>);
    if (!holds_alternative<T>(value)) {
        throw bad_variant_access((std::string("value is ") + std::to_string(value.index())).c_str());
    }
    return reinterpret_ref<T>(value.data);
}

template <typename T, typename... Types>
T&& get(Variant<Types...>&& value) {
    static_assert(in_typelist_v<T, Types...>);
    if (!holds_alternative<T>(value)) {
        throw bad_variant_access((std::string("value is ") + std::to_string(value.index())).c_str());
    }
    return std::move(reinterpret_ref<T>(value.data));
}

template <size_t I, typename... Types>
pos_t<I, Types...>& get(Variant<Types...>& value) {
    static_assert(0 <= I && I < sizeof...(Types));
    return get<pos_t<I, Types...>>(value);
}

template <size_t I, typename... Types>
const pos_t<I, Types...>& get(const Variant<Types...>& value) {
    static_assert(0 <= I && I < sizeof...(Types));
    return get<pos_t<I, Types...>>(value);
}

template <size_t I, typename... Types>
pos_t<I, Types...>&& get(Variant<Types...>&& value) {
    static_assert(0 <= I && I < sizeof...(Types));
    return std::move(get<pos_t<I, Types...>>(std::move(value)));
}
