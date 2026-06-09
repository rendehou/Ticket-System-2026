#ifndef SJTU_BPT_HPP
#define SJTU_BPT_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include "../utils/vector/vector.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class MemoryRiver {
private:
    /* your code here */
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
    void ensure_open() {
        if (!file.is_open()) file.open(file_name,std::ios::in|std::ios::out|std::ios::binary);
    }
public:
    MemoryRiver() = default;
    MemoryRiver(const string& file_name) : file_name(file_name) {}
    ~MemoryRiver() { if (file.is_open()) file.close(); }

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        if (file.is_open()) file.close();
        bool exists = false;
        if (!file_name.empty()) {
            ifstream probe(file_name, std::ios::binary);
            exists = probe.good();
        }
        if (!exists) {
            file.open(file_name, std::ios::out|std::ios::binary|std::ios::trunc);
            int tmp = 0;
            for (int i = 0; i < info_len; ++i)
                file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
            file.close();
        }
        file.open(file_name, std::ios::in|std::ios::out|std::ios::binary);
    }

    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekg((n-1)*sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    void write_info(int tmp, int n) {
        if (n > info_len) return;
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekp((n-1)*sizeof(int));
        file.write(reinterpret_cast<char *>(&tmp),sizeof(int));
    }

    int write(T &t) {
        ensure_open();
        if(!file.is_open()) return 0;
        file.clear();
        file.seekp(0,std::ios::end);
        int index = file.tellp();
        file.write(reinterpret_cast<char *>(&t),sizeofT);
        return index;
    }

    void update(T &t, const int index) {
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekp(index);
        file.write(reinterpret_cast<char *>(&t),sizeofT);
    }

    void read(T &t, const int index) {
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekg(index);
        file.read(reinterpret_cast<char *>(&t),sizeofT);
    }

    void Delete(int index) { /* your code here */ }
};

template<class Key, class Value, int N = 50>
struct Data {//存储键值对
    Key key;
    Value value;
    Data() = default;
    Data(const Key& k) : key(k) {}
    Data(const Key& k, const Value& v) : key(k), value(v) {}
    Data &operator=(const Data& other) {
        if (this == &other) return *this;
        key = other.key;
        value = other.value;
        return *this;
    }
    bool operator<(const Data& other) const {
        if(key < other.key || (key == other.key && value < other.value)) return true;
        return false;
    }
    bool operator==(const Data& other) const {
        return key == other.key && value == other.value;
    }
    bool operator<=(const Data& other) const {
        return *this < other || *this == other;
    }
    bool operator>(const Data& other) const {
        return other < *this;
    }
    bool operator>=(const Data& other) const {
        return other <= *this;
    }
};

template<class Key, class Value, int N = 50>
struct Node {//树的节点
    bool is_leaf = 0;
    int size = 0;
    int parent = -1;
    int children[N+2];
    int right = -1;
    int left = -1;
    // 原始字节存储，避免默认构造清零
    alignas(Data<Key, Value, N>) char raw[(N+1) * sizeof(Data<Key, Value, N>)];
    Data<Key, Value, N>& d(int i) { return reinterpret_cast<Data<Key, Value, N>&>(raw[i * sizeof(Data<Key, Value, N>)]); }
    const Data<Key, Value, N>& d(int i) const { return reinterpret_cast<const Data<Key, Value, N>&>(raw[i * sizeof(Data<Key, Value, N>)]); }
};

template<class Key, class Value, int N = 50>
class bpt{
    private:
        MemoryRiver<Node<Key, Value, N>,2> mr;
        int root = -1;
        Node<Key, Value, N> cached_root;
        bool root_cached = false;
        
        void refresh_root_cache() {
            if (root <= 0) { root_cached = false; return; }
            mr.read(cached_root, root);
            root_cached = true;
        }
        
        void read_root(Node<Key, Value, N>& node) {
            if (root_cached) node = cached_root;
            else if (root > 0) { mr.read(node, root); cached_root = node; root_cached = true; }
        }
        
