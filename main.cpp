/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 循环引用问题, 导致指针不会被释放
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev;
    int value;

    // 这个构造函数有什么可以改进的？
    // explicit 防止错误的隐式类型转换
    explicit Node(int val) {
        value = val;
    }

    void insert(int val) {
        std::shared_ptr<Node> node = std::make_shared<Node>(val);
        node->next = next;
        if (next)
            next->prev = node;
        next = node;
    }

    void erase() {
        if (!prev.expired())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   
        // 应输出多少次？为什么少了？
        // 13次，未被正常释放
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;
    List(List const &other) {
        printf("List 被拷贝！\n");

        head = std::make_shared<Node>(other.head->value);
        std::shared_ptr<Node> cur = other.head;
        while (cur.get()->next) cur = cur.get()->next;
        while (cur != other.head) {
            head->insert(cur->value);
            head->next->prev = head;
            cur = cur->prev.lock();
        }
    }

    List &operator=(List const &) = delete;  
    // 为什么删除拷贝赋值函数也不出错？
    // 使用了拷贝构造函数

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        std::shared_ptr<Node> node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node *at(size_t index) const {
        Node* curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  
    // 有什么值得改进的？
    // 引用，减少值拷贝
    printf("[");
    for (Node* curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = List();
    a = List();

    return 0;
}
