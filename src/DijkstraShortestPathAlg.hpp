#pragma once
#include <set>
#include <map>
#include <vector>
#include "GraphElements.hpp"
#include "Graph.hpp"

using namespace std;
using std::shared_ptr;

class DijkstraShortestPathAlg {
  private: // members

    shared_ptr<Graph> m_pDirectGraph;

    std::map<shared_ptr<BaseVertex>, double> m_mpStartDistanceIndex;
    std::map<shared_ptr<BaseVertex>, shared_ptr<BaseVertex>> m_mpPredecessorVertex;

    std::set<int> m_stDeterminedVertices;

    std::multiset<shared_ptr<BaseVertex>, WeightLess<shared_ptr<BaseVertex> > > m_quCandidateVertices;

  public:
    DijkstraShortestPathAlg(shared_ptr<Graph> pGraph):m_pDirectGraph(pGraph) {}
    ~DijkstraShortestPathAlg(void) {
        clear();
    }

    void clear();

    shared_ptr<BasePath> get_shortest_path(shared_ptr<BaseVertex> source, shared_ptr<BaseVertex> sink);

    void set_predecessor_vertex(shared_ptr<BaseVertex> vt1, shared_ptr<BaseVertex> vt2) {
        m_mpPredecessorVertex[vt1] = vt2;
    }

    double get_start_distance_at(shared_ptr<BaseVertex> vertex) {
        return m_mpStartDistanceIndex.find(vertex)->second;
    }

    void set_start_distance_at(shared_ptr<BaseVertex> vertex, double weight) {
        m_mpStartDistanceIndex[vertex] = weight;
    }

    void get_shortest_path_flower(shared_ptr<BaseVertex> root) {
        determine_shortest_paths(NULL, root, false);
    }

    // The following two methods are prepared for the top-k shortest paths algorithm
    shared_ptr<BasePath> update_cost_forward(shared_ptr<BaseVertex> vertex);
    void correct_cost_backward(shared_ptr<BaseVertex> vertex);

  protected:

    void determine_shortest_paths(shared_ptr<BaseVertex> source, shared_ptr<BaseVertex> sink, bool is_source2sink);

    void improve2vertex(shared_ptr<BaseVertex> cur_vertex_pt, bool is_source2sink);

};

