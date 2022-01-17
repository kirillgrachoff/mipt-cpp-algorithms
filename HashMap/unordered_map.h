#pragma once

#include <memory>
#include <vector>
#include <iterator>
#include <limits>

template <typename T, typename Alloc = std::allocator<T>>
class List {
    struct Node {
        T value = T();
        Node* prev = nullptr;
        Node* next = nullptr;
        template <typename... Args>
        Node(Args&&... args): value(std::forward<Args>(args)...) {}
    };

    Alloc alloc_;
    typename std::allocator_traits<Alloc>::template rebind_alloc<Node> allocNode_;
    Node* buf_;
    size_t size_ = 0;
    using Traits = std::allocator_traits<Alloc>;
    using TraitsNode = typename std::allocator_traits<Alloc>::template rebind_traits<Node>;

    void linkBefore(Node* element, Node* value) {
        value->next = element;
        value->prev = element->prev;
        element->prev->next = value;
        element->prev = value;
    }

    void eraseBefore(Node* element) {
        Node* trash = cutBefore(element);
        Traits::destroy(alloc_, &(trash->value));
        TraitsNode::deallocate(allocNode_, trash, 1);
    }

    Node* cutBefore(Node* element) {
        Node* trash = element->prev;
        trash->prev->next = element;
        element->prev = trash->prev;
        return trash;
    }

    void constructBuf() {
        buf_ = TraitsNode::allocate(allocNode_, 1);
        buf_->next = buf_;
        buf_->prev = buf_;
    }

    void destroyBuf() {
        TraitsNode::deallocate(allocNode_, buf_, 1);
    }

public:
    explicit List(const Alloc& alloc = Alloc()): alloc_(alloc), allocNode_(alloc) {
        constructBuf();
    }

    List(size_t n, const Alloc& alloc = Alloc()): alloc_(alloc) {
        constructBuf();
        size_ = n;
        for (size_t i = 0; i < n; ++i) {
            Node* newNode = TraitsNode::allocate(allocNode_, 1);
            TraitsNode::construct(allocNode_, newNode);
            linkBefore(buf_, newNode);
        }
    }

    List(size_t n, const T& value, const Alloc& alloc = Alloc()): List(alloc) {
        for (size_t i = 0; i < n; ++i) {
            push_back(value);
        }
    }

    List(const List<T, Alloc>& list):
            alloc_(Traits::select_on_container_copy_construction(list.alloc_)),
            allocNode_(TraitsNode::select_on_container_copy_construction(list.allocNode_)) {
        constructBuf();
        for (auto it = list.cbegin(); it != list.cend(); ++it) {
            push_back(*it);
        }
    }

    List(List&& list) {
        constructBuf();
        swap(list);
    }

    List& operator=(List<T, Alloc>&& list) {
        using std::swap;
        clear();
        swap(buf_, list.buf_);
        if (Traits::propagate_on_container_move_assignment::value && alloc_ != list.alloc_) {
            alloc_ = list.alloc_;
            allocNode_ = list.allocNode_;
        }
        swap(size_, list.size_);
        return *this;
    }

    List& operator=(const List<T, Alloc>& list) {
        if (this == &list) return *this;
        clear();
        if (TraitsNode::propagate_on_container_copy_assignment::value && alloc_ != list.alloc_) {
            destroyBuf();
            alloc_ = list.alloc_;
            allocNode_ = list.allocNode_;
            constructBuf();
        }
        for (auto& it : list) {
            push_back(it);
        }
        return *this;
    }

    ~List() {
        clear();
        destroyBuf();
    }

    template <bool isConst>
    class iterator_impl {
    private:
        Node* current = nullptr;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::conditional_t<isConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using pointer = value_type*;

        iterator_impl() = default;
        iterator_impl(const iterator_impl<isConst>&) = default;
        iterator_impl(iterator_impl<isConst>&&) = default;

        void swap(iterator_impl<isConst>& that) {
            std::swap(current, that.current);
        }

        iterator_impl<isConst>& operator=(const iterator_impl<isConst>&) = default;

        operator iterator_impl<true>() const {
            return iterator_impl<true>(this->current);
        }

        iterator_impl<isConst>& operator++() {
            current = current->next;
            return *this;
        }

        iterator_impl<isConst> operator++(int) {
            auto ans = *this;
            ++*this;
            return ans;
        }

        iterator_impl<isConst>& operator--() {
            current = current->prev;
            return *this;
        }

        iterator_impl<isConst> operator--(int) {
            auto ans = *this;
            --*this;
            return ans;
        }

        reference operator*() const {
            return current->value;
        }

        pointer operator->() const {
            return &(current->value);
        }

        explicit operator bool() const {
            return current != nullptr;
        }

        friend bool operator==(const iterator_impl<isConst>& a, const iterator_impl<isConst>& b) {
            return a.current == b.current;
        }

        friend bool operator!=(const iterator_impl<isConst>& a, const iterator_impl<isConst>& b) {
            return !(a == b);
        }

