#pragma once
#include <algorithm>
#include <limits>
#include <set>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "GraphElements.hpp"
#include "Pair.h"
#include "Link.h"
using namespace std;
class Green;
class Path : public BasePath {
  public:

    Path(const std::vector<shared_ptr<BaseVertex>>& vertex_list, double weight):BasePath(vertex_list,weight) {}

    // display the content
    void PrintOut(std::ostream& out_stream) const {
        out_stream << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
        for(std::vector<shared_ptr<BaseVertex>>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end(); ++pos) {
            out_stream << (*pos)->getID() << " ";
        }
        out_stream << std::endl <<  "=============================================" << std::endl;
    }
};

class Graph {
  public: // members

    const static double DISCONNECT;

    typedef set<shared_ptr<BaseVertex>>::iterator VertexPtSetIterator;
    typedef map<shared_ptr<BaseVertex>, shared_ptr<set<shared_ptr<BaseVertex> > >>::iterator BaseVertexPt2SetMapIterator;

  protected: // members

    // Basic information
    map<shared_ptr<BaseVertex>, shared_ptr<set<shared_ptr<BaseVertex> > >> m_mpFanoutVertices;
    map<shared_ptr<BaseVertex>, shared_ptr<set<shared_ptr<BaseVertex> > >> m_mpFaninVertices;
    map<int, double> m_mpEdgeCodeWeight;
    vector<shared_ptr<BaseVertex>> m_vtVertices;
    int m_nEdgeNum;
    int m_nVertexNum;

    map<int, shared_ptr<BaseVertex>> m_mpVertexIndex;

    // Members for graph modification
    set<int> m_stRemovedVertexIds;
    set<pair<int,int> > m_stRemovedEdge;

  public:

    // Constructors and Destructor
    Graph() {};
    Graph(const Green& NetS);
    Graph(map<Pair,Link> & links ,int opened_node, int opened_link);
    Graph(const string& file_name);
    Graph(const Graph& rGraph);
    ~Graph(void);

    void clear();

    shared_ptr<BaseVertex> get_vertex(int node_id);

    int get_edge_code(const shared_ptr<BaseVertex> start_vertex_pt, const shared_ptr<BaseVertex> end_vertex_pt) const;
    shared_ptr<set<shared_ptr<BaseVertex> > > get_vertex_set_pt(shared_ptr<BaseVertex> vertex_, map<shared_ptr<BaseVertex>, shared_ptr<set<shared_ptr<BaseVertex> > >>& vertex_container_index);

    double get_original_edge_weight(const shared_ptr<BaseVertex> source, const shared_ptr<BaseVertex> sink);
    double get_edge_weight(const shared_ptr<BaseVertex> source, const shared_ptr<BaseVertex> sink);
    void get_adjacent_vertices(shared_ptr<BaseVertex> vertex, set<shared_ptr<BaseVertex>>& vertex_set);
    void get_precedent_vertices(shared_ptr<BaseVertex> vertex, set<shared_ptr<BaseVertex>>& vertex_set);

    /// Methods for changing graph
    void remove_edge(const pair<int,int> edge) {
        m_stRemovedEdge.insert(edge);
    }

    void remove_vertex(const int vertex_id) {
        m_stRemovedVertexIds.insert(vertex_id);
    }

    void recover_removed_edges() {
        m_stRemovedEdge.clear();
    }

    void recover_removed_vertices() {
        m_stRemovedVertexIds.clear();
    }

    void recover_removed_edge(const pair<int,int> edge) {
        m_stRemovedEdge.erase(m_stRemovedEdge.find(edge));
    }

    void recover_removed_vertex(int vertex_id) {
        m_stRemovedVertexIds.erase(m_stRemovedVertexIds.find(vertex_id));
    }

  private:
    void _import_from_file(const std::string& file_name);

};



