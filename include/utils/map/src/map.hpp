/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less <Key>
> class map {
 public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef pair<const Key, T> value_type;
  class Node {
    public:
    value_type value;
    Node *left,*right;
    Node* parent;//用于表示父亲节点
    int height;//用于计算平衡度
    //int balance_value;//平衡度,只能为-1 0 1
    //Node():left(nullptr),right(nullptr),height(1){}
    Node(const value_type& x, Node* p) : value(x), left(nullptr), right(nullptr), parent(p), height(1) {}
  };

  Node* root;
  size_t size_;
  Compare cmp;

  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */
  class const_iterator;
  class iterator {
    friend class const_iterator;
    friend class map;
   private:
    Node* n;
    map* owner;
    /**
     * TODO add data members
     *   just add whatever you want.
     */
   public:
    iterator() {
      // TODO
      n = nullptr;
      owner = nullptr;
    }
    iterator(Node* m,map* ma) {
      n = m;
      owner = ma;
    }
    iterator(const iterator &other) {
      this->n = other.n;
      this->owner = other.owner;
    }

    /**
     * TODO iter++
     */
    iterator operator++(int) {
      if (owner == nullptr || n == nullptr) throw invalid_iterator();
      iterator old = *this;
      n = owner->next(n);
      return old;
    }

    /**
     * TODO ++iter
     */
    iterator &operator++() {
      if (owner == nullptr || n == nullptr) throw invalid_iterator();
      n = owner->next(n);
      return *this;
    }

    /**
     * TODO iter--
     */
    iterator operator--(int) {
      if (owner == nullptr) throw invalid_iterator();
      iterator old = *this;
      if (n == nullptr) {
        Node* cur = owner->root;
        if (cur == nullptr) throw invalid_iterator();
        while (cur->right) cur = cur->right;
        n = cur;
        return old;
      }
      Node* p = owner->prev(n);
      if (p == nullptr) throw invalid_iterator();
      n = p;
      return old;
    }

    /**
     * TODO --iter
     */
    iterator &operator--() {
      if (owner == nullptr) throw invalid_iterator();
      if (n == nullptr) {
        Node* cur = owner->root;
        if (cur == nullptr) throw invalid_iterator();
        while (cur->right) cur = cur->right;
        n = cur;
        return *this;
      }
      Node* p = owner->prev(n);
      if (p == nullptr) throw invalid_iterator();
      n = p;
      return *this;
    }
    /**
     * a operator to check whether two iterators are same (pointing to the same memory).
     */
    value_type &operator*() const {
      if(!n) throw invalid_iterator();
      return n->value;
    }

    bool operator==(const iterator &rhs) const {return (n == rhs.n && owner == rhs.owner);}
    bool operator==(const const_iterator &rhs) const {return (n == rhs.n && owner == rhs.owner);}

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {return !(n == rhs.n && owner == rhs.owner);}
    bool operator!=(const const_iterator &rhs) const {return !(n == rhs.n && owner == rhs.owner);}

    /**
     * for the support of it->first.
     * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
     */
    value_type *operator->() const {
      if(!n) throw invalid_iterator();
      return &(n->value);
    }
  };

  class const_iterator {
    friend class iterator;
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
   private:
    // data members.
    Node* n;
    const map* owner;

   public:
    const_iterator() {
      // TODO
      n = nullptr;
      owner = nullptr;
    }

    const_iterator(Node* m, const map* ma) {
      n = m;
      owner = ma;
    }

    const_iterator(const const_iterator &other) {
      // TODO
      this->n = other.n;
      this->owner = other.owner;
    }

    const_iterator(const iterator &other) {
      // TODO
      this->n = other.n;
      this->owner = other.owner;
    }

    const_iterator operator++(int) {
      if (owner == nullptr || n == nullptr) throw invalid_iterator();
      const_iterator old = *this;
      n = owner->next(n);
      return old;
    }

    const_iterator &operator++() {
      if (owner == nullptr || n == nullptr) throw invalid_iterator();
      n = owner->next(n);
      return *this;
    }

    const_iterator operator--(int) {
      if (owner == nullptr) throw invalid_iterator();
      const_iterator old = *this;
      if (n == nullptr) {
        Node* cur = owner->root;
        if (cur == nullptr) throw invalid_iterator();
        while (cur->right) cur = cur->right;
        n = cur;
        return old;
      }
      Node* p = owner->prev(n);
      if (p == nullptr) throw invalid_iterator();
      n = p;
      return old;
    }

    const_iterator &operator--() {
      if (owner == nullptr) throw invalid_iterator();
      if (n == nullptr) {
        Node* cur = owner->root;
        if (cur == nullptr) throw invalid_iterator();
        while (cur->right) cur = cur->right;
        n = cur;
        return *this;
      }
      Node* p = owner->prev(n);
      if (p == nullptr) throw invalid_iterator();
      n = p;
      return *this;
    }

    const value_type &operator*() const {
      if(!n) throw invalid_iterator();
      return n->value;
    }

    bool operator==(const const_iterator &rhs) const {return (n == rhs.n && owner == rhs.owner);}
    bool operator!=(const const_iterator &rhs) const {return !(n == rhs.n && owner == rhs.owner);}

    const value_type *operator->() const {
      if(!n) throw invalid_iterator();
      return &(n->value);
    }
  };

  /**
   * TODO two constructors
   */
  map() {
    root = nullptr;
    size_ = 0;
  }

  map(const map &other) {
    root = copy_node(other.root, nullptr);
    size_ = other.size_;
    cmp = other.cmp;
  }

  /**
   * TODO assignment operator
   */
  map &operator=(const map &other) {
    if (this == &other) return *this;
    clear();
    root = copy_node(other.root, nullptr);
    size_ = other.size_;
    cmp = other.cmp;
    return *this;
  }

  /**
   * TODO Destructors
   */
  void clear_(Node* x){
    if(!x) return ;
    clear_(x->left);
    clear_(x->right);
    delete x;
  }

  ~map() {
    clear_(root);
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key equivalent to key.
   * If no such element exists, an exception of type `index_out_of_bound'
   */
  T &at(const Key &key) {
    Node* tmp = find_(key);
    if(tmp) return tmp->value.second;
    else throw index_out_of_bound();
  }

  const T &at(const Key &key) const {
    Node* tmp = find_(key);
    if(tmp) return tmp->value.second;
    else throw index_out_of_bound();
  }

  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to key,
   *   performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    Node* tmp = find_(key);
    if(tmp) return tmp->value.second;
    value_type v(key, T());
    pair<iterator, bool> ret = insert(v);
    return ret.first->second;
  }

  const T &operator[](const Key &key) const {
    Node* tmp = find_(key);
    if(tmp) return tmp->value.second;
    else throw index_out_of_bound();
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {//按关键字的大小排序
    Node* cur = root;
    if (!cur) return end();
    while (cur->left) cur = cur->left;
    return iterator(cur, this);
  }

  const_iterator cbegin() const {
    Node* cur = root;
    if (!cur) return cend();
    while (cur->left) cur = cur->left;
    return const_iterator(cur, this);
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() {
    return iterator(nullptr,this);
  }

  const_iterator cend() const {
    return const_iterator(nullptr,this);
  }

  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const {
    return (size_ == 0);
  }

  /**
   * returns the number of elements.
   */
  size_t size() const {
    return size_;
  }

  /**
   * clears the contents
   */
  void clear() {
    clear_(root);
    root = nullptr;
    size_ = 0;
  }

  /**
   * insert an element.
   * return a pair, the first of the pair is
   *   the iterator to the new element (or the element that prevented the insertion),
   *   the second one is true if insert node_nextessfully, or false.
   */
  pair<iterator, bool> insert(const value_type &value) {
    Node* tmp = insert_(value, root, nullptr); 
    if (tmp) {
      return pair<iterator, bool>(iterator(tmp, this), true);
    } 
    else {
      Node* exist = find_(value.first);
      return pair<iterator, bool>(iterator(exist, this), false);
    }
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
   */
  void erase(iterator pos) {
    if (pos.owner != this || pos == end() || pos.n == nullptr) throw invalid_iterator();
    erase_(pos.n);
    --size_;
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    if(find_(key)) return 1;
    else return 0;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is returned.
   */
  iterator find(const Key &key) {
    Node* tmp = find_(key);
    if(tmp) return iterator(tmp,this);
    else return end();
  }

  const_iterator find(const Key &key) const {
    Node* tmp = find_(key);
    if(tmp) return const_iterator(tmp,this);
    else return cend();
  }

 private:
  Node* copy_node(Node* other_node, Node* parent) {//构造函数，辅助深拷贝
    if (!other_node) return nullptr;
    Node* new_node = new Node(other_node->value, parent);
    new_node->height = other_node->height;
    new_node->left = copy_node(other_node->left, new_node);
    new_node->right = copy_node(other_node->right, new_node);
    return new_node;
  }

  int h(Node* n){
    if(n == nullptr) return 0;
    return n->height;
  }

  int update(Node* n){
    if(h(n->left) < h(n->right)) return h(n->right)+1;
    else return h(n->left)+1;
  }

  //这些是旋转函数，辅助调整平衡
  void ll(Node*& x){//左边特别重，右转
    Node* y = x->left;
    x->left = y->right;
    if(y->right) y->right->parent = x;
    y->parent = x->parent;
    y->right = x;
    x->parent = y;
    x->height = update(x);
    y->height = update(y);
    x = y;
  }
  
  void rr(Node*& x){//右重，左转
    Node* y = x->right;
    x->right = y->left;
    if(y->left) y->left->parent = x;
    y->parent = x->parent;
    y->left = x;
    x->parent = y;
    x->height = update(x);
    y->height = update(y);
    x = y;
  }
  
  void lr(Node*& x) { 
    rr(x->left); 
    ll(x);
  }
  
  void rl(Node*& x) {
    ll(x->right);
    rr(x);
  }
  
  Node* insert_(const value_type &v, Node*& node, Node* parent) {//插入从上往下，自动规划好平衡
    if (node == nullptr) {
      size_++;
      node = new Node(v, parent);
      return node;
    }
    Node* res = nullptr;
    if (cmp(v.first, node->value.first)) {
      res = insert_(v, node->left, node);
      if (h(node->left) - h(node->right) == 2) {
        if (cmp(v.first, node->left->value.first)) ll(node);
        else lr(node);
      }
    } 
    else if (cmp(node->value.first, v.first)) {
      res = insert_(v, node->right, node);
      if (h(node->right) - h(node->left) == 2) {
        if (cmp(node->right->value.first, v.first)) rr(node);
        else rl(node);
      }
    } 
    else return nullptr;
    node->height = update(node);
    return res;
  }
  
  Node* find_(const Key& key) const {
    Node* tmp = root;
    while(tmp) {
      if(cmp(key, tmp->value.first)) tmp = tmp->left;
      else if(cmp(tmp->value.first, key)) tmp = tmp->right;
      else {
        return tmp;
      }
    }
    return nullptr;
  }

  //辅助找下一个或上一个位置
  Node* next(Node* node) const {
    if (!node) return nullptr;
    if (node->right) {
      Node* tmp = node->right;
      while (tmp->left) tmp = tmp->left;
      return tmp;
    }
    Node* p = node->parent;
    while (p && node == p->right) {
      node = p;
      p = p->parent;
    }
    return p;
  }
  
  Node* prev(Node* node) const {
    if (!node) return nullptr;
    if (node->left) {
      Node* tmp = node->left;
      while (tmp->right) tmp = tmp->right;
      return tmp;
    }
    Node* p = node->parent;
    while (p && node == p->left) {
      node = p;
      p = p->parent;
    }
    return p;
  }

  //用于erase维持平衡
  void rebalance(Node* p) {//从下往上保持平衡
    while (p) {
      p->height = update(p);
      int balance = h(p->left) - h(p->right);
      if (balance == 2) {
        if (h(p->left->left) >= h(p->left->right)) ll(get_parent(p));
        else lr(get_parent(p));
      } 
      else if (balance == -2) {
        if (h(p->right->right) >= h(p->right->left)) rr(get_parent(p));
        else rl(get_parent(p));
      }
      p = p->parent;
    }
  }

  Node*& get_parent(Node* p) {//返回父节点指原先指向的位置
    if (!p->parent) return root;
    if (p->parent->left == p) return p->parent->left;
    else return p->parent->right;
  }

  void erase_(Node* node) {
    if (!node) return;
    if (node->left && node->right) {//有两个孩子，找它后继最小的，然后替换原来位置
      Node* node_next = node->right;
      while (node_next->left) node_next = node_next->left;

      Node* node_parent = node->parent;
      Node* node_left = node->left;
      Node* node_right = node->right;

      Node* node_next_parent = node_next->parent;
      Node* node_next_right = node_next->right;

      if (node_parent) {
        if (node_parent->left == node) node_parent->left = node_next;
        else node_parent->right = node_next;
      } 
      else {
        root = node_next;
      }

      if (node_next_parent == node) {//如果就是它的儿子
        node_next->parent = node_parent;
        node_next->left = node_left;
        if (node_left) node_left->parent = node_next;
        
        node_next->right = node;
        node->parent = node_next;

        node->left = nullptr;
        node->right = node_next_right;
        if (node_next_right) node_next_right->parent = node;
      } 
      else {//不是儿子，就涉及更多节点的交换
        if (node_next_parent->left == node_next) node_next_parent->left = node;
        else node_next_parent->right = node;

        node_next->parent = node_parent;
        node_next->left = node_left;
        if (node_left) node_left->parent = node_next;
        node_next->right = node_right;
        if (node_right) node_right->parent = node_next;

        node->parent = node_next_parent;
        node->left = nullptr;
        node->right = node_next_right;
        if (node_next_right) node_next_right->parent = node;
      }

      int temp_h = node->height;
      node->height = node_next->height;
      node_next->height = temp_h;
    }

    Node* child = node->left ? node->left : node->right;//此时转换为只有一个孩子的情况，直接改变父节点指针
    Node* p = node->parent;

    if (child) child->parent = p;

    if (!p) {
      root = child;
    } 
    else {
      if (p->left == node) p->left = child;
      else p->right = child;
    }

    delete node;
    rebalance(p);
  }
};

}

#endif