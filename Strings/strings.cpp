#include <bits/stdc++.h>

using ll = long long;

constexpr size_t alphabet_size = 15;

template <typename T>
class IndexedValue {
private:
    T value_;
    size_t index_;
public:
    IndexedValue(const T& value, size_t index) : value_(value), index_(index) {}

    const T& value() const {
        return value_;
    }

    size_t index() const {
        return index_;
    }
};

template <typename T>
class RefrenOutput;

class Refren {
protected:
    size_t start_ = 0;
    size_t length_ = 0;
    size_t count_ = 0;

public:
    Refren() = default;
    Refren(size_t start, size_t len, size_t count) : start_(start), length_(len), count_(count) {}
    Refren(const Refren&) = default;
    Refren(Refren&&) = default;
    Refren& operator=(const Refren&) = default;
    Refren& operator=(Refren&&) = default;

    uint64_t metrics() const {
        return length_ * count_;
    }

    size_t length() const {
        return length_;
    }

    template <typename DTO>
    Refren(const DTO& answer) {
        auto [start, len, cnt] = answer;
        start_ = start;
        length_ = len;
        count_ = cnt;
    }

    template <typename T>
    RefrenOutput<T> from(const std::vector<T>& s) const {
        return RefrenOutput<T>(*this, s);
    }
};

template <typename T>
class RefrenOutput : public Refren {
private:
    std::vector<T> s;

    RefrenOutput(const Refren& refren, const std::vector<T>& s) : Refren(refren), s(s) {}
    friend class Refren;
public:

    RefrenOutput(const RefrenOutput<T>&) = default;
    RefrenOutput(RefrenOutput<T>&&) = default;

    RefrenOutput<T>& operator=(const RefrenOutput<T>&) = default;
    RefrenOutput<T>& operator=(RefrenOutput<T>&&) = default;


    friend std::ostream& operator<<(std::ostream& out, const RefrenOutput& r) {
        for (size_t i = 0; i < r.length_; ++i) {
            out << r.s[i + r.start_] << ' ';
        }
        return out;
    }
};

namespace suffix_array {
template <size_t alphabet_size, typename T>
std::vector<size_t> sort_cycle_shifts(const std::vector<T>& s) {
    const int32_t n = s.size();
    std::vector<int32_t> ans(n);
    std::vector<int32_t> nans(n);
    std::vector<int32_t> color;
    std::vector<int32_t> ncolor;
    std::vector<int32_t> cnt;
    cnt.resize(15, 0LL);
    for (auto& it : s) cnt[it]++;
    for (size_t i = 1; i < cnt.size(); ++i) cnt[i] += cnt[i - 1];
    for (size_t i = n; i--;) {
        ans[--cnt[s[i]]] = i;
    }
    color.assign(n, 0);
    cnt.assign(1, 1);
    color[ans[0]] = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[ans[i]] == s[ans[i - 1]]) {
            cnt.back()++;
            color[ans[i]] = color[ans[i - 1]];
        } else {
            color[ans[i]] = color[ans[i - 1]] + 1;
            cnt.push_back(cnt.back() + 1);
        }
    }
    for (int32_t len = 1; len < n; len *= 2) {
        for (size_t i = s.size(); i--;) {
            int32_t v = ans[i] - len;
            if (v < 0) v += n;
            nans[--cnt[color[v]]] = v;
        }
        ans.swap(nans);
        ncolor.assign(n, -1);
        ncolor[ans[0]] = 0;
        cnt.assign(1, 1);
        for (size_t i = 1; i < s.size(); ++i) {
            if (color[ans[i]] == color[ans[i - 1]] && color[(ans[i] + len) % n] == color[(ans[i - 1] + len) % n]) {
                ncolor[ans[i]] = ncolor[ans[i - 1]];
                cnt.back()++; // cnt[curr_color]++;
            } else {
                ncolor[ans[i]] = ncolor[ans[i - 1]] + 1;
                cnt.push_back(cnt.back() + 1);
            }
        }
        color.swap(ncolor);
    }
    std::vector<size_t> suffix_array;
    suffix_array.reserve(ans.size());
    for (auto it : ans) suffix_array.emplace_back(it);
    return suffix_array;
}

