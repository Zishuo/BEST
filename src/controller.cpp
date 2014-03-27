#include <iostream>
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/timer.hpp>
#include "controller.h"
#include "generator.h"
#include "place.h"
#include "NetStruture.h"
using namespace std;

controller::controller() {
}

bool controller::Initialize( const boost::program_options::variables_map & mp ) {
    try {
        in_netgraph_path = mp["in_net_graph_file_path"].as<string>();
        in_datacenter_path = mp["in_datacenter_file_path"].as<string>();
        in_block_path = mp["in_block_file_path"].as<string>();

        out_service_flow = mp["out_service_flow_file_path"].as<string>();
        out_sd_flow = mp["out_sd_flow_file_path"].as<string>();
        out_place = mp["out_place_file_path"].as<string>();
        out_green = mp["out_green_file_path"].as<string>();
        out_report = mp["out_report_file_path"].as<string>();
        thread_num = mp["thread_limit"].as<int>();

        node_num = mp["node_number"].as<int>();
        link_num = mp["link_number"].as<int>();
        core_num = mp["core_number"].as<int>();
        aggt_num = mp["aggregate_number"].as<int>();
        accs_num = mp["access_number"].as<int>();
        dtct_num = mp["datacenter_number"].as<int>();
        blck_num = mp["block_number"].as<int>();
        srvc_num = mp["service_number"].as<int>();
        background_flow = mp["background_flow"].as<double>();

        hot_flow_beg = mp["hot_flow_begin"].as<int>();
        hot_flow_end = mp["hot_flow_end"].as<int>();
        hot_flow_dev = mp["hot_flow_deviation"].as<int>();
        hot_flow_dev_scale = mp["hot_flow_deviation_scale"].as<int>();
        hot_flow_step = mp["hot_flow_step"].as<int>();
        hot_flow_scale = mp["hot_flow_scale"].as<int>();

        hot_rate_beg = mp["hot_rate_begin"].as<int>();
        hot_rate_end = mp["hot_rate_end"].as<int>();
        hot_rate_step = mp["hot_rate_step"].as<int>();
        hot_rate_scale = mp["hot_rate_scale"].as<int>();

        normal_flow_beg = mp["normal_flow_begin"].as<int>();
        normal_flow_end = mp["normal_flow_end"].as<int>();
        normal_flow_dev = mp["normal_flow_deviation"].as<int>();
        normal_flow_dev_scale = mp["hot_flow_deviation_scale"].as<int>();
        normal_flow_step = mp["normal_flow_step"].as<int>();
        normal_flow_scale = mp["normal_flow_scale"].as<int>();
    } catch(exception & e) {
        cerr << e.what() <<endl;
        return false;
    }

    core_core_link_num = 0;
    core_low_level_link_num = 0;

    if (!load_net_graph()) {
        return false;
    }

    if (!load_block()) {
        return false;
    }

    if (!load_datacenter()) {
        return false;
    }

    return true;
}



void controller::Run() {
    ofstream report("report.txt",std::ios_base::app);
    int count = 0;
    int ava_node = node_num - aggt_num/2 - core_num/2 - accs_num;
    int ava_link = link_num - aggt_num/2 - core_num/2 - accs_num  + 1;
    report
            << " available nodes : " << ava_node << endl
            << " available links : " << ava_link <<endl<<endl;

    for (int hot_flow = hot_flow_beg; hot_flow <= hot_flow_end;
            hot_flow = hot_flow + hot_flow_step) {

        for (int hot_rate = hot_rate_beg; hot_rate <= hot_rate_end;
                hot_rate = hot_rate + hot_rate_step) {

            for (int normal_flow = normal_flow_beg; normal_flow <= normal_flow_end;
                    normal_flow = normal_flow + normal_flow_step ) {
                boost::timer onec_timer;
                string out_prefix = gen_time_prefix();
                do_clear();

                cout << "================================================="<<endl;
                cout << "computing " << ++count << " times " <<endl;
                report << "================================================"<<endl;
                report
                        << " log prefix : " << out_prefix << endl
                        << " hot rate : " << hot_rate/(double)hot_rate_scale
                        << " hot flow : " << hot_flow/(double)hot_flow_scale
                        << " normal flow : " << normal_flow/(double)normal_flow_scale<<endl;



                boost::timer fun_timer;
                double min_time = 0,max_time = 0;
                cout << "generate flow...";
                generate(
                    service_flow_,
                    srvc_num,
                    is_balanced,
                    blocks,
                    core_num,aggt_num,
                    hot_flow,hot_flow_scale,hot_flow_dev,hot_flow_dev_scale,
                    normal_flow,normal_flow_scale,normal_flow_dev,normal_flow_dev_scale,
                    hot_rate,hot_rate_scale,
                    out_prefix,out_service_flow);
                cout << "done. "<<fun_timer.elapsed()<<endl;



                //min
                cout << "place min...";
                fun_timer.restart();
                double min_ob_value = 0;
                min_ob_value = place(service_flow_,blocks,s_d_flow_,links,datacenter_capacity_,srvc_num,
                                     node_num,core_num,aggt_num,accs_num,background_flow,"min",
                                     out_prefix,out_sd_flow,out_place);
                min_time += fun_timer.elapsed();
                cout << "done. "<<fun_timer.elapsed() << endl;

                cout << "green min...";
                fun_timer.restart();
                Green green_min(nodes,links,s_d_flow_,
                                core_num,aggt_num,accs_num,
                                thread_num,
                                out_prefix+"_min",out_green);
                green_min.Run();
                min_time += fun_timer.elapsed();
                cout << "done. " << fun_timer.elapsed() << endl;

                report
                        << "	min object value : " <<min_ob_value <<endl
                        << "	node close : " << node_num - green_min.opened_node
                        <<" rate : " << (node_num - green_min.opened_node)/(double)(ava_node)<<endl
                        << "	link close : " << link_num - green_min.opened_link
                        <<" rate : " << (link_num - green_min.opened_link)/(double)(ava_link)<<endl
                        << "------------------------------------------------"<< min_time <<endl;

                //max
                cout << "place max...";
                fun_timer.restart();
                double max_ob_value = 0;
                max_ob_value = place(service_flow_,blocks,s_d_flow_,links,datacenter_capacity_,srvc_num,
                                     node_num,core_num,aggt_num,accs_num,background_flow,"max",
                                     out_prefix,out_sd_flow,out_place);
                max_time += fun_timer.elapsed();
                cout << "done. " <<fun_timer.elapsed()<<endl;

                cout << "green max...";
                fun_timer.restart();
                Green green_max(nodes,links,s_d_flow_,
                                core_num,aggt_num,accs_num,
                                thread_num,
                                out_prefix+"_max",out_green);
                green_max.Run();
                max_time += fun_timer.elapsed();
                cout << "done. " << fun_timer.elapsed() << endl;

                report
                        << "	max object value : " <<max_ob_value <<endl
                        << "	node close : " << node_num - green_max.opened_node <<" rate : " << (node_num - green_max.opened_node)/(double)(ava_node)<<endl
                        << "	link close : " << link_num - green_max.opened_link <<" rate : " << (link_num - green_max.opened_link)/(double)(ava_link)<<endl
                        <<"------------------------------------------------"<<max_time<<endl;
                report <<"min-max time : " << onec_timer.elapsed()<<endl<<endl;
            }
        }
    }
}

