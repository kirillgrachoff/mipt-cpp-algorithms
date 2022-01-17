#pragma once
#include <memory>
#include <iterator>

template <typename T, typename Alloc = std::allocator<T>>
class List {
    struct Node {
        T value = T();
        Node* prev = nullptr;
        Node* next = nullptr;
        Node() = default;
        Node(const T& value, Node* prev = nullptr, Node* next = nullptr): value(value), prev(prev), next(next) {}
        Node(T&& value, Node* prev = nullptr, Node* next = nullptr): value(std::move(value)), prev(prev), next(next) {}
    };

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

        iterator_impl(const iterator_impl<isConst>& other): current(other.current) {}

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

    Alloc alloc_;
    typename std::allocator_traits<Alloc>::template rebind_alloc<Node> allocNode_;
    Node* buf_;
    size_t size_ = 0;
    using Traits = std::allocator_traits<Alloc>;
    using TraitsNode = typename std::allocator_traits<Alloc>::template rebind_traits<Node>;

    void insertBefore(Node* element, const T& value) {
        Node* nvalue = TraitsNode::allocate(allocNode_, 1);
        TraitsNode::construct(allocNode_, nvalue, value, element->prev, element);
        linkBefore(element, nvalue);
    }

    void linkBefore(Node* element, Node* value) {
        value->next = element;
        value->prev = element->prev;
        element->prev->next = value;
        element->prev = value;
    }

    void eraseBefore(Node* element) {
        Node* trash = cutBefore(element);
        TraitsNode::destroy(allocNode_, trash);
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

    void push_front(const T& value) {
        ++size_;
        insertBefore(buf_->next, value);
    }

    void push_back(const T& value) {
        ++size_;
        insertBefore(buf_, value);
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
        while (!empty()) pop_back();
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

    void insert(const_iterator pos, const T& value) {
        ++size_;
        insertBefore(pos.current, value);
    }

    iterator erase(const_iterator pos) {
        --size_;
        ++pos;
        iterator ans(pos.current);
        eraseBefore(pos.current);
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