template <size_t alphabet_size, typename T>
std::vector<size_t> build_suffix_array(std::vector<T> s) {
    s.push_back(0);
    std::vector<size_t> answer = sort_cycle_shifts<alphabet_size>(s);
    answer.erase(answer.begin());
    return answer;
}

template <typename T>
std::vector<size_t> count_lcp(const std::vector<T>& str, const std::vector<size_t>& suff_array) {
    const size_t n = str.size();
    std::vector<size_t> pos(suff_array.size());
    for (size_t i = 0; i < str.size(); ++i) pos[suff_array[i]] = i;
    std::vector<size_t> lcp(n - 1, 0);
    size_t l0 = 1;
    for (size_t i = 0; i < str.size(); ++i) {

        if (pos[i] == n - 1) continue;
        l0 = std::max(l0, 1ul) - 1;
        size_t idx = pos[i];
        size_t l = suff_array[idx];
        size_t r = suff_array[idx + 1];
        l += l0;
        r += l0;
        while (l < str.size() && r < str.size()) {
            if (str[l] == str[r]) {
                ++l0;
                ++l;
                ++r;
            } else break;
        }
        lcp[idx] = l0;
    }
    return lcp;
}

std::vector<size_t> count_representations(std::vector<size_t> lcp_array) {
    std::vector<size_t> left(lcp_array.size(), 0);
    std::vector<size_t> right(lcp_array.size(), 0);
    std::stack<IndexedValue<size_t>> st;
    st.emplace(lcp_array.front(), 0);
    for (size_t i = 1; i < lcp_array.size(); ++i) {
        while (!st.empty() && st.top().value() >= lcp_array[i]) {
            st.pop();
        }
        if (st.empty()) {
            left[i] = i;
        } else {
            left[i] = i - st.top().index() - 1;
        }
        st.emplace(lcp_array[i], i);
    }
    while (!st.empty()) st.pop();

    /// here
    std::reverse(lcp_array.begin(), lcp_array.end());
    st.emplace(lcp_array.front(), 0);
    for (size_t i = 1; i < lcp_array.size(); ++i) {
        while (!st.empty()) {
            if (st.top().value() >= lcp_array[i]) st.pop();
            else break;
        }
        if (st.empty()) {
            right[i] = i;
        } else {
            right[i] = i - st.top().index() - 1;
        }
        st.push({lcp_array[i], i});
    }
    while (!st.empty()) {
        st.pop();
    }
    std::reverse(right.begin(), right.end());
    std::reverse(lcp_array.begin(), lcp_array.end());
    std::vector<size_t> ans(lcp_array.size(), 0);

    for (size_t i = 0; i < ans.size(); ++i) {
        ans[i] = left[i] + right[i] + 2; /// don't touch "+2"
    }

    return ans;
}

Refren get_refren(const std::vector<ll>& str) {
    if (str.size() == 1) {
        return {/*start=*/0, /*len=*/1, /*count=*/1};
    }
    std::vector<size_t> suffix_array = build_suffix_array<alphabet_size>(str);
    std::vector<size_t> lcp_array = count_lcp(str, suffix_array);
    std::vector<size_t> cnt = count_representations(lcp_array);

    std::vector<uint64_t> metrics(lcp_array.size());

    for (size_t i = 0; i < lcp_array.size(); ++i) {
        metrics[i] = uint64_t(cnt[i]) * lcp_array[i];
    }

    size_t idx = max_element(metrics.begin(), metrics.end()) - metrics.begin();

    if (metrics[idx] < str.size()) {
        return {/*start=*/0, /*len=*/str.size(), /*count=*/1};
    } else {
        return {/*start=*/suffix_array[idx], /*len=*/lcp_array[idx], /*count=*/cnt[idx]};
    }
}
}

namespace suffix_automaton {
template <typename TChar>
class SuffixAutomaton {
private:
    constexpr static size_t suff_npos = -1;

    struct Node {
        std::map<TChar, size_t> go;
        size_t suff;
        size_t len;
        size_t idx;
    };