controller::~controller( void ) {
    cout << "successfully done." <<endl;
}

void controller::do_clear() {
    service_flow_.clear();
    s_d_flow_.clear();
}

std::string controller::gen_time_prefix() {
    return boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
}

bool controller::load_net_graph() {
    ifstream net_graph_stream(in_netgraph_path.c_str());
    if (!net_graph_stream.is_open()) {
        return false;
    }
    for (int i = 0; i < node_num; ++i) {
        int src_id = 0,dst_id = 0,atc_num = 0;

        net_graph_stream >> src_id;
        net_graph_stream >> atc_num;
        for (int j = 0; j < atc_num; ++j ) {
            double max_flow = 0, weight = 0;
            net_graph_stream >> dst_id;
            net_graph_stream >> max_flow;
            net_graph_stream >> weight;

            if (weight == 0) {
                weight = 1.0/max_flow;
            }
            Link new_link;
            new_link.max_flow = max_flow;
            new_link.start_vertex = src_id;
            new_link.end_vertex = dst_id;
            new_link.weight = weight;
            links[Pair(src_id,dst_id)] = new_link;

            if (nodes.find(src_id) == nodes.end()) {
                Node node;
                node.id = src_id;
                if (src_id < core_num) {
                    node.nt = cr;
                } else if(src_id < core_num + aggt_num && src_id > core_num) {
                    node.nt = mt;
                } else {
                    node.nt = ac;
                }
                nodes[src_id] = node;
            }

            if (nodes.find(dst_id) == nodes.end()) {
                Node node;
                node.id = dst_id;
                if (dst_id < core_num) {
                    node.nt = cr;
                } else if(dst_id < core_num + aggt_num && dst_id > core_num) {
                    node.nt = mt;
                } else {
                    node.nt = ac;
                }
                nodes[dst_id] = node;
            }
        }
    }

    for (auto i = datacenter_capacity_.begin(); i != datacenter_capacity_.end(); ++i) {
        nodes[i->first].nt = dc;
    }

    for (auto i = links.begin(); i != links.end(); ++i) {
        if (nodes[i->first.peer_1].nt == cr && nodes[i->first.peer_2].nt == cr) {
            core_core_link_num++;
        } else {
            core_low_level_link_num++;
        }
    }


    if (links.size() != link_num) {
        cerr << "load links error." <<endl;
        return false;
    } else {
        return true;
    }
}

bool controller::load_block() {
    ifstream block_stream(in_block_path);

    if (!block_stream.is_open()) {
        return false;
    }

    for (int i = 0; i < blck_num; ++i) {
        string line;
        int node;
        vector<int> block;
        getline(block_stream,line);
        stringstream ss(line);
        while(ss >>node) {
            block.push_back(node);
        }
        blocks.push_back(block);
    }
    return true;
}

bool controller::load_datacenter() {
    ifstream datacenter_stream(in_datacenter_path);
    if (!datacenter_stream.is_open()) {
        return false;
    }
    for (int i = 0; i < dtct_num; ++i) {
        int dt,capacity;
        datacenter_stream >>dt>>capacity;
        datacenter_capacity_[dt] = capacity;
    }
    return true;
}



