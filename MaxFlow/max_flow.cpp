
#include <vector>
#include <queue>
#include <cassert>
#include <iostream>
#include <functional>
#include <memory>
#include <list>

template <typename T>
T sign(T value) {
    if (value == 0) return value;
    if constexpr (std::is_signed_v<T>) {
        if (value < static_cast<T>(0)) return static_cast<T>(-1);
    }
    return static_cast<T>(1);
}

template <typename T>
class IotaView {
private:
    T begin_;
    T end_;

public:
    IotaView() noexcept(std::is_nothrow_default_constructible_v<T>) = delete;
    IotaView(const IotaView&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
    IotaView(IotaView&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    IotaView& operator=(const IotaView&) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
    IotaView& operator=(IotaView&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

    ~IotaView() = default;

    IotaView(T begin, T end) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : begin_(begin)
            , end_(end)
    {}

    class IotaIterator {
    private:
        T value_;

        IotaIterator(const T& value) : value_(value) {}

        friend class IotaView<T>;

    public:
        IotaIterator() = default;
        IotaIterator(const IotaIterator&) = default;
        IotaIterator(IotaIterator&&) = default;

        IotaIterator& operator=(const IotaIterator&) = default;
        IotaIterator& operator=(IotaIterator&&) = default;

        T operator*() {
            return value_;
        }

        IotaIterator& operator++() {
            ++value_;
            return *this;
        }

        IotaIterator operator++(int) {
            auto ans = *this;
            ++*this;
            return ans;
        }

        friend bool operator==(const IotaIterator& a, const IotaIterator& b) {
            return a.value_ == b.value_;
        }

        friend bool operator!=(const IotaIterator& a, const IotaIterator& b) {
            return !(a == b);
        }
    };

    auto begin() const {
        return IotaIterator(begin_);
    }

    auto end() const {
        return IotaIterator(end_);
    }
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class Network;

template <typename TVertex, typename TFlow>
class Edge {
private:
    const TVertex from_;
    const TVertex to_;
    const TFlow capacity_;
    TFlow flow_;

    friend class Network<Edge, TVertex, TFlow>;

public:
    Edge(TVertex from, TVertex to, TFlow capacity, TFlow flow)
            : from_(from)
            , to_(to)
            , capacity_(capacity)
            , flow_(flow)
    {}

    TVertex from() const {
        return from_;
    }

    TVertex to() const {
        return to_;
    }

    TFlow capacity() const {
        return capacity_;
    }

    TFlow flow() const {
        return flow_;
    }

    TFlow remained_flow() const {
        return capacity() - flow();
    }

private:
    void push_flow(TFlow delta) {
        assert(delta <= remained_flow());
        flow_ += delta;
    }
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class Network {
private:
    using Edge = TEdge<TVertex, TFlow>;
    std::vector<std::vector<size_t>> adj_;
    std::vector<Edge> edges_;
    const TVertex vertices_count_;

    void requirements(TEdge<TVertex, TFlow> edge) {
        edge.from();
        edge.to();
        edge.capacity();
        edge.flow();
        edge.push_flow(0);
        edge.remained_flow();
    }

public:
    class OutgoingEdgeIterator {
    private:
        using Iter = typename std::vector<size_t>::iterator;
        using Net = Network<TEdge, TVertex, TFlow>;
        Net* network_;
        Iter current_;

        friend class Network<TEdge, TVertex, TFlow>;

        OutgoingEdgeIterator(Net* network, Iter position)
                : network_(network)
                , current_(position)
        {}

    public:
        OutgoingEdgeIterator() noexcept(std::is_nothrow_default_constructible_v<Iter>) = default;
        OutgoingEdgeIterator(const OutgoingEdgeIterator&) noexcept(std::is_nothrow_copy_constructible_v<Iter>) = default;
        OutgoingEdgeIterator(OutgoingEdgeIterator&&) noexcept(std::is_nothrow_move_constructible_v<Iter>) = default;

        OutgoingEdgeIterator& operator=(const OutgoingEdgeIterator&) & noexcept(std::is_nothrow_copy_assignable_v<Iter>) = default;
        OutgoingEdgeIterator& operator=(OutgoingEdgeIterator&&) & noexcept(std::is_nothrow_move_assignable_v<Iter>) = default;

        OutgoingEdgeIterator& operator++() {
            ++current_;
            return *this;
        }

        OutgoingEdgeIterator operator++(int) {
            auto result = *this;
            ++*this;
            return result;
        }

        friend bool operator==(const OutgoingEdgeIterator& a, const OutgoingEdgeIterator& b) {
            return a.network_ == b.network_ && a.current_ == b.current_;
        }

        friend bool operator!=(const OutgoingEdgeIterator& a, const OutgoingEdgeIterator& b) {
            return !(a == b);
        }

        friend bool operator<(const OutgoingEdgeIterator& a, const OutgoingEdgeIterator& b) {
            return a.current_ < b.current_;
        }

        Edge& operator*() {
            return network_->edges_[edge_id()];
        }

        Edge* operator->() {
            return &network_->edges_[edge_id()];
        }

        const Edge& operator*() const {
            return network_->edges_[edge_id()];
        }

        const Edge* operator->() const {
            return &network_->edges_[edge_id()];
        }

        size_t edge_id() const {
            return *current_;
        }

        size_t reversed_edge_id() const {
            return *current_ ^ 1;
        }

        Edge& reversed() {
            return network_->edges_[reversed_edge_id()];
        }

        void push_flow(TFlow flow) {
            operator*().push_flow(flow);
            reversed().push_flow(-flow);
        }
    };

    Network() = delete;
    Network(const Network&) = default;
    Network(Network&&) = default;

    Network& operator=(const Network&) = default;
    Network& operator=(Network&&) = default;

    ~Network() = default;

    Network(TVertex vertices)
            : adj_(vertices)
            , vertices_count_(vertices)
    {}

    constexpr static bool bidirected() {
        return true;
    }

    constexpr static bool directed() {
        return false;
    }

    void add_edge(TVertex from, TVertex to, TFlow capacity, bool bidirectional) {
        assert(from < vertices_count_);
        assert(to < vertices_count_);
        assert(capacity >= 0);
        adj_[from].push_back(edges_.size());
        edges_.emplace_back(from, to, capacity, 0);
        adj_[to].push_back(edges_.size());
        edges_.emplace_back(to, from, capacity * static_cast<TFlow>(bidirectional), 0);
    }

    auto outgoing_edges(TVertex v) {
        return IotaView(OutgoingEdgeIterator(this, adj_[v].begin()), OutgoingEdgeIterator(this, adj_[v].end()));
    }

    size_t size() const {
        return this->vertices_count_;
    }

    const std::vector<TEdge<TVertex, TFlow>>& all_edges() const {
        return edges_;
    }

    constexpr size_t nvertex() const {
        return std::numeric_limits<TVertex>::max();
    }
};

template class Network<Edge, int, int>;

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class MaxFlowAlgorithm {
protected:
    using Net = Network<TEdge, TVertex, TFlow>;
    Net& network_;
    const TVertex source_;
    const TVertex target_;

public:
    MaxFlowAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : network_(network)
            , source_(source)
            , target_(target)
    {}

    virtual TFlow push_max_flow() = 0;

    virtual ~MaxFlowAlgorithm() = default;
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class IPushLiftAlgorithm : public MaxFlowAlgorithm<TEdge, TVertex, TFlow> {
private:
    using Net = Network<TEdge, TVertex, TFlow>;

    std::vector<size_t> height_;
    std::vector<TFlow> extra_flow_;

    void push(typename Net::OutgoingEdgeIterator edge) {
        if (height_[edge->from()] != height_[edge->to()] + 1) {
            if (edge->from() != this->source_) return;
        }
        TFlow delta = std::min(extra_flow(edge->from()), edge->remained_flow());
        edge.push_flow(delta);
        extra_flow_[edge->from()] -= delta;
        extra_flow_[edge->to()] += delta;
    }

    void lift(TVertex vertex) {
        if (extra_flow_[vertex] == 0) return;
        if (vertex == this->source_ || vertex == this->target_) return;
        size_t new_height = std::numeric_limits<size_t>::max() / 4;
        for (auto edge : this->network_.outgoing_edges(vertex)) {
            if (edge->remained_flow() == 0) continue;
            new_height = std::min(new_height, height_[edge->to()]);
        }
        ++new_height;
        if (height_[vertex] <= new_height) {
            height_[vertex] = new_height;
        }
    }

protected:
    size_t height(TVertex vertex) const {
        return height_[vertex];
    }

    TFlow extra_flow(TVertex vertex) const {
        return extra_flow_[vertex];
    }

    std::list<TVertex> discharge(TVertex vertex) {
        std::list<TVertex> ans;
        while (extra_flow_[vertex] > 0) {
            for (auto edge : this->network_.outgoing_edges(vertex)) {
                TFlow prev_flow = extra_flow_[edge->to()];
                push(edge);
                if (prev_flow == 0 && extra_flow_[edge->to()] > 0 && edge->to() != this->target_ && edge->to() != this->source_) {
                    ans.push_back(edge->to());
                }
                if (extra_flow_[vertex] == 0) break;
            }
            if (extra_flow_[vertex] == 0) break;
            lift(vertex);
        }
        ans.remove_if([this](TVertex v) { return this->extra_flow(v) == 0; });
        return ans;
    }

    virtual void push_preflow() = 0;

public:
    IPushLiftAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : MaxFlowAlgorithm<TEdge, TVertex, TFlow>(network, source, target)
            , height_(network.size(), 0)
            , extra_flow_(network.size(), 0)
    {}

    TFlow push_max_flow() final override {
        height_.assign(this->network_.size(), 0);
        extra_flow_.assign(this->network_.size(), 0);
        height_[this->source_] = this->network_.size();
        extra_flow_[this->source_] = std::numeric_limits<TFlow>::max();
        for (auto edge : this->network_.outgoing_edges(this->source_)) {
            push(edge);
        }
        extra_flow_[this->source_] = 0;
        push_preflow();
        return extra_flow(this->target_);
    }

    ~IPushLiftAlgorithm() override = default;
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class GoldbergAlgorithm : public IPushLiftAlgorithm<TEdge, TVertex, TFlow> {
private:
    void push_preflow() final override {
        std::list<TVertex> overflow_vertices;
        for (size_t i = 0; i < this->network_.size(); ++i) {
            if (i == this->target_) continue;
            if (this->extra_flow(i) > 0) overflow_vertices.push_back(i);
        }
        while (true) {
            auto iter = overflow_vertices.begin();
            while (iter != overflow_vertices.end()) {
                TVertex vertex = *iter;
                size_t prev_height = this->height(vertex);
                overflow_vertices.splice(overflow_vertices.end(), this->discharge(vertex));
                if (this->extra_flow(vertex) == 0) {
                    auto next_iter = std::next(iter);
                    overflow_vertices.erase(iter);
                    iter = next_iter;
                } else if (this->height(vertex) != prev_height) {
                    overflow_vertices.erase(iter);
                    overflow_vertices.push_front(vertex);
                    iter = overflow_vertices.begin();
                } else {
                    ++iter;
                }
            }
            if (iter == overflow_vertices.end()) break;
        }
    }

public:
    GoldbergAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : IPushLiftAlgorithm<TEdge, TVertex, TFlow>(network, source, target)
    {}

    ~GoldbergAlgorithm() override = default;
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class IBlockingFlowAlgorithm : public MaxFlowAlgorithm<TEdge, TVertex, TFlow> {
protected:
    using Net = Network<TEdge, TVertex, TFlow>;
    std::vector<int> dist_;

    virtual TFlow push_blocking_flow() = 0;

    bool bfs() {
        dist_.assign(this->network_.size(), -1);
        dist_[this->source_] = 0;
        std::deque<TVertex> q{this->source_};
        while (!q.empty()) {
            TVertex v = q.front();
            q.pop_front();
            for (auto edge : this->network_.outgoing_edges(v)) {
                if (dist_[edge->to()] != -1) continue;
                if (edge->remained_flow() <= 0) continue;
                dist_[edge->to()] = dist_[v] + 1;
                q.push_back(edge->to());
            }
        }
        return dist_[this->target_] != -1;
    }

public:
    IBlockingFlowAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : MaxFlowAlgorithm<TEdge, TVertex, TFlow>(network, source, target)
            , dist_(network.size())
    {}

    TFlow push_max_flow() {
        TFlow ans(0);
        while (bfs()) {
            auto delta = push_blocking_flow();
            if (delta == 0) break;
            ans += delta;
        }
        return ans;
    }

    ~IBlockingFlowAlgorithm() override = default;
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class DinicAlgorithm : public IBlockingFlowAlgorithm<TEdge, TVertex, TFlow> {
private:
    using Net = Network<TEdge, TVertex, TFlow>;
    std::vector<typename IotaView<typename Net::OutgoingEdgeIterator>::IotaIterator> iter_;

    TFlow push_blocking_flow() final override {
        for (size_t i = 0; i < iter_.size(); ++i) {
            iter_[i] = this->network_.outgoing_edges(i).begin();
        }
        return push_blocking_flow_(this->source_, this->target_, std::numeric_limits<TFlow>::max());
    }

    TFlow push_blocking_flow_(TVertex vertex, TVertex target, TFlow limited_flow) {
        if (limited_flow <= 0) return 0;
        if (vertex == target) return limited_flow;
        TFlow ans = 0;
        for (; iter_[vertex] != this->network_.outgoing_edges(vertex).end(); ++iter_[vertex]) {
            auto edge = *iter_[vertex];
            if (this->dist_[edge->to()] != this->dist_[vertex] + 1) continue;
            if (edge->remained_flow() <= 0) continue;
            TFlow add = push_blocking_flow_(edge->to(), target, std::min(limited_flow, edge->remained_flow()));

            limited_flow -= add;
            edge.push_flow(add);
            ans += add;
            if (limited_flow <= 0) break; // !!! important (I've deleted it a few iterations ago)
        }
        return ans;
    }

public:
    DinicAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : IBlockingFlowAlgorithm<TEdge, TVertex, TFlow>(network, source, target)
            , iter_(network.size())
    {}
};

template <template <typename, typename> typename TEdge, typename TVertex, typename TFlow>
class ThreeIndiansAlgorithm : public IBlockingFlowAlgorithm<TEdge, TVertex, TFlow> {
private:
    using Net = Network<TEdge, TVertex, TFlow>;
    std::vector<TFlow> potential_ingoing;
    std::vector<TFlow> potential_outgoing;
    std::vector<char> edge_available_;
    std::vector<typename IotaView<typename Net::OutgoingEdgeIterator>::IotaIterator> iter_forward_;
    std::vector<typename IotaView<typename Net::OutgoingEdgeIterator>::IotaIterator> iter_backward_;
    std::queue<TVertex> dead_vertex_queue;
    std::vector<char> vertex_is_dead;

    static constexpr TFlow max_potential_ = std::numeric_limits<TFlow>::max() / 4;

    TFlow potential_(TVertex v) const {
        if (v == this->source_) [[unlikely]] return potential_outgoing[v];
        if (v == this->target_) [[unlikely]] return potential_ingoing[v];
        return std::min(potential_outgoing[v], potential_ingoing[v]);
    }

    void mark_dead(TVertex v) {
        if (vertex_is_dead[v]) return;
        vertex_is_dead[v] = true;
        dead_vertex_queue.push(v);
    }

    TFlow push_blocking_flow() final override {
        vertex_is_dead.assign(this->network_.size(), false);
        edge_available_.assign(this->network_.all_edges().size(), true);
        iter_forward_.clear();
        iter_forward_.reserve(this->network_.size());
        iter_backward_.clear();
        iter_backward_.reserve(this->network_.size());
        for (size_t i = 0; i < this->network_.size(); ++i) {
            iter_forward_.emplace_back(this->network_.outgoing_edges(i).begin());
            iter_backward_.emplace_back(this->network_.outgoing_edges(i).begin());
        }

        calc_potential();
        TFlow ans = 0;
        while (true) {
            remove_dead_vertices();
            if (vertex_is_dead[this->source_] || vertex_is_dead[this->target_]) break;
            TVertex r = find_reference_vertex();
            if (r == this->network_.nvertex()) break;
            if (potential_(r) == 0) break;
            TFlow f = potential_(r);
            push_flow_through(r, f);
            ans += f;
        }
        return ans;
    }

    void calc_potential() {
        potential_ingoing.assign(this->network_.size(), 0);
        potential_outgoing.assign(this->network_.size(), 0);

        for (size_t i = 0; i < this->network_.size(); ++i) {
            for (auto edge : this->network_.outgoing_edges(i)) {
                if (!is_forward_edge(edge)) continue;
                potential_outgoing[i] += edge->remained_flow();
                if (potential_outgoing[i] >= max_potential_) {
                    potential_outgoing[i] = max_potential_;
                }
                potential_ingoing[edge->to()] += edge->remained_flow();
                if (potential_ingoing[edge->to()] >= max_potential_) {
                    potential_ingoing[edge->to()] = max_potential_;
                }
            }
        }
        for (size_t i = 0; i < this->network_.size(); ++i) {
            if (potential_(i) == 0) mark_dead(i);
        }
    }

    TVertex find_reference_vertex() const {
        TVertex answer = this->source_;
        for (size_t vertex = 0; vertex < this->network_.size(); ++vertex) {
            if (vertex_is_dead[vertex]) continue;
            if (potential_(vertex) <= 0) continue;
            if (potential_(vertex) <= potential_(answer)) answer = vertex;
        }
        if (potential_(answer) == 0 || vertex_is_dead[answer]) {
            return this->network_.nvertex();
        }
        return answer;
    }

    constexpr static bool forward_flow() {
        return false;
    }

    constexpr static bool backward_flow() {
        return !forward_flow();
    }

    void push_flow_through(TVertex r, TFlow flow) {
        assert(push_flow(r, this->target_, flow, forward_flow(), [this](const typename Net::OutgoingEdgeIterator& e) { return is_forward_edge(e); }) == flow);
        assert(push_flow(r, this->source_, flow, backward_flow(), [this](const typename Net::OutgoingEdgeIterator& e) { return is_backward_edge(e); }) == flow);
    }

    bool is_forward_edge(const typename Net::OutgoingEdgeIterator& edge) {
        if (!edge_available_[edge.edge_id()]) return false;
        return this->dist_[edge->from()] + 1 == this->dist_[edge->to()];
    }

    bool is_backward_edge(const typename Net::OutgoingEdgeIterator& edge) {
        if (!edge_available_[edge.edge_id()]) return false;
        return this->dist_[edge->from()] == this->dist_[edge->to()] + 1;
    }

    TFlow push_flow(TVertex from, TVertex to, TFlow flow, bool backward, std::function<bool(const typename Net::OutgoingEdgeIterator&)> pred) {
        if (from == to) return flow;
        if (flow <= 0) return 0;
        TFlow ans = 0;
        auto& iter = backward ? iter_backward_[from] : iter_forward_[from];
        auto& ingoing = backward ? potential_outgoing : potential_ingoing;
        auto& outgoing = backward ? potential_ingoing : potential_outgoing;
        for (; iter != this->network_.outgoing_edges(from).end(); ++iter) {
            auto edge = *iter;
            if (!pred(edge)) continue;
            TFlow delta = push_flow(edge->to(), to, std::min(flow, backward ? edge.reversed().remained_flow() : edge->remained_flow()), backward, pred);
            ingoing[edge->to()] -= delta;
            if (potential_(edge->to()) == 0) mark_dead(edge->to());
            flow -= delta;
            ans += delta;
            edge.push_flow(backward ? -delta : delta);
            if (flow == 0) break;
        }
        outgoing[from] -= ans;
        if (potential_(from) == 0) mark_dead(from);
        return ans;
    }

    void remove_dead_vertices() {
        while (!dead_vertex_queue.empty()) {
            TVertex v = dead_vertex_queue.front();
            dead_vertex_queue.pop();
            for (auto edge : this->network_.outgoing_edges(v)) {
                if (!is_forward_edge(edge) && !is_backward_edge(edge)) continue;
                if (is_forward_edge(edge)) {
                    potential_outgoing[edge->from()] -= edge->remained_flow();
                    potential_ingoing[edge->to()] -= edge->remained_flow();
                }
                if (is_backward_edge(edge)) {
                    potential_outgoing[edge->to()] -= edge.reversed().remained_flow();
                    potential_ingoing[edge->from()] -= edge.reversed().remained_flow();
                }
                edge_available_[edge.edge_id()] = false;
                edge_available_[edge.reversed_edge_id()] = false;
                if (potential_(edge->to()) == 0) {
                    mark_dead(edge->to());
                }
            }
        }
    }

public:
    ThreeIndiansAlgorithm(Network<TEdge, TVertex, TFlow>& network, TVertex source, TVertex target)
            : IBlockingFlowAlgorithm<TEdge, TVertex, TFlow>(network, source, target)
    {}
};

using namespace std;

constexpr int max_penalty = std::numeric_limits<decltype(max_penalty)>::max() / 4;

void solve() {
    int n;
    cin >> n;
    vector<int> cost(n);
    int maximum_sum = 0;
    for (auto& it : cost) {
        cin >> it;
        if (it > 0) maximum_sum += it;
    }

    Network<Edge, int, long long> net(n + 2);
    const int acquire = n;
    const int decline = acquire + 1;

    for (int theme_no = 0; theme_no < n; ++theme_no) {
        if (cost[theme_no] > 0) {
            net.add_edge(acquire, theme_no, cost[theme_no], net.directed());
        } else {
            net.add_edge(theme_no, decline, -cost[theme_no], net.directed());
        }
    }
    for (int theme_no = 0; theme_no < n; ++theme_no) {
        int k;
        cin >> k;
        while (k--) {
            int requires_theme;
            cin >> requires_theme;
            --requires_theme;
            net.add_edge(theme_no, requires_theme, max_penalty, net.directed());
        }
    }

    unique_ptr<MaxFlowAlgorithm<Edge, int, long long>> algorithm;
    algorithm = make_unique<GoldbergAlgorithm<Edge, int, long long>>(net, acquire, decline);

    cout << maximum_sum - algorithm->push_max_flow() << '\n';
}

int main() {
    solve();
}