    public:
        bpt(const string& filename = "bpt"){
            mr.initialise(filename);
            mr.get_info(root,1);
            if (root <= 0) root = -1;
            if (root > 0) refresh_root_cache();
        }
        ~bpt(){
            mr.write_info(root,1);
        }
        bool empty() const { return root <= 0; }
        void insert(const Key& key, const Value& value);
        void remove(const Key& key, const Value& value);
        sjtu::vector<Value> find_all(const Key& key);//返回所有匹配的value
        void split_Node(int node_index);
        int find_(const Key& key, const Value& value);//返回找到的位置（节点index）
        bool find_value(const Key& key, Value& out);//直接返回值，跳过vector
        void find_values_batch(const Key* keys, Value* values, bool* found, int count);//批量查，keys有序
        void merge_node(int node_index);
};
template<class Key, class Value, int N = 50>
int find_in_Node(const Node<Key, Value, N>& node, const Data<Key, Value, N> data_) {
    //这里说明，一共N个标记，对应N+1棵子树，其中对第i个标记，它对应的子树在它前方，子树里的key都小等标记
    int l = 0, r = node.size - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        if (node.d(mid) < data_) l = mid + 1;
        else if (node.d(mid) > data_) r = mid - 1;
        else return mid;
    }
    return l;//返回node中第一个大于等于data的位置，如果没有则返回node尾部
}

template<class Key, class Value, int N = 50>
int find_child_in_Node(const Node<Key, Value, N>& node, const Data<Key, Value, N> data_) {
    //内部节点向下走时用upper_bound，返回第一个大于data的位置
    int l = 0, r = node.size - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        if (node.d(mid) <= data_) l = mid + 1;
        else r = mid - 1;
    }
    return l;
}

