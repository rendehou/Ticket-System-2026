
#include <iostream>
#include <fstream>
#include <cstring>

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

    ~MemoryRiver() {
        if (file.is_open()) file.close();
    }

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

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekg((n-1)*sizeof(int));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
        /* your code here */
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekp((n-1)*sizeof(int));//注意一会来查一下这里是n还是n-1
        file.write(reinterpret_cast<char *>(&tmp),sizeof(int));
        /* your code here */
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        ensure_open();
        if(!file.is_open()) return 0;
        file.clear();
        file.seekp(0,std::ios::end);
        int index = file.tellp();
        file.write(reinterpret_cast<char *>(&t),sizeofT);
        return index;
        /* your code here */
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekp(index);
        file.write(reinterpret_cast<char *>(&t),sizeofT);
        /* your code here */
    }

    //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
    void read(T &t, const int index) {
        ensure_open();
        if(!file.is_open()) return;
        file.clear();
        file.seekg(index);
        file.read(reinterpret_cast<char *>(&t),sizeofT);
        /* your code here */
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        
        /* your code here */
    }
};

constexpr int N = 50;//N叉树

template<class Key, class Value>
struct Data {//存储键值对
    Key key;
    Value value;
    Data() {
        value = Value();
    }
    Data(const Key& k, const Value& v) {
        key = k;
        value = v;
    }
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

template<class Key, class Value>
struct Node {//树的节点
    bool is_leaf = 0;//是否为叶子节点
    Data<Key, Value> data[N+1];
    //多给一个的空间，这样在插入时就不需要考虑满了之后的情况了，直接插入，等插入完了再分裂，实际上每个节点范围为N/2到N
    int size = 0;//存了多少了,这里指的是标记
    int parent = -1;
    int children[N+2];//注意如果是叶节点就没有这些
    int right = -1;//右兄弟
    int left = -1;//左兄弟
    Node() = default;
};

template<class Key, class Value>
class bpt{
    private:
        MemoryRiver<Node<Key, Value>,2> mr;
        int root = -1;
        
    public:
        bpt(){
            mr.initialise("bpt");
            mr.get_info(root,1);
            if (root <= 0) root = -1;
        }
        ~bpt(){
            mr.write_info(root,1);
        }
        void insert(const Key& key, const Value& value);
        void remove(const Key& key, const Value& value);
        void find(const Key& key);
        void split_Node(int node_index);
        int find_(const Key& key, const Value& value);//返回找到的位置
        void merge_node(int node_index);
};

template<class Key, class Value>
int find_in_Node(const Node<Key, Value>& node, const Data<Key, Value> data_) {
    //这里说明，一共N个标记，对应N+1棵子树，其中对第i个标记，它对应的子树在它前方，子树里的key都小等标记
    int l = 0, r = node.size - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        if (node.data[mid] < data_) l = mid + 1;
        else if (node.data[mid] > data_) r = mid - 1;
        else return mid;
    }
    return l;//返回node中第一个大于等于data的位置，如果没有则返回node尾部
}

template<class Key, class Value>
int find_child_in_Node(const Node<Key, Value>& node, const Data<Key, Value> data_) {
    //内部节点向下走时用upper_bound，返回第一个大于data的位置
    int l = 0, r = node.size - 1;
    while (l <= r) {
        int mid = (l + r) / 2;
        if (node.data[mid] <= data_) l = mid + 1;
        else r = mid - 1;
    }
    return l;
}