    std::vector<TChar> s;
    int32_t last = 0;
    std::vector<Node> data{
            {/*go=*/{}, /*suff=*/suff_npos, /*len=*/0, /*idx=*/static_cast<size_t>(-1)}
    };

    size_t suff(size_t v) const {
        return data[v].suff;
    }

    size_t& suff(size_t v) {
        return data[v].suff;
    }

    size_t len(size_t v) const {
        return data[v].len;
    }

    size_t& len(size_t v) {
        return data[v].len;
    }

    bool can_go(size_t v, TChar c) const {
        return data[v].go.count(c);
    }

    size_t go(size_t v, TChar c) const {
        return data[v].go.at(c);
    }

    size_t& go(size_t v, TChar c) {
        return data[v].go[c];
    }

    size_t add_clone(size_t v) {
        size_t ans = data.size();
        data.push_back(data[v]);
        return ans;
    }

    size_t add_node() {
        size_t ans = data.size();
        data.push_back({/*go=*/{}, /*suff=*/suff_npos, /*len=*/s.size(), /*idx=*/s.size()});
        return ans;
    }

public:
    void add(ll c) {
        s.push_back(c);
        size_t nlast = add_node();

        size_t p;
        for (p = last; p != suff_npos && !can_go(p, c); p = suff(p)) {
            go(p, c) = nlast;
        }

        if (p == suff_npos) {
            suff(nlast) = 0;
            last = nlast;
            return;
        }
        size_t q = go(p, c);
        if (len(q) == len(p) + 1) { // q is `longest` in class of q ([q])
            suff(nlast) = q;
            last = nlast;
            return;
        } else { // divide [q] class into two classes
            size_t clone = add_clone(q);
            len(clone) = len(p) + 1; // BUG (fixed)
            suff(q) = clone;
            suff(nlast) = clone;
            for (; p != suff_npos && go(p, c) == q; p = suff(p)) {
                go(p, c) = clone;
            }
            last = nlast;
        }
    }

    bool is_substring(const std::vector<TChar>& str) const {
        size_t v = 0;
        for (auto it : s) {
            if (!can_go(v, it)) return false;
            v = go(v, it);
        }
        return true;
    }

    std::tuple<size_t, size_t, size_t> get_refren() { // pll { len, endIdx, count }
        std::vector<size_t> pathes(data.size(), 0);
        auto count = [&pathes](size_t v) {
            return pathes[v];
        };
        auto calc_count = [this, &pathes]() {
            ll p = last;
            for (; p > 0; p = suff(p)) {
                pathes[p] = 1;
            }
            std::vector<char> used(data.size(), 0);
            std::function<void(ll)> dfs = [&](ll v) {
                if (used[v]) return;
                used[v] = true;
                for (auto &[ch, to] : data[v].go) {
                    dfs(to);
                    pathes[v] += pathes[to];
                }
            };
            for (size_t i = 0; i < data.size(); ++i) {
                dfs(i);
            }
        };
        calc_count();
        size_t idx = 0;
        for (size_t i = 1; i < data.size(); ++i) {
            if (count(i) * len(i) > count(idx) * len(idx)) idx = i;
        }
        return {len(idx), data[idx].idx, count(idx)};
    }
};

Refren get_refren(const std::vector<ll>& str) {
    SuffixAutomaton<ll> automaton;
    for (auto it : str) {
        automaton.add(it);
    }
    auto [len, end_index, count] = automaton.get_refren();
    return Refren(/*start=*/end_index - len, len, count);
}
}

namespace suffix_tree {
template <size_t alphabet_size, typename TChar>
class SuffixTree {
private:
    constexpr static TChar empty_char = 0;

    struct Node {
        size_t depth = 0;
        size_t from = 0;
        size_t to = 0;
        std::map<TChar, std::shared_ptr<Node>> go;

        Node() = default;
        Node(size_t depth, size_t from, size_t to) : depth(depth), from(from), to(to) {}
    };
    std::vector<TChar> s;

    std::shared_ptr<Node> root_;