template<class Key, class Value, int N>
void bpt<Key, Value, N>::split_Node(int node_index){
    Node<Key, Value, N> node;
    mr.read(node, node_index);

    Node<Key, Value, N> new_splited_Node;//在右侧新分出一块
    int new_splited_Node_index;
    Data<Key, Value, N> up_data;
    new_splited_Node.is_leaf = node.is_leaf;

    if(node.is_leaf) {//是叶子就只改变data

        int mid = node.size / 2;
        for(int i = mid; i < node.size; i++) {
            new_splited_Node.d(i-mid) = node.d(i);
        }
        new_splited_Node.size = node.size - mid;
        node.size = mid;

        //修改指针
        new_splited_Node.left = node_index;
        new_splited_Node.right = node.right;
        new_splited_Node.parent = node.parent;
        new_splited_Node_index = mr.write(new_splited_Node);
        node.right = new_splited_Node_index;
        mr.update(node, node_index);

        //如果有后继要修改后继指针
        if(new_splited_Node.right != -1) {
            Node<Key, Value, N> node_right;
            mr.read(node_right, new_splited_Node.right);
            node_right.left = new_splited_Node_index;
            mr.update(node_right, new_splited_Node.right);
        }
        up_data = new_splited_Node.d(0);

    } else {

        //基本信息
        int mid = node.size / 2;
        int right_size = node.size - mid - 1;
        up_data = node.d(mid);
        new_splited_Node.is_leaf = 0;
        new_splited_Node.parent = node.parent;

        //data和children
        for(int i = 0; i < right_size; i++) {
            new_splited_Node.d(i) = node.d(mid + 1 + i);
        }
        for(int i = 0; i < right_size + 1; i++) {
            new_splited_Node.children[i] = node.children[mid + 1 + i];
        }
        new_splited_Node.size = right_size;
        node.size = mid;

        //修改左右指针
        new_splited_Node.right = node.right;
        new_splited_Node.left = node_index;
        new_splited_Node_index = mr.write(new_splited_Node);
        node.right = new_splited_Node_index;
        mr.update(node, node_index);
        if(new_splited_Node.right != -1) {
            Node<Key, Value, N> node_right;
            mr.read(node_right, new_splited_Node.right);
            node_right.left = new_splited_Node_index;
            mr.update(node_right, new_splited_Node.right);
        }

        //修改被分裂节点的子节点的父亲指针
        for(int i = 0; i < right_size + 1; i++) {
            int child_index = new_splited_Node.children[i];
            if(child_index == -1) continue;
            Node<Key, Value, N> child_node;
            mr.read(child_node, child_index);
            child_node.parent = new_splited_Node_index;
            mr.update(child_node, child_index);
        }

    }
        
    if(node.parent == -1){//要创造新根

        //建立新根
        Data<Key, Value, N> up_key = up_data;
        Node<Key, Value, N> new_root;//建立新根
        new_root.d(0) = up_key;
        new_root.size = 1;
        new_root.is_leaf = 0;
        new_root.parent = -1;
        new_root.children[0] = node_index;
        new_root.children[1] = new_splited_Node_index;

        root = mr.write(new_root);//写入新根并更新root
        mr.write_info(root,1);
        refresh_root_cache();
        node.parent = root;
        new_splited_Node.parent = root;
        mr.update(node, node_index);
        mr.update(new_splited_Node, new_splited_Node_index);
        return;
    }

    else {//不是孤点，直接把提升的key插入父亲节点
        Node<Key, Value, N> parent_node;
        mr.read(parent_node, node.parent);//读出父亲节点
        int id_in_parent = find_in_Node(parent_node, up_data);

        //移动数据挪位置
        for(int i = parent_node.size; i > id_in_parent; i--) {
            parent_node.d(i) = parent_node.d(i-1);
        }
        for(int i = parent_node.size + 1; i > id_in_parent + 1; i--) { //children单独右移，避免越界
            parent_node.children[i] = parent_node.children[i-1];
        }

        parent_node.d(id_in_parent) = up_data;
        parent_node.children[id_in_parent+1] = new_splited_Node_index;
        parent_node.size++;
        new_splited_Node.parent = node.parent;//修改新分裂块的父亲指针
        mr.update(new_splited_Node, new_splited_Node_index);//写回新分裂块

        if(parent_node.size > N) {//父亲节点个数超了
            mr.update(parent_node, node.parent);//先把父亲节点写回去，保证分裂时读到的是最新的父亲节点
            split_Node(node.parent);
            return;
        }

        else {
            mr.update(parent_node, node.parent);
            return;
        }

    }
}

template<class Key, class Value, int N>
void bpt<Key, Value, N>::insert(const Key& key, const Value& value) {

    Data<Key, Value, N> data = Data<Key, Value, N>(key, value);

    if(root <= 0) {
        Node<Key, Value, N> new_node;
        new_node.is_leaf = 1;//插入根,是根又是叶
        new_node.d(0) = data;//data 0-based
        new_node.size = 1;
        root = mr.write(new_node);
        mr.write_info(root,1);
        refresh_root_cache();
        return;
    }

    int current_index = root;
    Node<Key, Value, N> current_Node;
    read_root(current_Node);//读出根，从根开始匹配

    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    //跳出循环的时候已经在叶子
    int id = find_in_Node(current_Node, data);
    if(id < current_Node.size && current_Node.d(id) == data) return;
    
    //腾出位置
    for(int i = current_Node.size; i > id; i--) {
        current_Node.d(i) = current_Node.d(i-1);
    } 
    current_Node.d(id) = data;
    current_Node.size++;

    //处理块的大小
    if(current_Node.size <= N) {
        mr.update(current_Node, current_index);
    }
    else {//需要分裂块 
        mr.update(current_Node, current_index);
        split_Node(current_index);
    }
    refresh_root_cache();
}