    protected:
        iterator_impl(Node* t): current(t) {}

        friend class List<T, Alloc>;
    };

    void swap(List<T, Alloc>& that) {
        if (this == &that) return;
        using std::swap;
        swap(buf_, that.buf_);
        swap(alloc_, that.alloc_);
        swap(allocNode_, that.allocNode_);
        swap(size_, that.size_);

    }

    void push_front(const T& value) {
        emplace(begin(), value);
    }

    void push_back(const T& value) {
        emplace(end(), value);
    }

    void pop_back() {
        --size_;
        eraseBefore(buf_);
    }

    void pop_front() {
        --size_;
        eraseBefore(buf_->next->next);
    }

    T& back() {
        return buf_->prev->value;
    }

    const T& back() const {
        return buf_->prev->value;
    }

    T& front() {
        return buf_->next->value;
    }

    const T& front() const {
        return buf_->next->value;
    }

    size_t size() const {
        return size_;
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    bool empty() const {
        return size() == 0;
    }

    Alloc get_allocator() {
        return alloc_;
    }

    using const_iterator = iterator_impl<true>;

    using iterator = iterator_impl<false>;

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using reverse_iterator = std::reverse_iterator<iterator>;

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        ++size_;
        Node* nvalue = TraitsNode::allocate(allocNode_, 1);
        Traits::construct(alloc_, &(nvalue->value), std::forward<Args>(args)...);
        linkBefore(pos.current, nvalue);
        --pos;
        return iterator(pos.current);
    }

    iterator erase(const_iterator pos) {
        if (pos.current == buf_) return end();
        --size_;
        ++pos;
        iterator ans(pos.current);
        eraseBefore(pos.current);
        return ans;
    }

    iterator erase(const_iterator begin, const_iterator end) {
        iterator ans(end.current);
        if (begin == end) return ans;
        while (begin.current->next != end.current) {
            --size_;
            eraseBefore(end.current);
        }
        return ans;
    }

    iterator begin() {
        return iterator(buf_->next);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        return const_iterator(buf_->next);
    }

    iterator end() {
        return iterator(buf_);
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cend() const {
        return const_iterator(buf_);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }
};

template <
        typename Key,
        typename Value,
        typename Hash = std::hash<Key>,
        typename Equal = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
    static size_t hashFn(const Key& key) {
        static Hash hash;
        return hash(key);
    }

    static bool equalFn(const Key& a, const Key& b) {
        static Equal equal;
        return equal(a, b);
    }

    constexpr static size_t kInitialBucketCount = 2000;
    constexpr static size_t kMaxSearchDist = 20;

    using NodeType = std::pair<const Key, Value>;
    using NodeTypeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>;

    using ListIterator = typename List<NodeType, NodeTypeAlloc>::iterator;
    using ListIteratorAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ListIterator>;

    Alloc alloc_;
    using Traits = std::allocator_traits<Alloc>;
    std::vector<ListIterator, ListIteratorAlloc> hashTable_;
    List<NodeType, NodeTypeAlloc> list_;

    double maxLoadFactor_ = 0.2;

    template<bool isConst>
    using IteratorImpl = typename List<NodeType, NodeTypeAlloc>::template iterator_impl<isConst>;

    void rehash() {
        bool correct = true;
        size_t count = hashTable_.size();
        hashTable_.assign(count, ListIterator());
        for (auto it = list_.begin(); it != list_.end() && correct; ++it) {
            size_t hashIndex = bucketIndex(it->first);
            for (size_t i = 0; i < kMaxSearchDist; ++i) {
                if (!hashTableElem(i + hashIndex)) {
                    hashTableElem(i + hashIndex) = it;
                    break;
                } else if (i == kMaxSearchDist - 1) {
                    correct = false;
                }
            }
        }
        if (!correct) {
            reserve(count * 2);
        }
    }

    size_t bucketIndex(const Key& key) {
        size_t ans = hashFn(key) % hashTable_.size();
        return ans;
    }

    IteratorImpl<false> findPlace(const Key& key) {
        size_t hashIndex = bucketIndex(key);

        for (size_t i = 0; i < kMaxSearchDist; ++i) {
            if (!hashTable_[(hashIndex + i) % hashTable_.size()]) {
                return list_.end();
            }
            if (equalFn(key, hashTable_[(hashIndex + i) % hashTable_.size()]->first)) {
                return hashTableElem(hashIndex + i);
            }
        }
        return list_.end();
    }

    IteratorImpl<true> findPlace(const Key& key) const {
        size_t hashIndex = bucketIndex(key);

        for (size_t i = 0; i < kMaxSearchDist; ++i) {
            if (!hashTable_[(hashIndex + i) % hashTable_.size()]) {
                return list_.end();
            }
            if (equalFn(key, hashTableElem(hashIndex + i)->first)) {
                return hashTable_[(hashIndex + i) % hashTable_.size()];
            }
        }
        return list_.end();
    }