    static std::shared_ptr<Node> split_vertex_at_depth(size_t new_depth, const std::shared_ptr<Node>& vertex, const std::vector<TChar>& str) {
        auto nvertex = std::make_shared<Node>(*vertex);
        size_t diff = vertex->depth - new_depth;
        vertex->depth = new_depth;
        vertex->go.clear();
        nvertex->from = vertex->to -= diff;
        vertex->go[str[nvertex->from]] = nvertex;
        return vertex;
    }

    static void build_suffix_tree(const std::shared_ptr<Node>& root, const std::vector<size_t>& suff_array, const std::vector<size_t>& lcp_array, const std::vector<TChar>& str) {
        std::stack<std::shared_ptr<Node>> st;
        st.push(root);
        for (size_t index = 0; index <= lcp_array.size(); ++index) {
            auto vertex = st.top();
            size_t start_pos = suff_array[index] + vertex->depth;
            auto added_vertex = st.top()->go[str[start_pos]]
                    = std::make_shared<Node>(/*depth=*/str.size() - start_pos + vertex->depth, /*from=*/start_pos, /*to=*/str.size());
            st.push(added_vertex);

            if (index == lcp_array.size()) return;

            while (st.top()->depth > lcp_array[index]) {
                vertex = st.top();
                st.pop();
            }
            if (st.top()->depth == lcp_array[index]) continue;
            size_t common_prefix = lcp_array[index];
            st.push(split_vertex_at_depth(common_prefix, vertex, str));
        }

    }

    void print_tree(const std::shared_ptr<Node>& node, std::vector<TChar>& out) const {
        for (size_t i = node->from; i < node->to; ++i) {
            out.push_back(s[i]);
        }
        if (node->go.empty()) {
            for (auto it : out) {
                std::cout << it << ' ';
            }
            std::cout << '\n';
        }
        for (const auto& [_, vertex] : node->go) {
            print_tree(vertex, out);
        }
        for (size_t i = node->from; i < node->to; ++i) {
            out.pop_back();
        }
    }

    size_t find_max_refren(const std::shared_ptr<Node>& vertex, std::tuple<size_t, size_t, size_t>& precounted_ans) const {
        if (vertex->go.empty()) return 1;
        size_t count = 0;
        for (const auto& [_, next] : vertex->go) {
            count += find_max_refren(next, precounted_ans);
        }
        size_t current_metrics = vertex->depth * count;
        if (current_metrics >= std::get<1>(precounted_ans) * std::get<2>(precounted_ans)) {
            precounted_ans = {/*start=*/vertex->to - vertex->depth, /*length=*/vertex->depth, /*count=*/count};
        }
        return count;
    }

public:
    SuffixTree(const std::vector<TChar>& str) : s(str) {
        s.push_back(empty_char);
        for (auto& it : s) ++it;
        auto suff_array = suffix_array::build_suffix_array<alphabet_size + 1>(s);
        auto lcp_array = suffix_array::count_lcp(s, suff_array);
        for (auto& it : s) --it;
        root_ = std::make_shared<Node>(/*depth=*/0, /*from=*/0, /*to=*/0);
        build_suffix_tree(root_, suff_array, lcp_array, s);
    }

    void print_tree() const {
        std::vector<TChar> out;
        print_tree(root_, out);
    }

    std::tuple<size_t, size_t, size_t> get_refren() const {
        std::tuple<size_t, size_t, size_t> ans{0, s.size() - 1, 1}; // {start, length, count}
        find_max_refren(root_, ans);
        return ans;
    }
};

void test(const std::string& s) {
    std::vector<int> str(s.begin(), s.end());
    for (auto& it : str) it -= 'a' - 1;
    SuffixTree<alphabet_size, int> t(str);
    t.print_tree();
}

Refren get_refren(const std::vector<ll>& s) {
    SuffixTree<alphabet_size, ll> tree(s);
    return tree.get_refren();
}
}


int main() {
    ll n, m;
    std::cin >> n >> m;
    std::vector<ll> s(n);
    for (auto& it : s) std::cin >> it;
    Refren r = suffix_tree::get_refren(s);

    std::cout << r.metrics() << '\n';
    std::cout << r.length() << '\n';
    std::cout << r.from(s) << '\n';
}
