#ifndef DEQUE_H
#define DEQUE_H

#include <algorithm>

template <typename T>
class Deque {
    size_t capacity_ = 0;
    int iteratorShift_ = 0; // it is constant while reallocating
    size_t begin_ = 0;
    size_t end_ = 0;
    T* data_;

    void prepare_push_front() {
        if (begin_ > 0) return;
        if (size() * 3 < capacity() * 2) move_to_center();
        else realloc_up();
    }

    void prepare_push_back() {
        if (end_ + 1 < capacity()) return;
        if (size() * 3 < capacity() * 2) move_to_center();
        else realloc_up();
    }

    void move_to_center() {
        size_t nbegin = capacity_ / 3;
        size_t nend = nbegin + size();
        T* ndata = new T[capacity_];
        std::move(data_ + begin_, data_ + end_, ndata + nbegin);
        begin_ = nbegin;
        end_ = nend;
        delete[] data_;
        data_ = ndata;
    }

    void realloc_up() {
        size_t ncapacity = capacity_ * 3 + 3;
        size_t nbegin = ncapacity / 3;
        size_t nend = nbegin + size();
        T* ndata = new T[ncapacity];
        std::move(data_ + begin_, data_ + end_, ndata + nbegin);
        begin_ = nbegin;
        end_ = nend;
        capacity_ = ncapacity;
        delete[] data_;
        data_ = ndata;
    }

    template <typename U, typename Container>
    class iterator_impl {
    public:
        int index = 0;
        Container* deque = nullptr;

        iterator_impl() = default;

        iterator_impl(int index, Container* deque): index(index), deque(deque) {}

        iterator_impl(const iterator_impl<U, Container>&) = default;

        iterator_impl<U, Container>& operator=(const iterator_impl<U, Container>&) = default;

        int cur_index() const {
            return index - deque->iteratorShift_;
        }

        U& operator*() const {
            return (*deque)[cur_index()];
        }

        U* operator->() const {
            return &(*deque)[cur_index()];
        }

        iterator_impl<U, Container>& operator++() {
            ++index;
            return *this;
        }

        iterator_impl<U, Container>& operator--() {
            --index;
            return *this;
        }

        iterator_impl<U, Container> operator++(int) {
            auto res = *this;
            ++index;
            return res;
        }

        iterator_impl<U, Container> operator--(int) {
            auto res = *this;
            --index;
            return res;
        }

        iterator_impl<U, Container>& operator+=(int shift) {
            index += shift;
            return *this;
        }

        iterator_impl<U, Container>& operator-=(int shift) {
            index -= shift;
            return *this;
        }

        bool operator<(const iterator_impl<U, Container>& that) const {
            return index < that.index;
        }

        bool operator==(const iterator_impl<U, Container>& that) const {
            return index == that.index;
        }

        bool operator!=(const iterator_impl<U, Container>& rhs) {
            return !(*this == rhs);
        }

        bool operator>(const iterator_impl<U, Container>& rhs) {
            return rhs < *this;
        }

        bool operator<=(const iterator_impl<U, Container>& rhs) {
            return *this < rhs || *this == rhs;
        }

        bool operator>=(const iterator_impl& rhs) {
            return *this > rhs || *this == rhs;
        }
    };

public:
    class iterator: public iterator_impl<T, Deque<T>> {
        iterator(int index, Deque<T>* deque): iterator_impl<T, Deque<T>>(index, deque) {}
        friend class Deque<T>;

    public:
        iterator operator+(int shift) {
            auto ans = *this;
            ans += shift;
            return ans;
        }

        iterator operator-(int shift) {
            auto ans = *this;
            ans -= shift;
            return ans;
        }

        int operator-(const iterator& iter) const {
            return this->index - iter.index;
        }
    };

    class const_iterator: public iterator_impl<const T, const Deque<T>> {
        const_iterator(int index, const Deque<T>* deque): iterator_impl<const T, const Deque<T>>(index, deque) {}
        friend class Deque<T>;
    public:
        const_iterator(const iterator& iter): const_iterator(iter.index, iter.deque) {}

        const_iterator& operator=(const iterator& that) {
            const_iterator copy = that;
            swap(copy);
            return *this;
        }

        const_iterator operator+(int shift) {
            auto ans = *this;
            ans += shift;
            return ans;
        }

        const_iterator operator-(int shift) {
            auto ans = *this;
            ans -= shift;
            return ans;
        }

        int operator-(const const_iterator& iter) {
            return this->index - iter.index;
        }
    };

    Deque(): data_(new T[capacity_]) {}

    Deque(const Deque<T>& that): capacity_(that.size()), end_(that.size()), data_(new T[capacity_]) {
        std::copy(that.data_ + that.begin_, that.data_ + that.end_, data_ + begin_);
    }

    Deque(int n, const T& value = T()): capacity_(n), end_(n), data_(new T[capacity_]) {
        std::fill(data_ + begin_, data_ + end_, value);
    }

    ~Deque() {
        delete[] data_;
    }

    void swap(Deque<T>& that) {
        std::swap(data_, that.data_);
        std::swap(iteratorShift_, that.iteratorShift_);
        std::swap(begin_, that.begin_);
        std::swap(end_, that.end_);
    }

    Deque<T>& operator=(const Deque<T>& that) {
        Deque tmp(that);
        swap(tmp);
        return *this;
    }

    T& operator[](size_t index) {
        return data_[begin_ + index];
    }

    const T& operator[](size_t index) const {
        return data_[begin_ + index];
    }

    const T& at(size_t index) const {
        if (index < 0) throw std::out_of_range("Buy a real computer\n");
        if (index >= size()) throw std::out_of_range("index is \\ge than size");
        return (*this)[index];
    }

    T& at(size_t index) {
        if (index < 0) throw std::out_of_range("Buy a real computer\n");
        if (index >= size()) throw std::out_of_range("index is \\ge than size");
        return (*this)[index];
    }

    size_t size() const {
        return end_ - begin_;
    }

    size_t capacity() const {
        return capacity_;
    }

    void push_back(const T& value) {
        prepare_push_back();
        data_[end_++] = value;
    }

    void push_front(const T& value) {
        prepare_push_front();
        --iteratorShift_;
        data_[--begin_] = value;
    }

    void pop_back() {
        --end_;
    }

    void pop_front() {
        ++iteratorShift_;
        ++begin_;
    }

    T& front() {
        return data_[begin_];
    }

    const T& front() const {
        return data_[begin_];
    }

    T& back() {
        return data_[end_ - 1];
    }

    const T& back() const {
        return data_[end_ - 1];
    }

    iterator begin() {
        return iterator(iteratorShift_, this);
    }

    const_iterator begin() const {
        return cbegin();
    }

    iterator end() {
        return iterator(iteratorShift_ + size(), this);
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return const_iterator(iteratorShift_, this);
    }

    const_iterator cend() const {
        return const_iterator(iteratorShift_ + size(), this);
    }

    void insert(const const_iterator& pos, const T& value) {
        push_back(value);
        int index = pos.cur_index();
        std::rotate(data_ + begin_ + index, data_ + end_ - 1, data_ + end_);
    }

    void erase(const const_iterator& pos) {
        int index = pos.cur_index();
        std::rotate(data_ + begin_ + index, data_ + begin_ + index + 1, data_ + end_);
        pop_back();
    }
};

#endif