template<class Key, class Value, int N>
sjtu::vector<Value> bpt<Key, Value, N>::find_all(const Key& key) {

    sjtu::vector<Value> result;
    Data<Key, Value, N> data_l(key);

    if(root <= 0) {
        return result;
    }

    int current_index = root;
    Node<Key, Value, N> current_Node;
    read_root(current_Node);//读出根，从根开始匹配，这个与insert类似
    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data_l);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    bool stop = false;
    while(true) {//这里要沿着叶节点不断循环匹配
        int id = find_in_Node(current_Node, data_l);
        for(int i = id; i < current_Node.size; i++) {//从第一个大于等于data的位置开始匹配
            if(current_Node.d(i).key == key) {
                result.push_back(current_Node.d(i).value);
            } else { //已经不是一个关键词了，停下
                stop = true; 
                break; 
            }
        }
        if(stop || current_Node.right == -1) break;//如果没有右兄弟了就结束
        mr.read(current_Node,current_Node.right);//更新到右兄弟继续匹配
    }
    return result;
}

template<class Key, class Value, int N>
int bpt<Key, Value, N>::find_(const Key& key, const Value& value) {//重载find函数，返回一个Node，如果没有找到就返回一个空Node
    Data<Key, Value, N> data = Data<Key, Value, N>(key, value);
    int current_index = root;
    Node<Key, Value, N> current_Node;
    if(root <= 0) {
        return -1;
    }

    read_root(current_Node);//读出根，从根开始匹配，这个与insert类似
    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    int id = find_in_Node(current_Node, data);
    if(id < current_Node.size && current_Node.d(id) == data) return current_index;
    
    return -1;
}

template<class Key, class Value, int N>
bool bpt<Key, Value, N>::find_value(const Key& key, Value& out) {
    if (root <= 0) return false;
    int current_index = root;
    Node<Key, Value, N> current_Node;
    read_root(current_Node);
    while (!current_Node.is_leaf) {
        // 只比较 key 找子节点（避免值比较导致错误分支）
        int id = 0;
        while (id < current_Node.size && !(key < current_Node.d(id).key)) id++;
        current_index = current_Node.children[id];
        mr.read(current_Node, current_index);
    }
    // 叶子节点线性扫描
    for (int i = 0; i < current_Node.size; i++) {
        if (current_Node.d(i).key == key) {
            out = current_Node.d(i).value;
            return true;
        }
    }
    return false;
}

// 批量查找：keys必须升序。找到keys[0]的叶子后沿right指针走，一次遍历匹配全部
template<class Key, class Value, int N>
void bpt<Key, Value, N>::find_values_batch(const Key* keys, Value* values, bool* found, int count) {
    if (root <= 0 || count <= 0) return;
    for (int i = 0; i < count; i++) found[i] = false;
    
    // 找包含keys[0]的叶子
    int cur = root;
    Node<Key, Value, N> node;
    read_root(node);
    while (!node.is_leaf) {
        int id = 0;
        while (id < node.size && !(keys[0] < node.d(id).key)) id++;
        cur = node.children[id];
        mr.read(node, cur);
    }
    
    int ki = 0;
    while (cur != -1 && ki < count) {
        // 跳过整个叶子如果它的max < 当前要找的key
        if (node.size > 0 && node.d(node.size - 1).key < keys[ki]) {
            cur = node.right;
            if (cur != -1) mr.read(node, cur);
            continue;
        }
        for (int i = 0; i < node.size && ki < count; i++) {
            const Key& lk = node.d(i).key;
            while (ki < count && keys[ki] < lk) ki++;
            if (ki >= count) break;
            if (keys[ki] == lk) {
                values[ki] = node.d(i).value;
                found[ki] = true;
                ki++;
            }
        }
        cur = node.right;
        if (cur != -1) mr.read(node, cur);
    }
}

