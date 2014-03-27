#pragma once
#include <string>
#include <boost/date_time.hpp>
#include <boost/program_options.hpp>
#include <map>
#include <vector>
#include "Pair.h"
#include "Link.h"
#include "Node.h"
class controller {
  public:
    controller();
    void Run();
    bool Initialize(const boost::program_options::variables_map & mp);
    ~controller(void);
  private:
    std::string in_netgraph_path,in_datacenter_path,in_block_path;
    std::string out_service_flow,out_sd_flow,out_place,out_green,out_report;

    int normal_flow_beg,normal_flow_end,normal_flow_step,normal_flow_dev,normal_flow_scale;
    int hot_flow_beg,hot_flow_end,hot_flow_step,hot_flow_dev,hot_flow_scale;
    int hot_rate_beg,hot_rate_end,hot_rate_step,hot_rate_scale;
    int dtct_num,core_num,aggt_num,accs_num;
    int blck_num;
    int node_num,link_num;
    int core_core_link_num;
    int core_low_level_link_num;
    bool is_balanced;

    std::map<int, std::multimap<int,double> > service_flow_;
    std::multimap<Pair,double> s_d_flow_;
    std::map<Pair,Link> links;
    std::map<int,Node> nodes;
    std::map<int,int> datacenter_capacity_;
    std::map<Pair,std::vector<std::vector<int>>> paths;
    std::vector<std::vector<int>> blocks;
    int srvc_num;
    int hot_flow_dev_scale;
    int normal_flow_dev_scale;
    size_t thread_num;
    double background_flow;
  private:
    void do_clear();
    std::string gen_time_prefix();
    bool load_net_graph();
    bool load_block();
    bool load_datacenter();
};


