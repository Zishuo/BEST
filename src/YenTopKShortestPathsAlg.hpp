#pragma once
#include <map>
#include <queue>
#include <set>
#include <vector>
#include "DijkstraShortestPathAlg.hpp"
#include "GraphElements.hpp"
#include "Graph.hpp"

using namespace std;

class YenTopKShortestPathsAlg {
    shared_ptr<Graph> m_pGraph;

    vector<shared_ptr<BasePath>> m_vResultList;
    map<shared_ptr<BasePath>, shared_ptr<BaseVertex>> m_mpDerivationVertexIndex;
    multiset<shared_ptr<BasePath>, WeightLess<shared_ptr<BasePath> > > m_quPathCandidates;



    int m_nGeneratedPathNum;

  private:

    void _init();

  public:

    shared_ptr<BaseVertex> m_pSourceVertex;
    shared_ptr<BaseVertex> m_pTargetVertex;

    YenTopKShortestPathsAlg(const Graph& graph):m_pSourceVertex(NULL), m_pTargetVertex(NULL) {
        m_pGraph = shared_ptr<Graph>(new Graph(graph));
        _init();
    }

    YenTopKShortestPathsAlg(const Graph& graph, shared_ptr<BaseVertex> pSource, shared_ptr<BaseVertex> pTarget)
        :m_pSourceVertex(pSource), m_pTargetVertex(pTarget) {
        m_pGraph = shared_ptr<Graph>(new Graph(graph));
        _init();
    }

    ~YenTopKShortestPathsAlg(void) {
        clear();
    }
    void SetNextCompute(shared_ptr<BaseVertex> pSource, shared_ptr<BaseVertex> pTarget);
    void clear();
    bool has_next();
    shared_ptr<BasePath> next();

    shared_ptr<BasePath> get_shortest_path(shared_ptr<BaseVertex> pSource, shared_ptr<BaseVertex> pTarget);
    void get_shortest_paths(shared_ptr<BaseVertex> pSource, shared_ptr<BaseVertex> pTarget, int top_k, vector<shared_ptr<BasePath>>&);
};