template<class Key, class Value, int N>
void bpt<Key, Value, N>::merge_node(int node_index) {
    Node<Key, Value, N> node;
    mr.read(node, node_index);
    if(node.parent == -1) {
        //如果是根节点
        if(!node.is_leaf && node.size == 0) {
            //根节点为空，且有一个孩子，则孩子成为新根
            root = node.children[0];
            Node<Key, Value, N> root_node;
            mr.read(root_node, root);
            root_node.parent = -1;
            mr.update(root_node, root);
            mr.write_info(root, 1);
            refresh_root_cache();
        } else if(node.size == 0) {
            root = -1;
            mr.write_info(root, 1);
            root_cached = false;
        }
        return;
    }

    Node<Key, Value, N> parent_node;
    mr.read(parent_node, node.parent);
    int child_pos = -1;
    for(int i = 0; i <= parent_node.size; i++) {
        if(parent_node.children[i] == node_index) {
            child_pos = i;
            break;
        }
    }

    Node<Key, Value, N> left_node, right_node;
    bool has_left = (child_pos > 0);
    bool has_right = (child_pos < parent_node.size);

    if(has_left) mr.read(left_node, parent_node.children[child_pos - 1]);
    if(has_right) mr.read(right_node, parent_node.children[child_pos + 1]);

    //尝试从左兄弟借
    if(has_left && left_node.size > N/2) {
        if(node.is_leaf) {
            for(int i = node.size; i > 0; i--) node.d(i) = node.d(i-1);
            node.d(0) = left_node.d(left_node.size - 1);
            node.size++;
            left_node.size--;
            parent_node.d(child_pos - 1) = node.d(0);
        } else {//内部节点要借一个分界值下来,然后
            for(int i = node.size; i > 0; i--) node.d(i) = node.d(i-1);
            for(int i = node.size + 1; i > 0; i--) node.children[i] = node.children[i-1];
            node.d(0) = parent_node.d(child_pos - 1);
            node.children[0] = left_node.children[left_node.size];
            parent_node.d(child_pos - 1) = left_node.d(left_node.size - 1);
            node.size++;
            left_node.size--;

            //更新借来的孩子的父亲指针
            Node<Key, Value, N> child_node;
            mr.read(child_node, node.children[0]);
            child_node.parent = node_index;
            mr.update(child_node, node.children[0]);
        }
        mr.update(node, node_index);
        mr.update(left_node, parent_node.children[child_pos - 1]);
        mr.update(parent_node, node.parent);
        return;
    }

    //尝试从右兄弟借
    if(has_right && right_node.size > N/2) {
        if(node.is_leaf) {
            node.d(node.size) = right_node.d(0);
            node.size++;
            for(int i = 0; i < right_node.size - 1; i++) right_node.d(i) = right_node.d(i+1);
            right_node.size--;
            parent_node.d(child_pos) = right_node.d(0);
        } else {
            node.d(node.size) = parent_node.d(child_pos);
            node.children[node.size + 1] = right_node.children[0];
            parent_node.d(child_pos) = right_node.d(0);
            node.size++;
            for(int i = 0; i < right_node.size - 1; i++) right_node.d(i) = right_node.d(i+1);
            for(int i = 0; i < right_node.size; i++) right_node.children[i] = right_node.children[i+1];
            right_node.size--;

            //更新借来的孩子的父亲指针
            Node<Key, Value, N> child_node;
            mr.read(child_node, node.children[node.size]);
            child_node.parent = node_index;
            mr.update(child_node, node.children[node.size]);
        }
        mr.update(node, node_index);
        mr.update(right_node, parent_node.children[child_pos + 1]);
        mr.update(parent_node, node.parent);
        return;
    }

    //和左兄弟合并
    if(has_left) {
        int left_index = parent_node.children[child_pos - 1];
        if(node.is_leaf) {
            for(int i = 0; i < node.size; i++) {
                left_node.d(left_node.size + i) = node.d(i);
            }
            left_node.size += node.size;
            left_node.right = node.right;
            if(node.right != -1) {
                Node<Key, Value, N> next_node;
                mr.read(next_node, node.right);
                next_node.left = left_index;
                mr.update(next_node, node.right);
            }
        } else {
            left_node.d(left_node.size) = parent_node.d(child_pos - 1);
            for(int i = 0; i < node.size; i++) {
                left_node.d(left_node.size + 1 + i) = node.d(i);
            }
            for(int i = 0; i <= node.size; i++) {
                left_node.children[left_node.size + 1 + i] = node.children[i];
                if(node.children[i] != -1) {
                    Node<Key, Value, N> child_node;
                    mr.read(child_node, node.children[i]);
                    child_node.parent = left_index;
                    mr.update(child_node, node.children[i]);
                }
            }
            left_node.size += node.size + 1;
        }
        mr.update(left_node, left_index);

        for(int i = child_pos - 1; i < parent_node.size - 1; i++) {
            parent_node.d(i) = parent_node.d(i + 1);
        }
        for(int i = child_pos; i < parent_node.size; i++) {
            parent_node.children[i] = parent_node.children[i + 1];
        }
        parent_node.size--;
        mr.update(parent_node, node.parent);

        if(parent_node.size < N/2) {
            merge_node(node.parent);
        }
        return;
    }

    //和右兄弟合并
    if(has_right) {
        int right_index = parent_node.children[child_pos + 1];
        if(node.is_leaf) {
            for(int i = 0; i < right_node.size; i++) {
                node.d(node.size + i) = right_node.d(i);
            }
            node.size += right_node.size;
            node.right = right_node.right;
            if(right_node.right != -1) {
                Node<Key, Value, N> next_node;
                mr.read(next_node, right_node.right);
                next_node.left = node_index;
                mr.update(next_node, right_node.right);
            }
        } else {
            node.d(node.size) = parent_node.d(child_pos);
            for(int i = 0; i < right_node.size; i++) {
                node.d(node.size + 1 + i) = right_node.d(i);
            }
            for(int i = 0; i <= right_node.size; i++) {
                node.children[node.size + 1 + i] = right_node.children[i];
                if(right_node.children[i] != -1) {
                    Node<Key, Value, N> child_node;
                    mr.read(child_node, right_node.children[i]);
                    child_node.parent = node_index;
                    mr.update(child_node, right_node.children[i]);
                }
            }
            node.size += right_node.size + 1;
        }
        mr.update(node, node_index);

        for(int i = child_pos; i < parent_node.size - 1; i++) {
            parent_node.d(i) = parent_node.d(i + 1);
        }
        for(int i = child_pos + 1; i < parent_node.size; i++) {
            parent_node.children[i] = parent_node.children[i + 1];
        }
        parent_node.size--;
        mr.update(parent_node, node.parent);

        if(parent_node.size < N/2) {
            merge_node(node.parent);
        }
        return;
    }
}

