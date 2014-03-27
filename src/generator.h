#pragma once
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include "Pair.h"

bool get_access(std::vector<int> & access_v,int core_s,int core_num,int agg_num) {
    for (int i = 0; i < 2; ++i) {
        access_v.push_back(core_num + agg_num + core_s * 4 + i);
    }
    return true;
}

double gen(
    int hot_flow,
    int hot_flow_scale,
    int hot_flow_dev,
    int hot_flow_dev_scale,
    int normal_flow,
    int normal_flow_scale,
    int normal_flow_dev,
    int normal_flow_dev_scale,
    int hot_rate,
    int hot_rate_scale,
    std::ostream &log) {
    double flow = 0;
    int rand_rate = rand()%hot_rate_scale;

    if (rand_rate < hot_rate) {
        //flow_1
        double dev = rand()%(2*hot_flow_dev+1) - hot_flow_dev;
//		log << "hot dev : " << dev <<std::endl;
        flow = hot_flow * (1 + dev/hot_flow_dev_scale);
        flow /= hot_flow_scale;
    } else {
        //flow_2
        double  dev = rand()%(2*normal_flow_dev+1) - normal_flow_dev;
//		log << "normal dev : " << dev <<std::endl;
        flow = normal_flow * (1 + dev/normal_flow_dev_scale);
        flow /= normal_flow_scale;
    }
    return flow;
}


void generate(std::map<int,std::multimap<int,double>> & service_flow_, //out
              int service_number,
              bool is_balanced,
              std::vector<std::vector<int>> blocks,
              int core_num,
              int agg_num,
              int hot_flow,
              int hot_flow_scale,
              int hot_flow_dev,
              int hot_flow_dev_scale,
              int normal_flow,
              int normal_flow_scale,
              int normal_flow_dev,
              int normal_flow_dev_scale,
              int hot_rate,
              int hot_rate_scale,
              std::string out_prefix,
              std::string out_service_flow_path) {
    std::ofstream service_flow_stream(out_service_flow_path+out_prefix+"_"+"ServiceFlow.txt");
    std::ofstream log(out_service_flow_path+out_prefix+"_"+"gen_log.txt");
    int block = service_number/blocks.size();
    for (int i = 0; i < service_number; ++i) {
        int hot_zone = i/block;
        log << "i " <<i << "blocks size : " << blocks.size() <<std::endl;
        log << "hot zone " << hot_zone <<std::endl;
        for (size_t j = 0; j < blocks.size(); ++j) {
            for (size_t k = 0; k < blocks[j].size(); ++k) {
                int node = blocks[j][k];
                std::vector<int> access_v;
                get_access(access_v,node,core_num,agg_num);

                for (size_t x = 0; x < access_v.size(); ++x) {
                    for(int y = 0; y < 4; ++y) {

                        double flow = 0;
                        if (j == hot_zone) {
                            flow = gen(hot_flow,hot_flow_scale,hot_flow_dev,hot_flow_dev_scale,
                                       normal_flow,normal_flow_scale,normal_flow_dev,normal_flow_dev_scale,
                                       hot_rate,hot_rate_scale,log);
                        } else {
                            flow = gen(hot_flow,hot_flow_scale,hot_flow_dev,hot_flow_dev_scale,
                                       normal_flow,normal_flow_scale,normal_flow_dev,normal_flow_dev_scale,
                                       0,hot_rate_scale,log);
                        }
                        service_flow_[i].insert(std::make_pair(access_v[x],flow));
                        service_flow_stream << i << " " << access_v[x] << " " << flow <<std::endl;
                    }
                }

            }
        }
    }

}

