#ifndef BZ
#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#undef assert
#define assert(v)
#endif

#include <bits/stdc++.h>

#define forEach(c) for (auto& it : c)

template <class T>
inline void chMax(T& a, T b) {
  if (a < b) a = b;
}

template <class T>
inline void chMin(T& a, T b) {
  if (b < a) a = b;
}

[[noreturn]] void panic() {
  std::cout << "-1\n";
  exit(0);
}

namespace splay {
struct node {
  bool needAssign;
  bool needAdd;
  bool needReverse;
  // for next_permutation
  int32_t suffixDown;  // \_>
  int32_t prefixUp;    // _/>
  // for prev_permutation
  int32_t suffixUp;
  int32_t prefixDown;
  int32_t size;
  int64_t value;  // it is also assignValue
  int64_t front;
  int64_t back;
  int64_t addval;  // it is addValue
  int64_t sum;
  node* parent = nullptr;
  node* left_ = nullptr;
  node* right_ = nullptr;

  bool needPush() const { return needAssign || needAdd || needReverse; }

  static void* operator new(size_t count) {
    static char globalBuf[10000000];
    static size_t globalBufIndex = 0;
    auto ans = globalBuf + globalBufIndex;
    globalBufIndex += count;
    return ans;
  }

  static void operator delete(void*) noexcept {}

  node(int64_t value)
      : needAssign(false),
        needAdd(false),
        needReverse(false),
        suffixDown(1),
        prefixUp(1),
        suffixUp(1),
        prefixDown(1),
        size(1),
        value(value),
        front(value),
        back(value),
        addval(0),
        sum(value) {}

