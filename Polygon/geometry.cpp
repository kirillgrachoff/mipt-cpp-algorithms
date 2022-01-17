#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

/// BTW this is the only necessary header
#include <ext/pb_ds/assoc_container.hpp>

template <typename K, typename V = __gnu_pbds::null_type, typename Comp = std::less<K>>
using indexed_tree = __gnu_pbds::tree<
        K, V, Comp, __gnu_pbds::rb_tree_tag, __gnu_pbds::tree_order_statistics_node_update>;

template <typename T, typename Comp = std::less<T>>
using indexed_set = indexed_tree<T, __gnu_pbds::null_type, Comp>;

template <typename K, typename V, typename Comp = std::less<K>>
using indexed_map = indexed_tree<K, V, Comp>;

template <typename T>
class ScanLine;

template <typename T>
class Point {
private:
    T x_ = 0;
    T y_ = 0;

public:
    Point() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
    Point(const Point&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
    Point(Point&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
    Point& operator=(const Point&) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
    Point& operator=(Point&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

    Point(const T& x, const T& y) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : x_(x), y_(y) {}

    T dot(const Point& p) const {
        return x_ * p.x_ + y_ * p.y_;
    }

    T cross(const Point& p) const {
        return x_ * p.y_ - p.x_ * y_;
    }

    T x() const {
        return x_;
    }

    T y() const {
        return y_;
    }

    Point& operator+=(const Point& p) & {
        x_ += p.x_;
        y_ += p.y_;
        return *this;
    }

    Point& operator-=(const Point& p) & {
        x_ -= p.x_;
        y_ -= p.y_;
        return *this;
    }

    Point& operator*=(const T& k) & {
        x_ *= k;
        y_ *= k;
    }

    Point operator-() const {
        return Point(-x_, -y_);
    }

    friend std::istream& operator>>(std::istream& in, Point<T>& p) {
        in >> p.x_ >> p.y_;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Point<T>& p) {
        out << p.x_ << ' ' << p.y_;
        return out;
    }

    T metrics() const {
        return std::abs(x_) + std::abs(y_);
    }

    template <typename CompX, typename CompY>
    static auto comporator(CompX comp_x, CompY comp_y) {
        return [comp_x, comp_y](const Point<T>& a, const Point<T>& b) {
            if (a.x() != b.x()) {
                return comp_x(a.x(), b.x());
            } else {
                return comp_y(a.y(), b.y());
            }
        };
    }
};

template <typename T>
Point<T> operator*(const Point<T>& x, const T& k) {
    Point<T> tmp(x);
    tmp *= k;
    return tmp;
}

template <typename T>
Point<T> operator*(const T& k, const Point<T>& x) {
    return x * k;
}

template <typename T>
Point<T> operator+(const Point<T>& a, const Point<T>& b) {
    Point<T> tmp(a);
    tmp += b;
    return tmp;
}

template <typename T>
Point<T> operator-(const Point<T>& a, const Point<T>& b) {
    Point<T> tmp(a);
    tmp -= b;
    return tmp;
}

template <typename T>
bool operator==(const Point<T>& a, const Point<T>& b) {
    return a.x() == b.x() && a.y() == b.y();
}

template <typename T>
bool operator!=(const Point<T>& a, const Point<T>& b) {
    return !(a == b);
}

template <typename T>
bool operator<(const Point<T>& a, const Point<T>& b) {
    if (a.x() != b.x()) {
        return a.x() < b.x();
    } else {
        return a.y() < b.y();
    }
}

template <typename T>
bool operator>(const Point<T>& a, const Point<T>& b) {
    return b < a;
}

template <typename T>
class Sector {
private:
    Point<T> base_;
    Point<T> direction_;

public:
    Sector(const Point<T>& base, const Point<T>& direction) noexcept(std::is_nothrow_copy_constructible_v<Point<T>>)
            : base_(base), direction_(direction) {}

    [[nodiscard]] T cross(Point<T> p) const {
        p -= base_;
        return direction_.cross(p);
    }

    [[nodiscard]] T dot(Point<T> p) const {
        p -= base_;
        return direction_.dot(p);
    }

    static Sector<T> from_pair(const Point<T>& a, const Point<T>& b) {
        return Sector(a, b - a);
    }

    static Sector<T> left_to_right(Point<T> a, Point<T> b) {
        using std::swap;
        if (a > b) swap(a, b);
        return Sector(a, b - a);
    }

    Point<T> begin() const {
        return base_;
    }

    Point<T> end() const {
        return base_ + direction_;
    }

    // necessary for PBDS
    friend std::ostream& operator<<(std::ostream& out, const Sector<T>& s) {
        out << "{{" << s.base_ << "} -> " << s.direction_ << "}";
        return out;
    }

    class Lower {
    public:
        bool operator()(const Sector<T>& a, const Sector<T>& b) const {
            if (a.direction_ == b.direction_ && a.base_ == b.base_) return false;

            if (a.direction_ == Point<T>(0, 0)) {
                if (b.cross(a.base_) == 0) return false;
                return !compare_sector_with_point(b, a.base_);
            }
            if (b.direction_ == Point<T>(0, 0)) {
                if (a.cross(b.base_) == 0) return false;
                return compare_sector_with_point(a, b.base_);
            }

            if (a.end() == b.begin() || a.begin() == b.end()) {
                return compare_intersected_sectors(a, b);
            }

            return compare_sectors(a, b);
        }

    private:
        [[nodiscard]] bool compare_sectors(const Sector<T>& a, const Sector<T>& b) const {
            if (a.base_.x() >= b.base_.x()) {
                if (b.cross(a.base_) != 0) return b.cross(a.base_) < 0;
            } else {
                if (a.cross(b.base_) != 0) return a.cross(b.base_) > 0;
            }
            return a.direction_.cross(b.direction_) > 0;
        }

        [[nodiscard]] bool compare_intersected_sectors(const Sector<T>& a, const Sector<T>& b) const {
            return a.end() == b.begin();
        }

        [[nodiscard]] bool compare_sector_with_point(const Sector<T>& a, const Point<T>& p) const {
            if (a.end() == p) return true;
            if (a.begin() == p) return false;
            return a.cross(p) > 0;
        }
    };
};

enum class Verdict {
    border, inside, outside
};

std::ostream& operator<<(std::ostream& out, const Verdict& verdict) {
    switch (verdict) {
        case Verdict::inside:
            out << "INSIDE";
            break;
        case Verdict::border:
            out << "BORDER";
            break;
        case Verdict::outside:
            out << "OUTSIDE";
            break;
    }
    return out;
}

// Visitor Pattern
template <typename T>
class Event {
protected:
    const Point<T> event_point;
    const size_t priority;

public:
    Event(const Point<T>& event_point, size_t priority) : event_point(event_point), priority(priority) {}
    virtual void process(ScanLine<T>& scanline) = 0;
    virtual ~Event() = default;

    friend bool operator<(const Event<T>& a, const Event<T>& b) {
        if (a.event_point != b.event_point) {
            return a.event_point < b.event_point;
        } else {
            return a.priority < b.priority;
        }
    }
};

template <typename T>
class AddSector : public Event<T> {
private:
    Sector<T> sector;

public:
    AddSector(const Sector<T>& sector) : Event<T>(sector.begin(), /*priority=*/0), sector(sector) {}

    void process(ScanLine<T>& scan_line) final override {
        assert(scan_line.insert_sector(sector));
    }

    ~AddSector() override = default;
};

template <typename T>
class EraseSector : public Event<T> {
private:
    Sector<T> sector;

public:
    EraseSector(const Sector<T>& sector) : Event<T>(sector.end(), /*priority=*/2), sector(sector) {}

    void process(ScanLine<T>& scan_line) final override {
        assert(scan_line.erase_sector(sector));
    }

    ~EraseSector() override = default;
};

template <typename T>
class WherePoint : public Event<T> {
    Verdict verdict;
    size_t index;

public:
    WherePoint(const Point<T>& point, size_t index) : Event<T>(point, /*priority=*/1), index(index) {}

    void process(ScanLine<T>& scan_line) final override {
        verdict = scan_line.where(this->event_point);
    }

    std::pair<size_t, Verdict> get_verdict() const {
        return {index, verdict};
    }

    ~WherePoint() override = default;
};

template <typename T>
class ScanLine {
private:
    indexed_set<Sector<T>, typename Sector<T>::Lower> line;

public:
    bool insert_sector(const Sector<T>& sector) {
        auto [iter, ok] = line.insert(sector);
        return ok;
    }

    bool erase_sector(const Sector<T>& sector) {
        return line.erase(sector);
    }

    bool check_border(typename indexed_set<Sector<T>, typename Sector<T>::Lower>::iterator point) {
        if (point != line.begin()) {
            auto prev = std::prev(point);
            if (prev->cross(point->begin()) == 0) return true;
        }
        auto next = std::next(point);
        if (next == line.end()) return false;
        return next->cross(point->begin()) == 0;
    }

    Verdict where(const Point<T>& p) {
        Sector<T> point_sector(p, Point<T>(0, 0));
        auto [iter, ok] = line.insert(point_sector);
        if (!ok) return Verdict::border;
        if (check_border(iter)) {
            line.erase(iter);
            return Verdict::border;
        }
        size_t index = line.order_of_key(point_sector);
        line.erase(iter);
        if (index & 1) {
            return Verdict::inside;
        } else {
            return Verdict::outside;
        }
    }

    void apply(Event<T>& action) {
        action.process(*this);
    }
};

using ll = long long;

std::vector<Point<ll>> input_figure() {
    ll n;
    std::cin >> n;
    std::vector<Point<ll>> a;
    for (ll i = 0; i < n; ++i) {
        Point<ll> v;
        std::cin >> v;
        if (!a.empty() && a.back() == v) continue;
        a.push_back(v);
    }
    return a;
}

std::vector<Point<ll>> input_ask() {
    ll k;
    std::cin >> k;
    std::vector<Point<ll>> ask(k);
    for (auto& it : ask) std::cin >> it;
    return ask;
}

void convert_figure_to_events(std::vector<std::shared_ptr<Event<ll>>>& events, const std::vector<Point<ll>>& a) {
    for (size_t i = 0; i < a.size(); ++i) {
        size_t j = i + 1;
        if (j == a.size()) j = 0;
        Sector<ll> border = Sector<ll>::left_to_right(a[i], a[j]);
        events.emplace_back(std::make_shared<AddSector<ll>>(border));
        events.emplace_back(std::make_shared<EraseSector<ll>>(border));
    }
}

void convert_request_to_events(std::vector<std::shared_ptr<Event<ll>>>& events, const std::vector<Point<ll>>& ask) {
    for (size_t i = 0; i < ask.size(); ++i) {
        events.emplace_back(std::make_shared<WherePoint<ll>>(ask[i], i));
    }
}

void solve() {
    // shared_ptr because of std::dynamic_pointer_cast
    std::vector<std::shared_ptr<Event<ll>>> events;

    std::vector<Point<ll>> a = input_figure();
    convert_figure_to_events(events, a);

    std::vector<Point<ll>> ask = input_ask();
    convert_request_to_events(events, ask);

    std::sort(events.begin(), events.end(), [](const auto& a, const auto& b) {
        return *a < *b;
    });

    ScanLine<ll> scan_line;
    for (auto& it : events) {
        scan_line.apply(*it);
    }

    std::vector<Verdict> answer(ask.size());
    for (auto& it : events) {
        auto answer_ptr = std::dynamic_pointer_cast<WherePoint<ll>>(it);
        if (!answer_ptr) continue;
        auto [index, verdict] = answer_ptr->get_verdict();
        answer[index] = verdict;
    }
    for (auto& it : answer) {
        std::cout << it << '\n';
    }
}

int main() {
#ifdef LOCAL
    freopen("z_in.txt", "r", stdin);
#endif
    ll t;
    std::cin >> t;
    while (t--) {
        solve();
        std::cout << '\n';
    }
}