template<class Key, class Value, int N>
void bpt<Key, Value, N>::remove(const Key& key, const Value& value){
    Data<Key, Value, N> data = Data<Key, Value, N>(key, value);
    int node_index = find_(key, value);
    if(node_index == -1) return;

    Node<Key, Value, N> node;
    mr.read(node, node_index);
    int id = find_in_Node(node, data);

    for(int i = id; i < node.size - 1; i++) {
        node.d(i) = node.d(i+1);
    }
    node.size--;
    mr.update(node, node_index);

    //若删除的是第一个元素且有父节点，需要向上更新父节点作为分界值的 data
    if(id == 0 && node.parent != -1 && node.size > 0) {
        int curr = node_index;
        while(curr != -1) {
            Node<Key, Value, N> curr_node;
            mr.read(curr_node, curr);
            if(curr_node.parent == -1) break;
            Node<Key, Value, N> parent_node;
            mr.read(parent_node, curr_node.parent);
            int child_pos = -1;
            for(int i = 0; i <= parent_node.size; i++) {
                if(parent_node.children[i] == curr) {
                    child_pos = i;
                    break;
                }
            }
            if(child_pos > 0) {
                parent_node.d(child_pos - 1) = node.d(0);
                mr.update(parent_node, curr_node.parent);
                break;
            }
            curr = curr_node.parent;
        }
    }

    if(node.parent == -1 && node.size == 0) {
        root = -1;
        mr.write_info(root, 1);
        root_cached = false;
        return;
    }

    if(node.size < N/2 && node.parent != -1) {
        merge_node(node_index);
    }
    refresh_root_cache();
}
#endif