  ~node() {
    delete left_;
    delete right_;
  }
};

void push(node*);

int64_t size(node* t) {
  if (!t) return 0;
  return t->size;
}

int64_t suffixDown(node* t) {
  if (!t) return 0;
  return t->suffixDown;
}

int64_t suffixUp(node* t) {
  if (!t) return 0;
  return t->suffixUp;
}

int64_t prefixDown(node* t) {
  if (!t) return 0;
  return t->prefixDown;
}

int64_t prefixUp(node* t) {
  if (!t) return 0;
  return t->prefixUp;
}

int64_t sum(node* t) {
  if (!t) return 0;
  return t->sum;
}

int64_t back(node* t, node* node::*left = &node::left_,
             node* node::*right = &node::right_) {
  assert(t);
  if (left == &node::left_) return t->back;
  return t->front;
}

int64_t front(node* t, node* node::*left = &node::left_,
              node* node::*right = &node::right_) {
  assert(t);
  if (left == &node::left_) return t->front;
  return t->back;
}

node* parent(node* t) {
  if (!t) return nullptr;
  return t->parent;
}

node*& parentLink(node* t) {
  assert(t != nullptr);
  if (t->parent->right_ == t) return t->parent->right_;
  return t->parent->left_;
}

void updatePrefixUp(node* t) {
  t->prefixUp = prefixUp(t->left_);
  if (t->prefixUp == size(t->left_) && t->left_ && t->value >= back(t->left_)) {
    ++(t->prefixUp);
  }
  chMax(t->prefixUp, 1);
  if (t->prefixUp == size(t->left_) + 1 && t->right_ &&
      front(t->right_) >= t->value) {
    t->prefixUp += prefixUp(t->right_);
  }
}

void updatePrefixDown(node* t) {
  t->prefixDown = prefixDown(t->left_);
  if (t->prefixDown == size(t->left_) && t->left_ &&
      t->value <= back(t->left_)) {
    ++(t->prefixDown);
  }
  chMax(t->prefixDown, 1);
  if (t->prefixDown == size(t->left_) + 1 && t->right_ &&
      front(t->right_) <= t->value) {
    t->prefixDown += prefixDown(t->right_);
  }
}

void updateSuffixUp(node* t) {
  t->suffixUp = suffixUp(t->right_);
  if (t->suffixUp == size(t->right_) && t->right_ &&
      t->value <= front(t->right_)) {
    ++(t->suffixUp);
  }
  chMax(t->suffixUp, 1);
  if (t->suffixUp == size(t->right_) + 1 && t->left_ &&
      back(t->left_) <= t->value) {
    t->suffixUp += suffixUp(t->left_);
  }
}

void updateSuffixDown(node* t) {
  t->suffixDown = suffixDown(t->right_);
  if (t->suffixDown == size(t->right_) && t->right_ &&
      t->value >= front(t->right_)) {
    ++(t->suffixDown);
  }
  chMax(t->suffixDown, 1);
  if (t->suffixDown == size(t->right_) + 1 && t->left_ &&
      back(t->left_) >= t->value) {
    t->suffixDown += suffixDown(t->left_);
  }
}

void update(node* t) {
  if (!t) return;
  assert(!t->needReverse);
  assert(!t->needAdd);
  assert(!t->needAssign);
  t->size = size(t->left_) + 1 + size(t->right_);
  t->sum = sum(t->left_) + t->value + sum(t->right_);
  updatePrefixUp(t);
  updatePrefixDown(t);
  updateSuffixUp(t);
  updateSuffixDown(t);
  if (t->left_) {
    t->front = front(t->left_);
  } else {
    t->front = t->value;
  }
  if (t->right_) {
    t->back = back(t->right_);
  } else {
    t->back = t->value;
  }
}

void doReverse(node* t) {
  using std::swap;
  if (!t) return;
  swap(t->left_, t->right_);
  swap(t->prefixUp, t->suffixDown);
  swap(t->prefixDown, t->suffixUp);
  swap(t->front, t->back);
}

void doAssign(node* t, int64_t value) {
  t->value = t->back = t->front = value;
  t->suffixDown = t->suffixUp = t->prefixDown = t->prefixUp = size(t);
  t->sum = size(t) * t->value;
}

void doAdd(node* t, int64_t addval) {
  t->addval += addval;
  t->value += addval;
  t->sum += addval * size(t);
  t->front += addval;
  t->back += addval;
}

void push(node* t);

void doPush(const node* t, node* node::*son) {
  if (!(t->*son)) return;
  auto p = t->*son;
  if (t->needAssign) {
    p->needAssign = true;
    p->needAdd = false;
    p->addval = 0;
    doAssign(p, t->value);
  } else if (t->needAdd) {
    if (p->needAssign) push(p);
    p->needAssign = false;
    p->needAdd = true;
    doAdd(p, t->addval);
  }
  if (t->needReverse) {
    p->needReverse ^= true;
    doReverse(p);
  }
}

void push(node* t) {
  if (!t) return;
  if (!t->needPush()) return;
  assert(!t->needAdd || !t->needAssign);
  doPush(t, &node::left_);
  doPush(t, &node::right_);
  t->needAdd = false;
  t->addval = 0;
  t->needAssign = false;
  t->needReverse = false;
}

// contract: a [<-son] b
void cut(node* a, node* b, node* node::*son) {
  if (!a) return;
  assert(b->*son == a && a->parent == b);
  assert(!b->needPush());
  a->parent = nullptr;
  b->*son = nullptr;
  update(b);
}

// contract: a [<-son] b
void link(node* a, node* b, node* node::*son) {
  if (!a) return;
  assert(b->*son == nullptr && a->parent == nullptr);
  assert(!b->needPush());
  a->parent = b;
  b->*son = a;
  update(b);
}

// contract: a <- b ... root
void smallRotateImpl(node* a, node* b, node*& parent, node* node::*left,
                     node* node::*right) {
  assert(b->*left == a && parent == b && a->parent == b);
  assert(!b->needPush() && !a->needPush());
  node* central = a->*right;
  if (central) cut(central, a, right);
  cut(a, b, left);
  if (central) link(central, b, left);
  parent = a;
  std::swap(a->parent, b->parent);
  link(b, a, right);
}

// contract: a <- b <- c ... root
void zigZigImpl(node* a, node* b, node* c, node*& parent, node* node::*left,
                node* node::*right) {
  smallRotateImpl(b, c, parent, left, right);
  smallRotateImpl(a, b, parent, left, right);
  assert(parent == a && a->*right == b && b->*right == c && c->parent == b &&
         b->parent == a);
}

// contract: b -> a <- c ... root
void zigZagImpl(node* a, node* b, node* c, node*& parent, node* node::*left,
                node* node::*right) {
  smallRotateImpl(a, b, c->*left, right, left);
  smallRotateImpl(a, c, parent, left, right);
  assert(parent == a && a->*left == b && a->*right == c && c->parent == a &&
         b->parent == a);
}

std::tuple<node * node::*, node * node::*> correctPtr(node* leaf, node* root) {
  if (root->left_ == leaf) {
    return {&node::left_, &node::right_};
  } else {
    return {&node::right_, &node::left_};
  }
}

// a <- b <- c <- ... root
std::tuple<bool, node * node::*, node * node::*> correctZigZig(node* a, node* b,
                                                               node* c) {
  if (c->left_ == b && b->left_ == a)
    return {true, &node::left_, &node::right_};
  if (c->right_ == b && b->right_ == a)
    return {true, &node::right_, &node::left_};
  return {false, &node::left_, &node::right_};
}

// b -> a <- c <- ... root
std::tuple<bool, node * node::*, node * node::*> correctZigZag(node* a, node* b,
                                                               node* c) {
  if (c->left_ == b && b->right_ == a)
    return {true, &node::left_, &node::right_};
  if (c->right_ == b && b->left_ == a)
    return {true, &node::right_, &node::left_};
  return {false, &node::left_, &node::right_};
}

void splay_(node*& tree, node* v) {
  if (v == nullptr) return;
  while (parent(parent(parent(v)))) {
    auto a = v;
    auto b = parent(a);
    auto c = parent(b);
    auto& tree = parentLink(c);
    if (std::get<0>(correctZigZig(a, b, c))) {
      auto [_, left, right] = correctZigZig(a, b, c);
      zigZigImpl(a, b, c, tree, left, right);
    } else if (std::get<0>(correctZigZag(a, b, c))) {
      auto [_, left, right] = correctZigZag(a, b, c);
      zigZagImpl(a, b, c, tree, left, right);
    } else
      assert(false);
    v = a;
  }
  if (parent(v) == nullptr) return;
  if (parent(parent(v)) == nullptr) {
    auto a = v;
    auto b = parent(a);
    auto [left, right] = correctPtr(a, b);
    smallRotateImpl(a, b, tree, left, right);
    v = a;
  } else if (parent(parent(parent(v))) == nullptr) {
    auto a = v;
    auto b = parent(a);
    auto c = parent(b);
    if (std::get<0>(correctZigZig(a, b, c))) {
      auto [_, left, right] = correctZigZig(a, b, c);
      zigZigImpl(a, b, c, tree, left, right);
    } else if (std::get<0>(correctZigZag(a, b, c))) {
      auto [_, left, right] = correctZigZag(a, b, c);
      zigZagImpl(a, b, c, tree, left, right);
    } else
      assert(false);
    v = a;
  } else
    assert(false);
}

void splayIdx(node*& t, ll k) {
  node* root = t;
  while (root) {
    push(root);
    if (size(root->left_) == k) break;
    if (size(root->left_) > k) {
      root = root->left_;
    } else {
      k -= size(root->left_) + 1;
      root = root->right_;
    }
  }
  splay_(t, root);
}

void merge(node*& t, node* add) {
  push(t);
  push(add);
  if (!t) {
    t = add;
    return;
  }
  if (!add) return;
  splay_idx(t, size(t) - 1);
  assert(size(t->left_) == size(t) - 1);
  link(add, t, &node::right_);
}

void splitSize(node*& t, node*& rem, ll idx) {
  if (idx >= size(t)) return;
  splay_idx(t, idx);
  rem = t->left_;
  if (rem) cut(rem, t, &node::left_);
  std::swap(t, rem);
}

void insert(node*& t, ll idx, ll value) {
  node* right = nullptr;
  split_size(t, right, idx);
  merge(t, new node(value));
  merge(t, right);
}

void print(node* t, std::string& s) {
  if (!t) return;
  push(t);
  print(t->left_, s);
  s += std::to_string(t->value);
  s += ' ';
  print(t->right_, s);
}

void toVector(node* t, std::vector<int64_t>& v) {
  if (!t) return;
  push(t);
  to_vector(t->left_, v);
  v.push_back(t->value);
  to_vector(t->right_, v);
}

node* findRoot(node* v) {
  while (parent(v)) {
    v = parent(v);
  }
  return v;
}

void makeRoot(node* const v) {
  node* cur = v;
  std::vector<node*> path;
  while (cur) {
    path.push_back(cur);
    cur = splay::parent(cur);
  }
  reverse(path.begin(), path.end());
  forEach(path) push(it);
  node* t = path.front();
  splay_(t, v);
}

ll idxOf(node* t) {
  make_root(t);
  push(t);
  return size(t->left_);
}

void assign(node* t, ll value) {
  push(t);
  t->needAssign = true;
  t->needAdd = false;
  t->addval = 0;
  doAssign(t, value);
}

void add(node* t, ll addval) {
  push(t);
  t->needAdd = true;
  doAdd(t, addval);
}

void reverse(node* t) {
  push(t);
  t->needReverse ^= true;
  doReverse(t);
}

void swapValues(node*& a, node*& b) {
  assert(parent(a) == nullptr && parent(b) == nullptr);
  node* la = a->left_;
  node* ra = a->right_;
  node* lb = b->left_;
  node* rb = b->right_;
  cut(la, a, &node::left_);
  cut(ra, a, &node::right_);
  cut(lb, b, &node::left_);
  cut(rb, b, &node::right_);
  std::swap(a, b);
  link(lb, b, &node::left_);
  link(rb, b, &node::right_);
  link(la, a, &node::left_);
  link(ra, a, &node::right_);
}

node* findNext(node* t, ll value, node* node::*left, node* node::*right) {
  push(t);
  if (t->value > value) {
    if (!(t->*left)) return t;
    if (back(t->*left, left, right) <= value) return t;
    node* better = find_next(t->*left, value, left, right);
    if (better->value > t->value) return t;
    return better;
  } else {
    if (!(t->*right)) return t;
    node* better = find_next(t->*right, value, left, right);
    return better;
  }
}

void next(node*& t, ll value, node* node::*left, node* node::*right) {
  node* best_match = find_next(t, value, left, right);
  make_root(best_match);
  t = best_match;
}

node* findPrev(node* t, ll value, node* node::*left, node* node::*right) {
  push(t);
  if (t->value < value) {
    if (!(t->*right)) return t;
    if (front(t->*right, left, right) >= value) return t;
    node* better = find_prev(t->*right, value, left, right);
    if (better->value < t->value) return t;
    return better;
  } else {
    if (!(t->*left)) return t;
    node* better = find_prev(t->*left, value, left, right);
    return better;
  }
}

void prev(node*& t, ll value, node* node::*left, node* node::*right) {
  node* best_match = find_prev(t, value, left, right);
  make_root(best_match);
  t = best_match;
}

void nextPermutation(node*& t) {
  if (suffixDown(t) == size(t)) {
    reverse(t);
    return;
  }
  node* suffix = nullptr;
  splay::split_size(t, suffix, size(t) - suffixDown(t));
  splay_idx(t, size(t) - 1);
  splay::next(suffix, t->value, &node::right_, &node::left_);
  splay::swap_values(t, suffix);
  splay::reverse(suffix);
  splay::merge(t, suffix);
}

void prevPermutation(node*& t) {
  if (suffixUp(t) == size(t)) {
    reverse(t);
    return;
  }
  node* suffix = nullptr;
  splay::split_size(t, suffix, size(t) - suffixUp(t));
  splay_idx(t, size(t) - 1);
  splay::prev(suffix, t->value, &node::left_, &node::right_);
  splay::swap_values(t, suffix);
  splay::reverse(suffix);
  splay::merge(t, suffix);
}
}  // namespace splay