    bool addIterator(IteratorImpl<false> iter) {
        size_t hashIndex = bucketIndex(iter->first);
        bool inserted = false;
        while (!inserted) {
            for (size_t i = 0; i < kMaxSearchDist; ++i) {
                if (!hashTable_[(hashIndex + i) % hashTable_.size()]) {
                    hashTable_[(hashIndex + i) % hashTable_.size()] = iter;
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                reserve(hashTable_.size() * 2);
            }
        }
        if (load_factor() > max_load_factor()) {
            reserve(hashTable_.size() * 2);
            return true;
        }
        return false;
    }

    decltype(auto) hashTableElem(size_t i) {
        return hashTable_[i % hashTable_.size()];
    }

public:
    using Iterator = IteratorImpl<false>;
    using ConstIterator = IteratorImpl<true>;

    UnorderedMap(Alloc alloc = Alloc()): alloc_(alloc), hashTable_(alloc_), list_(alloc_) {
        reserve(kInitialBucketCount);
    }

    UnorderedMap(const UnorderedMap& that):
        alloc_(Traits::select_on_container_copy_construction(that.alloc_)),
        hashTable_(alloc_),
        list_(that.list_)
    {
        reserve(that.hashTable_.size());
        rehash();
    }

    UnorderedMap& operator=(UnorderedMap& that) {
        list_ = that.list_;
        hashTable_ = that.hashTable_;
        if (Traits::propagate_on_container_copy_assignment::value) {
            alloc_ = that.alloc_;
        }
        rehash();
        return *this;
    }

    UnorderedMap(UnorderedMap&& that) = default;

    UnorderedMap& operator=(UnorderedMap&& that) {
        list_ = std::move(that.list_);
        hashTable_ = std::move(that.hashTable_);
        if (Traits::propagate_on_container_move_assignment::value) {
            alloc_ = std::move(that.alloc_);
        }
        return *this;
    }

    Value& operator[](const Key& key) {
        Iterator it = findPlace(key);
        if (it == list_.end()) {
            it = list_.emplace(it, key, Value());
            if (addIterator(it)) {
                return find(key)->second;
            }
        }
        return it->second;
    }

    Value& at(const Key& key) {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("no such element");
        }
        return it->second;
    }

    const Value& at(const Key& key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("no such element");
        }
        return it->second;
    }

    size_t size() const {
        return list_.size();
    }

    std::pair<Iterator, bool> insert(const NodeType& node) {
        return emplace(node);
    }

    std::pair<Iterator, bool> insert(NodeType&& node) {
        return emplace(std::move(const_cast<Key&>(node.first)), std::move(node.second));
    }

    template <typename InputIt>
    void insert(InputIt begin, InputIt end) {
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    template <typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        Iterator place = list_.emplace(list_.end(), std::forward<Args>(args)...);
        Iterator it = findPlace(place->first);
        if (it != list_.end() && equalFn(place->first, it->first)) {
            if (it == place) {
                return {it, true};
            } else {
                list_.pop_back();
                return {it, false};
            }
        }
        addIterator(place);
        return {place, true};
    }

    void erase(Iterator elem) {
        if (elem == list_.end()) return;
        size_t hashIndex = bucketIndex(elem->first);
        for (size_t i = 0; i < kMaxSearchDist; ++i) {
            if (hashTableElem(hashIndex + i) == elem) {
                hashTableElem(hashIndex + i) = Iterator();
                break;
            }
        }
        list_.erase(elem);
    }

    void erase(Iterator begin, Iterator end) {
        for (auto it = begin; it != end;) {
            auto next = it;
            std::advance(next, 1);
            erase(it);
            it = next;
        }
    }

    Iterator find(const Key& key) {
        Iterator it = findPlace(key);
        if (it == list_.end() || !equalFn(it->first, key)) {
            return end();
        }
        return Iterator(it);
    }

    ConstIterator find(const Key& key) const {
        ConstIterator it = findPlace(key);
        if (it == list_.end() || !equalFn(it->first, key)) {
            return cend();
        }
        return ConstIterator(it);
    }

    void reserve(size_t size) {
        if (size <= hashTable_.size()) return;
        hashTable_.resize(size);
        if (!list_.empty()) rehash();
    }

    constexpr size_t max_size() const {
        return std::numeric_limits<size_t>::max();
    }

    double load_factor() const {
        return static_cast<double>(list_.size()) / hashTable_.size();
    }

    double max_load_factor() const {
        return maxLoadFactor_;
    }

    void max_load_factor(double factor) {
        maxLoadFactor_ = factor;
        if (load_factor() > factor) {
            reserve(hashTable_.size() * 2);
        }
    }

    Iterator begin() {
        return Iterator(list_.begin());
    }

    ConstIterator begin() const {
        return cbegin();
    }

    ConstIterator cbegin() const {
        return ConstIterator(list_.begin());
    }

    Iterator end() {
        return Iterator(list_.end());
    }

    ConstIterator end() const {
        return cend();
    }

    ConstIterator cend() const {
        return ConstIterator(list_.end());
    }
};
