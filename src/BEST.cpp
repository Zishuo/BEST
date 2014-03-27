#include <iostream>
#include <fstream>
#include <exception>
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/progress.hpp>
#include <boost/program_options.hpp>
#include "controller.h"
using namespace std;
using namespace boost;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc_command("Allowed options");
    po::variables_map vm_command;
    string config_file_path;
    desc_command.add_options()
    ("c", po::value<string>(), "set config file path");

    try {

        po::store(po::parse_command_line(argc,argv,desc_command),vm_command);
        po::notify(vm_command);
        config_file_path = vm_command["c"].as<string>();
    } catch(...) {
        desc_command.print(cout);
        return 0;
    }

    cout << "command line : " <<endl;
    for (auto i = vm_command.begin(); i != vm_command.end(); ++i) {
        cout << i->first << " " << i->second.as<string>() <<endl;
    }


    po::options_description desc_config("");
    desc_config.add_options()
    ("in_net_graph_file_path", po::value<string>(), "网络拓补")
    ("in_datacenter_file_path", po::value<string>(), "数据中心")
    ("in_block_file_path", po::value<string>(), "分块信息")

    ("out_service_flow_file_path", po::value<string>(), "")
    ("out_sd_flow_file_path", po::value<string>(), "")
    ("out_place_file_path", po::value<string>(), "")
    ("out_green_file_path", po::value<string>(), "")
    ("out_report_file_path", po::value<string>(), "")

    ("node_number", po::value<int>(), "")
    ("link_number", po::value<int>(), "")
    ("core_number", po::value<int>(), "")
    ("aggregate_number", po::value<int>(), "")
    ("access_number", po::value<int>(), "")
    ("datacenter_number", po::value<int>(), "")
    ("block_number", po::value<int>(), "")
    ("service_number", po::value<int>(), "")

    ("hot_flow_begin", po::value<int>(), "")
    ("hot_flow_end", po::value<int>(), "")
    ("hot_flow_deviation", po::value<int>(), "")
    ("hot_flow_deviation_scale", po::value<int>(), "")
    ("hot_flow_step", po::value<int>(), "")
    ("hot_flow_scale", po::value<int>(), "")

    ("hot_rate_begin", po::value<int>(), "")
    ("hot_rate_end", po::value<int>(), "")
    ("hot_rate_step", po::value<int>(), "")
    ("hot_rate_scale", po::value<int>(), "")

    ("normal_flow_begin", po::value<int>(), "")
    ("normal_flow_end", po::value<int>(), "")
    ("normal_flow_deviation", po::value<int>(), "")
    ("normal_flow_deviation_scale", po::value<int>(), "")
    ("normal_flow_step", po::value<int>(), "")
    ("normal_flow_scale", po::value<int>(), "")
    ("background_flow", po::value<double>(), "")
    ("thread_limit", po::value<int>(), "");


    po::variables_map vm_config;
    try {
        po::store(po::parse_config_file<char>(config_file_path.c_str(),desc_config),vm_config);
        po::notify(vm_config);
    } catch(std::exception& e) {
        cerr << e.what() <<endl;
        desc_config.print(cout);
        return 0;
    }
    cout <<"====================================================="<<endl;
    cout <<"load config para : " <<endl;
    for (auto i = vm_config.begin(); i != vm_config.end(); ++i) {
        try {
            cout << i->first << "		=		" << i->second.as<string>() <<endl;
        } catch(...) {
            try {
                cout << i->first << "		=		" << i->second.as<int>() <<endl;
            } catch(...) {
                cout << i->first << "		=		" << i->second.as<double>() <<endl;
            }

        }
    }

    controller c;
    if (!c.Initialize(vm_config)) {
        cout << "Initialize fail!" <<endl;
        return 5;
    }
    c.Run();


    //[配置文件] [网络拓补] [网络层次] [数据中心] [分区信息] [服务流量结果]
    //[源目标端流量结果] [服务放置结果] [链路优化结果] [脚本执行总报告]



    return 0;
}