class SplayTree {
  splay::node* root = nullptr;

  static std::tuple<splay::node*, splay::node*, splay::node*> subsegmentSplit(
      splay::node*& t, ll l, ll r) {
    splay::node* right = nullptr;
    splay::node* mid = nullptr;
    splay::split_size(t, right, r);
    splay::split_size(t, mid, l);
    return {t, mid, right};
  }

  static splay::node* subsegmentMerge(splay::node* left, splay::node* mid,
                                      splay::node* right) {
    splay::merge(left, mid);
    splay::merge(left, right);
    return left;
  }

  static void dfs_push(splay::node* t) {
    splay::push(t);
    if (t->left_) dfs_push(t->left_);
    if (t->right_) dfs_push(t->right_);
  }

 public:
  SplayTree() = default;
  ~SplayTree() { delete root; }

  void insert(ll idx, ll x) { splay::insert(root, idx, x); }

  size_t size() const { return splay::size(root); }

  void erase(ll idx) {
    auto [left, mid, right] = subsegment_split(root, idx, idx + 1);
    delete mid;
    splay::merge(left, right);
    root = left;
  }

  // [l; r)
  void assign(ll l, ll r, ll value) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    splay::assign(mid, value);
    root = subsegment_merge(left, mid, right);
  }

  // [l; r)
  void reverse(ll l, ll r) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    splay::reverse(mid);
    root = subsegment_merge(left, mid, right);
  }

  // [l; r)
  xll sum(ll l, ll r) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    xll ans = splay::sum(mid);
    root = subsegment_merge(left, mid, right);
    return ans;
  }

  void add(ll l, ll r, xll x) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    splay::add(mid, x);
    root = subsegment_merge(left, mid, right);
  }

  void nextPermutation(int64_t l, int64_t r) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    splay::next_permutation(mid);
    root = subsegment_merge(left, mid, right);
  }

  void prevPermutation(int64_t l, int64_t r) {
    auto [left, mid, right] = subsegment_split(root, l, r);
    splay::prev_permutation(mid);
    root = subsegment_merge(left, mid, right);
  }

  std::string toString() {
    std::string s;
    splay::print(root, s);
    if (!s.empty()) s.pop_back();
    return s;
  }

  std::vector<ll> toVector() {
    std::vector<ll> a;
    splay::to_vector(root, a);
    return a;
  }

  void pushBack(int64_t v) { splay::merge(root, new splay::node(v)); }

  void pushAll() { dfs_push(root); }
};

