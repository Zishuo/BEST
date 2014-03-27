#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "Node.h"
#include "Link.h"
#include "Graph.hpp"
#include "YenTopKShortestPathsAlg.hpp"
#include "Pair.h"
#define _AFXDLL

using namespace std;
class Green {

  public:
    typedef map<int,Node> NODES;
    typedef map<Pair,Link> LINKS;
    typedef multimap<Pair,double> S_D_FLOWS;
    typedef map<Pair,vector<vector<int> > > PATHS;

    Green(
        const map<int,Node> & nodes,
        const map<Pair,Link> & links,
        const multimap<Pair,double> & s_d_flow_,
        int core_num,
        int aggt_num,
        int accs_num,
        size_t thread_num,
        const string & out_prefix, const string & out_green);

    bool Run();
    double sum_nodes();
    double sum_links();
    //members
    int cpu_num;
    int opened_node;
    int opened_link;
    ofstream log;
    int core_num_,aggt_num_,accs_num_;
    S_D_FLOWS s_d_flows_;
    NODES nodes_;
    LINKS links_;
    PATHS paths_;

    map<int,set<int>> attach;
    boost::interprocess::interprocess_semaphore mutex_c;
    size_t thread_num_;

  private:


    void close_node( int node_id, vector<Pair> & changed_links );
    void open_node( int node_id, const vector<Pair> & changed_links );
    void close_link( Pair L );
    void open_link( Pair L );


    bool check_subject(PATHS &paths);
    bool check_connected();

    size_t check_path(double flow,const vector<vector<int>> &paths_in, vector<vector<int>> & path_passed);
    size_t select_path(const vector<vector<int>> & path_in , vector<int> & path_selected);

    void push_flow(double f,vector<int>& a_path);
    template <typename T>
    static bool less_occ_less_flow_chonce(const T& L1, const T& L2);

    PATHS compute_all_shortest_path();
    size_t compute_all_shortest_path_multi_thread(PATHS & paths, size_t thread_num = 4);
    void compute_node_occurrence( PATHS & paths );
    void compute_link_occurrence( PATHS & paths );
    void recompute_short_path(int s, int e, PATHS & paths);
    void thread_compute_short_path(int s, int e, const std::vector<Pair> & thread_blocks, PATHS & paths);
    void recompute_shortest_path_with_changed_node(int node_id, PATHS & paths, size_t thread_num = 4);
    void recompute_shortest_path_with_changed_link(const Link & L, PATHS & paths, size_t thread_num = 4);


    //
    double compute_power();
    void print_path( const vector<vector<int>> & a_s_d_path ,ostream & out);
    void print_paths(PATHS & paths , ofstream & out);
    void store_path(ofstream &path_cache, PATHS &paths  );
    void restore_path(ifstream &path_cache, PATHS &paths  );
    void print_load(ostream &os);
    void print_status(ostream &os);
};





