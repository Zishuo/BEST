#pragma once
#include <vector>
#include <string>
///define the node type 0,1,2,3,else
enum node_type {ac = 0, dc = 1, mt = 2, cr = 3, uk=  4};
class Node {
  public:
    ///�ڵ�ID
    int id;

    ///�ڵ�����
    node_type nt;

    ///�ڵ����
    double power;

    ///�ڵ��Ƿ��
    bool state;

    ///�����ڵ�����
    double flow;

    ///¶�����
    int occurrence;

    ///��������
    double flow_chance;
    Node() {
        this->ini();
    };

    Node(int _id) {
        this->ini();
        this->id = _id;
    };

    Node(int _id, node_type _nt, double _power, bool _isopen) {
        this->ini();
        this->id = _id;
        this->nt = _nt;
        this->power = _power;
        this->state = _isopen;
    };

    Node(const Node& node) {
        *this = node;
    };

    int get_id()const {
        return this->id;
    }
    void set_id(int id) {
        this->id = id;
    }

    node_type get_node_type()const {
        return this->nt;
    }
    void set_node_type(node_type nt) {
        this->nt = nt;
    }
    double get_power()const {
        return this->power;
    }
    void set_power(double power) {
        this->power = power;
    }

    bool is_open()const {
        return this->state;
    }
    void set_open(bool isopen) {
        this->state = isopen;
    }

    double Flow() const {
        return flow;
    }
    void Add_flow(double _flow) {
        flow+= _flow;
    }
    void Flow(double val) {
        flow = val;
    }

    int Occurrence() const {
        return occurrence;
    }
    void Add_occurrence() {
        ++occurrence;
    }
    void Occurrence(int val) {
        occurrence = val;
    }

    double Flow_chance() const {
        return flow_chance;
    }
    void Add_flow_chance(double _flow) {
        flow_chance += _flow;
    }
    void Flow_chance(double val) {
        flow_chance = val;
    }

    std::string toString() {
        std::string str;
        return str;
    }
    ~Node(void) {};

    Node& operator=	(const Node& node) {
        this->id = node.get_id();
        this->power = node.get_power();
        this->state = node.is_open();
        this->nt = node.get_node_type();
        this->occurrence = node.Occurrence();
        this->flow = node.Flow();
        this->flow_chance = node.Flow_chance();
        return *this;
    }
  private:
    void ini() {
        id = -1;
        nt = uk;
        state = true;
        flow = 0;
        occurrence = 0;
        flow_chance = 0;
        power = 0;
    }

};