void solve() {
  ll n, q;
  std::cin >> n;
  SplayTree t;
  while (n--) {
    ll a;
    std::cin >> a;
    t.push_back(a);
  }
  std::cin >> q;
  while (q--) {
    ll type;
    std::cin >> type;
    ll l, r, x, pos;
    switch (type) {
      case 1:
        std::cin >> l >> r;
        std::cout << t.sum(l, r + 1) << '\n';
        break;
      case 2:
        std::cin >> x >> pos;
        t.insert(pos, x);
        break;
      case 3:
        std::cin >> pos;
        t.erase(pos);
        break;
      case 4:
        std::cin >> x >> l >> r;
        t.assign(l, r + 1, x);
        break;
      case 5:
        std::cin >> x >> l >> r;
        t.add(l, r + 1, x);
        break;
      case 6:
        std::cin >> l >> r;
        t.next_permutation(l, r + 1);
        break;
      case 7:
        std::cin >> l >> r;
        t.prev_permutation(l, r + 1);
        break;
      default:
        panic();
    }
  }
  std::cout << t.to_string() << '\n';
}

int main() {
  std::ios::sync_with_stdio(false), std::cin.tie(nullptr),
      std::cout.tie(nullptr);
  std::cout << std::setprecision(10) << std::fixed;
#ifdef LOCAL
  auto start = std::chrono::system_clock::now();
#endif
  solve();
#ifdef LOCAL
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<ld> elapsed_seconds = end - start;
  std::cout << "\ntime: " << elapsed_seconds.count() << " s\n";
#endif
}
