#pragma once

class Link {
  public:
    int start_vertex;

    int end_vertex;

    double max_flow;

    double power;

    double weight;

    double flow;

    int occurrence;

    double flow_chance;

    bool state;

  public:
    Link() {
        ini();
    }
    Link(int start_vertex, int end_vertex) {
        ini();
        this->start_vertex = start_vertex;
        this->end_vertex = end_vertex;
    }

    Link(int start_vertex, int end_vertex, double weight, double power, double max_flow) {
        ini();
        this->start_vertex = start_vertex;
        this->end_vertex = end_vertex;
        this->weight = weight;
        this->power = power;
        this->max_flow = max_flow;
        this->state = true;
    };

    Link(const Link& link) {
        this->start_vertex = link.Start_vertex();
        this->end_vertex = link.End_vertex();
        this->power = link.Power();
        this->state = link.Is_open();
        weight = link.GetWeight();
        this->flow = link.Flow();
        this->max_flow = link.Max_flow();
        this->occurrence = link.Occurrence();
        this->flow_chance = link.Flow_chance();
    }

    bool operator < (const Link & inter) const{
        if (occurrence < inter.occurrence) {
            return true;
        } else if(occurrence == inter.occurrence) {
            if (flow_chance < inter.flow_chance) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }



    bool flag;


    int End_vertex() const {
        return end_vertex;
    }
    void End_vertex(int val) {
        end_vertex = val;
    }
    int Start_vertex() const {
        return start_vertex;
    }
    void Start_vertex(int val) {
        start_vertex = val;
    }
    double Power() const {
        return power;
    }
    void Power(double val) {
        power = val;
    }
    bool Is_open() const {
        return state;
    }
    void Is_open(bool val) {
        state = val;
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
    double Flow() const {
        return flow;
    }
    void Add_flow(double _flow) {
        flow += _flow;
    }
    void Flow(double val) {
        flow = val;
    }
    double GetWeight()const {
        return weight;
    }
    void SetWeight(double val) {
        weight = val;
    }
    double Max_flow() const {
        return max_flow;
    }
    void Max_flow(double val) {
        max_flow = val;
    }
    double Flow_chance() const {
        return flow_chance;
    }
    void Add_flow_chance(double f) {
        flow_chance += f;
    }
    void Flow_chance(double val) {
        flow_chance = val;
    }
    ~Link(void) {};

  private:
    void ini() {
        flow = 0;
        max_flow = 0;
        occurrence = 0;
        flow_chance = 0;
        state = true;
        flag = false;
        weight = 0;
        power = 0;
        start_vertex = -1;
        end_vertex = -1;
    }
};