template<class Key, class Value>
void bpt<Key, Value>::split_Node(int node_index){
    Node<Key, Value> node;
    mr.read(node, node_index);

    Node<Key, Value> new_splited_Node;//在右侧新分出一块
    int new_splited_Node_index;
    Data<Key, Value> up_data;
    new_splited_Node.is_leaf = node.is_leaf;

    if(node.is_leaf) {//是叶子就只改变data

        int mid = node.size / 2;
        for(int i = mid; i < node.size; i++) {
            new_splited_Node.data[i-mid] = node.data[i];
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
            Node<Key, Value> node_right;
            mr.read(node_right, new_splited_Node.right);
            node_right.left = new_splited_Node_index;
            mr.update(node_right, new_splited_Node.right);
        }
        up_data = new_splited_Node.data[0];

    } else {

        //基本信息
        int mid = node.size / 2;
        int right_size = node.size - mid - 1;
        up_data = node.data[mid];
        new_splited_Node.is_leaf = 0;
        new_splited_Node.parent = node.parent;

        //data和children
        for(int i = 0; i < right_size; i++) {
            new_splited_Node.data[i] = node.data[mid + 1 + i];
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
            Node<Key, Value> node_right;
            mr.read(node_right, new_splited_Node.right);
            node_right.left = new_splited_Node_index;
            mr.update(node_right, new_splited_Node.right);
        }

        //修改被分裂节点的子节点的父亲指针
        for(int i = 0; i < right_size + 1; i++) {
            int child_index = new_splited_Node.children[i];
            if(child_index == -1) continue;
            Node<Key, Value> child_node;
            mr.read(child_node, child_index);
            child_node.parent = new_splited_Node_index;
            mr.update(child_node, child_index);
        }

    }
        
    if(node.parent == -1){//要创造新根

        //建立新根
        Data<Key, Value> up_key = up_data;
        Node<Key, Value> new_root;//建立新根
        new_root.data[0] = up_key;
        new_root.size = 1;
        new_root.is_leaf = 0;
        new_root.parent = -1;
        new_root.children[0] = node_index;
        new_root.children[1] = new_splited_Node_index;

        root = mr.write(new_root);//写入新根并更新root
        mr.write_info(root,1);
        node.parent = root;
        new_splited_Node.parent = root;
        mr.update(node, node_index);
        mr.update(new_splited_Node, new_splited_Node_index);
        return;
    }

    else {//不是孤点，直接把提升的key插入父亲节点
        Node<Key, Value> parent_node;
        mr.read(parent_node, node.parent);//读出父亲节点
        int id_in_parent = find_in_Node(parent_node, up_data);

        //移动数据挪位置
        for(int i = parent_node.size; i > id_in_parent; i--) {
            parent_node.data[i] = parent_node.data[i-1];
        }
        for(int i = parent_node.size + 1; i > id_in_parent + 1; i--) { //children单独右移，避免越界
            parent_node.children[i] = parent_node.children[i-1];
        }

        parent_node.data[id_in_parent] = up_data;
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

template<class Key, class Value>
void bpt<Key, Value>::insert(const Key& key, const Value& value) {

    Data<Key, Value> data = Data<Key, Value>(key, value);

    if(root <= 0) {
        Node<Key, Value> new_node;
        new_node.is_leaf = 1;//插入根,是根又是叶
        new_node.data[0] = data;//data 0-based
        new_node.size = 1;
        root = mr.write(new_node);
        mr.write_info(root,1);
        return;
    }

    int current_index = root;
    Node<Key, Value> current_Node;
    mr.read(current_Node,root);//读出根，从根开始匹配

    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    //跳出循环的时候已经在叶子
    int id = find_in_Node(current_Node, data);
    if(id < current_Node.size && current_Node.data[id] == data) return;
    
    //腾出位置
    for(int i = current_Node.size; i > id; i--) {
        current_Node.data[i] = current_Node.data[i-1];
    } 
    current_Node.data[id] = data;
    current_Node.size++;

    //处理块的大小
    if(current_Node.size <= N) {
        mr.update(current_Node, current_index);
        return;
    }
    else {//需要分裂块 
        mr.update(current_Node, current_index);
        split_Node(current_index);
    }

}

template<class Key, class Value>
void bpt<Key, Value>::find(const Key& key) {

    int flag = 0;//标记是否找到
    Data<Key, Value> data_l = Data<Key, Value>(key, Value());//初始化data为最小值，这样查找时才能找到所有的条目

    if(root <= 0) {
        std::cout << "null" << std::endl;
        return;
    }

    int current_index = root;
    Node<Key, Value> current_Node;
    mr.read(current_Node,root);//读出根，从根开始匹配，这个与insert类似
    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data_l);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    bool first = true;
    bool stop = false;
    while(true) {//这里要沿着叶节点不断循环匹配
        int id = find_in_Node(current_Node, data_l);
        for(int i = id; i < current_Node.size; i++) {//从第一个大于等于data的位置开始匹配
            if(current_Node.data[i].key == key) {
                if(!first) std::cout << " ";
                std::cout << current_Node.data[i].value;
                first = false;
                flag = 1;
            } else { //已经不是一个关键词了，停下
                stop = true; 
                break; 
            }
        }
        if(stop || current_Node.right == -1) break;//如果没有右兄弟了就结束
        mr.read(current_Node,current_Node.right);//更新到右兄弟继续匹配
    }
    if(!flag) {
        std::cout << "null";
    }
    std::cout << std::endl;
}

template<class Key, class Value>
int bpt<Key, Value>::find_(const Key& key, const Value& value) {//重载find函数，返回一个Node，如果没有找到就返回一个空Node
    Data<Key, Value> data = Data<Key, Value>(key, value);
    int current_index = root;
    Node<Key, Value> current_Node;
    if(root <= 0) {
        return -1;
    }

    mr.read(current_Node,root);//读出根，从根开始匹配，这个与insert类似
    while(!current_Node.is_leaf) {
        int id = find_child_in_Node(current_Node, data);
        current_index = current_Node.children[id];//更新进入子树
        mr.read(current_Node,current_index);//再读出新子树的位置
    }

    int id = find_in_Node(current_Node, data);
    if(id < current_Node.size && current_Node.data[id] == data) return current_index;
    
    return -1;
}

template<class Key, class Value>
void bpt<Key, Value>::merge_node(int node_index) {
    Node<Key, Value> node;
    mr.read(node, node_index);
    if(node.parent == -1) {
        //如果是根节点
        if(!node.is_leaf && node.size == 0) {
            //根节点为空，且有一个孩子，则孩子成为新根
            root = node.children[0];
            Node<Key, Value> root_node;
            mr.read(root_node, root);
            root_node.parent = -1;
            mr.update(root_node, root);
            mr.write_info(root, 1);
        } else if(node.size == 0) {
            root = -1;
            mr.write_info(root, 1);
        }
        return;
    }

    Node<Key, Value> parent_node;
    mr.read(parent_node, node.parent);
    int child_pos = -1;
    for(int i = 0; i <= parent_node.size; i++) {
        if(parent_node.children[i] == node_index) {
            child_pos = i;
            break;
        }
    }

    Node<Key, Value> left_node, right_node;
    bool has_left = (child_pos > 0);
    bool has_right = (child_pos < parent_node.size);

    if(has_left) mr.read(left_node, parent_node.children[child_pos - 1]);
    if(has_right) mr.read(right_node, parent_node.children[child_pos + 1]);

    //尝试从左兄弟借
    if(has_left && left_node.size > N/2) {
        if(node.is_leaf) {
            for(int i = node.size; i > 0; i--) node.data[i] = node.data[i-1];
            node.data[0] = left_node.data[left_node.size - 1];
            node.size++;
            left_node.size--;
            parent_node.data[child_pos - 1] = node.data[0];
        } else {//内部节点要借一个分界值下来,然后
            for(int i = node.size; i > 0; i--) node.data[i] = node.data[i-1];
            for(int i = node.size + 1; i > 0; i--) node.children[i] = node.children[i-1];
            node.data[0] = parent_node.data[child_pos - 1];
            node.children[0] = left_node.children[left_node.size];
            parent_node.data[child_pos - 1] = left_node.data[left_node.size - 1];
            node.size++;
            left_node.size--;

            //更新借来的孩子的父亲指针
            Node<Key, Value> child_node;
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
            node.data[node.size] = right_node.data[0];
            node.size++;
            for(int i = 0; i < right_node.size - 1; i++) right_node.data[i] = right_node.data[i+1];
            right_node.size--;
            parent_node.data[child_pos] = right_node.data[0];
        } else {
            node.data[node.size] = parent_node.data[child_pos];
            node.children[node.size + 1] = right_node.children[0];
            parent_node.data[child_pos] = right_node.data[0];
            node.size++;
            for(int i = 0; i < right_node.size - 1; i++) right_node.data[i] = right_node.data[i+1];
            for(int i = 0; i < right_node.size; i++) right_node.children[i] = right_node.children[i+1];
            right_node.size--;

            //更新借来的孩子的父亲指针
            Node<Key, Value> child_node;
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
                left_node.data[left_node.size + i] = node.data[i];
            }
            left_node.size += node.size;
            left_node.right = node.right;
            if(node.right != -1) {
                Node<Key, Value> next_node;
                mr.read(next_node, node.right);
                next_node.left = left_index;
                mr.update(next_node, node.right);
            }
        } else {
            left_node.data[left_node.size] = parent_node.data[child_pos - 1];
            for(int i = 0; i < node.size; i++) {
                left_node.data[left_node.size + 1 + i] = node.data[i];
            }
            for(int i = 0; i <= node.size; i++) {
                left_node.children[left_node.size + 1 + i] = node.children[i];
                if(node.children[i] != -1) {
                    Node<Key, Value> child_node;
                    mr.read(child_node, node.children[i]);
                    child_node.parent = left_index;
                    mr.update(child_node, node.children[i]);
                }
            }
            left_node.size += node.size + 1;
        }
        mr.update(left_node, left_index);

        for(int i = child_pos - 1; i < parent_node.size - 1; i++) {
            parent_node.data[i] = parent_node.data[i + 1];
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
                node.data[node.size + i] = right_node.data[i];
            }
            node.size += right_node.size;
            node.right = right_node.right;
            if(right_node.right != -1) {
                Node<Key, Value> next_node;
                mr.read(next_node, right_node.right);
                next_node.left = node_index;
                mr.update(next_node, right_node.right);
            }
        } else {
            node.data[node.size] = parent_node.data[child_pos];
            for(int i = 0; i < right_node.size; i++) {
                node.data[node.size + 1 + i] = right_node.data[i];
            }
            for(int i = 0; i <= right_node.size; i++) {
                node.children[node.size + 1 + i] = right_node.children[i];
                if(right_node.children[i] != -1) {
                    Node<Key, Value> child_node;
                    mr.read(child_node, right_node.children[i]);
                    child_node.parent = node_index;
                    mr.update(child_node, right_node.children[i]);
                }
            }
            node.size += right_node.size + 1;
        }
        mr.update(node, node_index);

        for(int i = child_pos; i < parent_node.size - 1; i++) {
            parent_node.data[i] = parent_node.data[i + 1];
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

template<class Key, class Value>
void bpt<Key, Value>::remove(const Key& key, const Value& value){
    Data<Key, Value> data = Data<Key, Value>(key, value);
    int node_index = find_(key, value);
    if(node_index == -1) return;

    Node<Key, Value> node;
    mr.read(node, node_index);
    int id = find_in_Node(node, data);

    for(int i = id; i < node.size - 1; i++) {
        node.data[i] = node.data[i+1];
    }
    node.size--;
    mr.update(node, node_index);

    //若删除的是第一个元素且有父节点，需要向上更新父节点作为分界值的 data
    if(id == 0 && node.parent != -1 && node.size > 0) {
        int curr = node_index;
        while(curr != -1) {
            Node<Key, Value> curr_node;
            mr.read(curr_node, curr);
            if(curr_node.parent == -1) break;
            Node<Key, Value> parent_node;
            mr.read(parent_node, curr_node.parent);
            int child_pos = -1;
            for(int i = 0; i <= parent_node.size; i++) {
                if(parent_node.children[i] == curr) {
                    child_pos = i;
                    break;
                }
            }
            if(child_pos > 0) {
                parent_node.data[child_pos - 1] = node.data[0];
                mr.update(parent_node, curr_node.parent);
                break;
            }
            curr = curr_node.parent;
        }
    }

    if(node.parent == -1 && node.size == 0) {
        root = -1;
        mr.write_info(root, 1);
        return;
    }

    if(node.size < N/2 && node.parent != -1) {
        merge_node(node_index);
    }
}
