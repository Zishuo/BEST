#include <algorithm>
#include <stack>
#include <set>
#include <limits>
#include <math.h>
#include <iostream>
#include <vector>
#include <thread>
#include <boost/timer.hpp>
#include <boost/lexical_cast.hpp>
#include "NetStruture.h"
//#define DEBUG
using namespace std;

Green::Green(const map<int,Node> & nodes,
             const map<Pair,Link> & links,
             const multimap<Pair,double> & s_d_flow,
             int core_num,
             int aggt_num,
             int accs_num,
             size_t thread_num,
             const string & out_prefix, const string & out_green)
    :core_num_(core_num),aggt_num_(aggt_num),accs_num_(accs_num),s_d_flows_(s_d_flow),links_(links),mutex_c(thread_num),thread_num_(thread_num) {
    for(auto i = links.begin(); i!=links.end(); ++i) {
        attach[i->first.peer_1].insert(i->first.peer_2);
        attach[i->first.peer_2].insert(i->first.peer_1);
    }
    opened_node = nodes.size();
    opened_link = links.size();
    log.open(out_green + out_prefix + "_" + "green_log.txt");
}

//main process
bool Green::Run() {

    boost::timer run_timer;
    boost::timer opt_node_timer;
    log << " @ " << "begin to close node..." << endl<<endl;

    ifstream path_cache_in("path.cache");
    ofstream path_cache_out;

    if (path_cache_in.is_open()) {
        restore_path(path_cache_in,paths_);
    } else {
        compute_all_shortest_path_multi_thread(paths_,thread_num_);
        path_cache_out.open("path.cache");
        store_path(path_cache_out, paths_);
    }

    compute_link_occurrence(paths_);
    compute_node_occurrence(paths_);

    if(!check_subject(paths_)) {
        print_load(log);
        cout <<"first check fail."<<endl;
        return false;
    };

    vector<Node> nodes_sorted;
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
        Node copy_of_node(i->second);
        nodes_sorted.push_back(copy_of_node);
    }

    //使用least occurence + least flow 对节点进行排序
    //sort the nodes according to the occurrence and the flow
    std::sort(nodes_sorted.begin(), nodes_sorted.end(),less_occ_less_flow_chonce<Node>);

    //for each node
    int n_count = 0;
    for (auto i = nodes_sorted.begin(); i != nodes_sorted.end(); ++i,n_count++) {

        int node_id = i->get_id();

        if (nodes_[node_id].get_node_type() == ac || nodes_[node_id].get_node_type() == dc) {
//			log << "access node pass." << endl;
            //若是一个接入节点则不能关闭
            continue;
        }

        log<<" # "<<node_id  <<" | "<<n_count<<endl;

        //关闭节点和与其相关的所有链路
        vector<Pair> changed_links;
        close_node(node_id,changed_links);

        //检测网络连通性
        if(!check_connected()) {
            open_node(node_id,changed_links);
            log << "connect fail."<<endl;
            continue;
        }

        //重新计算最短路径
        PATHS path_backup = paths_;
        recompute_shortest_path_with_changed_node(node_id, paths_, cpu_num);

        //检查并安排流量
        if(!check_subject(paths_)) {
            open_node(node_id,changed_links);
            paths_ = path_backup;
            log << "check fail."<<endl;
        } else {
            log << "close success."<<endl;
        }

    }

    log << " @ " << "complete to close node..."<<opt_node_timer.elapsed()<<endl;

    //TODO:record

    boost::timer opt_link_timer;
    log <<" @ "<< "begin close link..." << endl;
    compute_link_occurrence(paths_);
    compute_node_occurrence(paths_);
    vector<Link> links_sorted;

    //只拷贝出状态为开的链路
    for (auto i = links_.begin(); i != links_.end(); ++i) {
        if (i->second.state) {
            links_sorted.push_back(i->second);
        }

    }
    std::sort(links_sorted.begin(), links_sorted.end());

    n_count = 0;
    //尝试关闭每一条链路
    for (size_t i = 0; i < links_sorted.size(); ++i) {
        Pair P(links_sorted[i].start_vertex,links_sorted[i].end_vertex);
        Link &L = links_[P];
        Node &src = nodes_[L.Start_vertex()];
        Node &dst = nodes_[L.End_vertex()];

        n_count++;
        //if (src.get_node_type() == ac || dst.get_node_type() == ac){
        ////	log<<"access link, skip" << endl;
        //	continue;
        //}
        log<< " % "
           << " S " << L.Start_vertex()
           <<" <->"
           << " E " << L.End_vertex()
           << " | "
           << n_count
           <<endl;

        ///关闭连接
        close_link(P);

        if (!check_connected()) {
            open_link(P);
            log << "connect fail."<<endl;
            continue;
        }

        //重新计算最短路径
        PATHS path_backup = paths_;
        recompute_shortest_path_with_changed_link(L,paths_,thread_num_);


        //检查并安排流量
        if(!check_subject(paths_)) {
            open_link(P);
            paths_ = path_backup;
            log << "check fail."<<endl;
        } else {
            log << "close success." <<endl;
        }
    }


    print_load(log);

    log<<endl<<" @ "<< "complete opt link : " << opt_link_timer.elapsed()<<endl;

    log<<endl<<" & "<< "complete opt : " << run_timer.elapsed() <<endl;

    return true;
}

//检查路径能否承受流量
size_t Green::check_path( double flow,const vector<vector<int>> &paths_in, vector<vector<int>> & path_passed) {
//	print_path(paths_in,cout);
    for (std::size_t i = 0; i < paths_in.size(); i++) {
        //检查路径上的各个链路是否能满足要求
        bool flag = true;
        size_t pos = 0;
        for(std::size_t j = 0; j < paths_in[i].size() - 1; j++) {
            int src = paths_in[i][j];
            int dst = paths_in[i][j+1];
            Link &l = links_[Pair(src,dst)];
            if (l.flow + flow > l.max_flow) {
                pos = j;
                flag = false;
                break;
            }
        }

        if (flag == true) {
            path_passed.push_back(paths_in[i]);
        } else {

        }
    }


    //所有路径失败，输出信息
    if (path_passed.size() == 0) {
        log << paths_in.size()<<endl;
        for (size_t i = 0; i < paths_in.size(); ++i) {
            const vector<int> &path_t = paths_in[i];
            for (size_t j = 0; j < path_t.size() - 1; ++j) {
                int src = paths_in[i][j];
                int dst = paths_in[i][j+1];
                Link &l = links_[Pair(src,dst)];
                log <<paths_in[i][j] <<"->(" <<l.flow<<"/"<<l.max_flow<<")";
            }
            log <<paths_in[i].back()<<endl;
        }
    }
    return path_passed.size();
}

///根据LF+OC选择路径
size_t Green::select_path( const vector<vector<int>> & paths_in , vector<int> & path_selected) {
    double max = 0;
    size_t pos = 0;
    for (size_t i = 0; i < paths_in.size(); ++i) {
        double sum = 0;
        //把所有路径上的flow_chance累积得到路径flow_chance
        for (size_t j = 1; j < paths_in[i].size()-1; ++j) {
            sum += nodes_[paths_in[i][j]].flow_chance;
        }

        if (sum > max) {
            max = sum;
            pos = i;
        }
    }
    path_selected = paths_in[pos];
    return path_selected.size();
}

///压入流量
void Green::push_flow( double f,vector<int>& path_in ) {
    for (size_t i = 0; i < path_in.size() - 1; i++) {
        int src = path_in[i];
        int dst = path_in[i+1];
        Link &l = links_[Pair(src,dst)];
        l.flow += f;
    }
}

///检查网络是否能够承载流量
bool Green::check_subject(Green::PATHS &paths) {
    boost::timer t;
    log << "check_subject..." <<endl;
    //1.清空链路上的所有流量
    for (auto i = links_.begin(); i != links_.end(); ++i) {
        i->second.flow = 0;
    }
//	cout << "clean done." <<endl;
    //2.遍历所有流量
    for (auto i = s_d_flows_.begin(); i != s_d_flows_.end(); ++i) {

        //flow
        double flow = i->second;
        //src
        int src = i->first.peer_1;
        //dst
        int dst = i->first.peer_2;

        if (dst == src || flow == 0) {
            //无条件通过
            continue;
        }


//		cout << "S : "<<src<<" D : "<< dst<< " flow :" << flow << endl;

        vector<vector<int>> paths_passed;
        //检测此流量是否满足其中任意一条路径
        if (check_path(flow,paths[Pair(src,dst)],paths_passed) == 0) {
            log << "failed opt on flow " << src <<"->" <<dst << " : " <<flow <<endl;
            print_path(paths.at(Pair(src,dst)),log);
            return false;
        }

        //从满足条件的路径中根据机会流量选择一条路径
        vector<int> path_selected;
        select_path(paths_passed,path_selected);

        //把流量填入路径中
        push_flow(flow,path_selected);
    }
    log << "time : " << t.elapsed() << endl;
    return true;
}

///打开节点，及其相邻链路
void Green::open_node( int node_id, const vector<Pair> & changed_links ) {
    opened_node++;

    //打开当前节点
    nodes_[node_id].state = true;

    //打开其刚才关闭的链路
    for(size_t i = 0; i != changed_links.size(); ++i) {
        Link & L = links_.at(changed_links[i]);
        L.state = true;
        opened_link++;
    }
}

///关闭节点，及其相邻链路
void Green::close_node( int node_id, vector<Pair> & changed_links) {
    opened_node--;

    ///关闭当前节点
    nodes_[node_id].state = false;

    ///关闭与其相关的所有链路
    for (auto i = attach.at(node_id).begin(); i != attach.at(node_id).end(); ++i) {
        if (links_.at(Pair(node_id,*i)).state == true) {
            links_.at(Pair(node_id,*i)).state = false;
            changed_links.push_back(Pair(node_id,*i));
            opened_link--;

        }
    }

}

///关闭链路
void Green::close_link( Pair P ) {
    log << "close link : " << P.peer_1 << "<->" <<P.peer_2 <<endl;
    links_.at(P).state = false;
    opened_link--;
}

///打开链路
void Green::open_link( Pair P ) {
    log << "open link : " << P.peer_1 << "<->" <<P.peer_2 <<endl;
    links_.at(P).state = true;
    opened_link++;
}

void Green::thread_compute_short_path( int s, int e, const vector<Pair> & thread_blocks , PATHS & paths ) {

    Graph G(links_,opened_node,opened_link);
    YenTopKShortestPathsAlg yenAlg(G);

    for (int x = s; x < e; ++x) {
        int i = thread_blocks[x].peer_1;
        int j = thread_blocks[x].peer_2;
        if (
            i == j ||
            nodes_[i].get_node_type() == mt ||
            nodes_[i].get_node_type() == cr ||
            nodes_[j].get_node_type() == mt ||
            nodes_[j].get_node_type() == cr) {
            //不计算自身到自身的最短路径，以及那些非接入层最短路径
            continue;
        }

        else {
            yenAlg.SetNextCompute(G.get_vertex(i),G.get_vertex(j));
            int cost =  (std::numeric_limits<int>::max)();
            int len_c = (std::numeric_limits<int>::max)();
            int max_rout = 0;
            int counter = 0;

            while(yenAlg.has_next()) {
                shared_ptr<BasePath> BP = yenAlg.next();
                int weight_c = (int)(BP->Weight());
                if (weight_c > cost) {
                    break;
                } else {
                    if (BP->length() > len_c) {
                        continue;
                    }
                    //find k equal path and add 1 occurrence if node in  each k-path
                    cost = weight_c;
                    len_c = BP->length();
                    counter++;
                    vector<int> a_path;
                    for (int k = 0; k < BP->length(); ++k) {

                        a_path.push_back(BP->GetVertex(k)->getID());
                    }
                    //boost::mutex::scoped_lock(paths_mutex);
                    ////	BP->PrintOut(log);
                    paths[Pair(i,j)].push_back(a_path);
                }
            }//while
        }//else

    }
}

size_t Green::compute_all_shortest_path_multi_thread(PATHS & paths, size_t thread_num) {
    boost::timer t;
    size_t path_num = 0;

    log << "computing shortest pass... " << endl;

    //compute the shortest path

    vector<Pair> thread_blocks;
    for (size_t i = core_num_ + aggt_num_; i < nodes_.size(); ++i) {
        for (size_t j = i + 1; j < nodes_.size(); ++j) {
            thread_blocks.push_back(Pair(i,j));
//			log << " i " << i << " j " << j <<endl;
        }
    }


    size_t block_size = thread_blocks.size()/thread_num;
    log << "path : " << thread_blocks.size() << " thread_num : " << thread_num <<endl;
    log << "block size : " << block_size <<endl;


    vector<shared_ptr<std::thread>> threads_ptr;
    size_t s = 0;
    size_t e = s + block_size;
    while(e <= thread_blocks.size() && s < thread_blocks.size()) {
        log << "from : " << s << " to : " << e <<endl;
        std::shared_ptr<std::thread> compute_thread(new std::thread(std::bind(&Green::thread_compute_short_path,this,s,e,ref(thread_blocks),ref(paths))));

        threads_ptr.push_back(compute_thread);
        if (e == nodes_.size())

        {
            break;
        }
        s = e;
        e += block_size;

        if (e > thread_blocks.size()) {
            e = thread_blocks.size();
        }
    }

    for (size_t i = 0; i < threads_ptr.size(); ++i) {
        threads_ptr.at(i)->join();
    }
    log << "time : " << t.elapsed() <<endl;
    print_paths(paths,log);
    return paths.size();
}

///计算节点OC + LF
void Green::compute_node_occurrence( PATHS & paths ) {

    boost::timer t;
    log << "computing oc+lf ..."<<endl;

    ///clear the node occurrence and flow
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
        i->second.occurrence = 0;
        i->second.flow_chance = 0;
    }

    for (auto i = paths.begin(); i != paths.end(); ++i) {
        vector<vector<int>> &a_paths = i->second;

        for (vector<vector<int>>::size_type i = 0; i < a_paths.size(); ++i) {
            vector<int> &a_path = a_paths.at(i);
            int src = a_path.front();
            int dst = a_path.back();

            for (std::vector<int>::size_type j = 1; j < a_path.size() - 1; ++j) {
                int node_id = a_path.at(j);

                for (auto flow_i = s_d_flows_.lower_bound(Pair(src,dst)); flow_i != s_d_flows_.upper_bound(Pair(src,dst)); ++flow_i) {
                    double flow = flow_i->second;
                    if (flow != 0) {
                        nodes_.at(node_id).occurrence++;
                        nodes_.at(node_id).flow_chance += flow;
                    }
                }
            }
        }
    }
    log << "time : " << t.elapsed()<<endl;
}

///计算链路occ + flow_chance
void Green::compute_link_occurrence( PATHS & paths ) {
    boost::timer t;
    log << "computing links oc + lf..." <<endl;

    for (auto i = links_.begin(); i != links_.end(); ++i) {
        i->second.occurrence = 0;
        i->second.flow_chance = 0;
    }

    //       源      目的 N条     节点
    for(auto i = paths.begin(); i != paths.end(); ++i) {
        vector<vector<int>> & a_paths = i->second;

        for (size_t j = 0; j < a_paths.size(); ++j) {
            vector<int> &a_path = a_paths.at(j);
            int src = a_path.front();
            int dst = a_path.back();

            for (auto k = s_d_flows_.lower_bound(Pair(src,dst)); k != s_d_flows_.upper_bound(Pair(src,dst)); ++k) {
                double flow = k->second;

                for (size_t x = 1; x< a_path.size(); ++x) {
                    assert(flow != 0);
                    int start_v = a_path[x-1];
                    int	end_v = a_path[x];
                    Link &L = links_.at(Pair(start_v,end_v));
                    L.occurrence++;
                    L.flow_chance += flow;
                }
            }

        }
    }
    log << "time : " << t.elapsed() <<endl;
}

///判断图是否连通
bool Green::check_connected() {
    boost::timer t;
    log << "checking network connectivity...";


    std::stack<int> node_stack;
    std::set<int> node_set;
    int num_of_node_open = 0;
    int beg_node = 0;

    //寻找一个开着的节点作为起始点，统计开着的节点数量
    for (auto it = nodes_.begin(); it != nodes_.end(); ++it) {
        if (it->second.is_open() == true) {
            ++num_of_node_open;
            beg_node = it->second.get_id();
        }
    }

    node_stack.push(beg_node);
    node_set.insert(beg_node);

    while(!node_stack.empty()) {

        int node_id = node_stack.top();
        node_stack.pop();

        for (auto i = attach.at(node_id).begin(); i != attach.at(node_id).end(); ++i) {
            ///此节点还未被遍历，且此节点是打开的,并且到此节点的链路是联通的
            if (node_set.find(*i) == node_set.end() &&
                    nodes_.at(*i).is_open() &&
                    links_.at(Pair(node_id,*i)).Is_open() == true
               ) {
                node_stack.push(*i);
                node_set.insert(*i);
            }
        }
    }

    log << "time : " << t.elapsed() << endl;

    if (node_set.size() == num_of_node_open) {
        return true;
    } else {
        return false;
    }
}

///比较函数
template<typename T>
bool Green::less_occ_less_flow_chonce( const T& T1, const T& T2 ) {
    if (T1.Occurrence() < T2.Occurrence()) {
        return true;
    } else if(T1.Occurrence() == T2.Occurrence()) {
        return T1.Flow_chance() < T2.Flow_chance();
    } else {
        return false;
    }
}

void Green::recompute_shortest_path_with_changed_link(const Link & L, PATHS & paths, size_t thread_num) {
    boost::timer t;
    log << "re-computing changed paths..." << endl;

    Pair P(L.start_vertex,L.end_vertex);
    int counter = 0;
    vector <std::shared_ptr<std::thread>> ts;

    //遍历paths
    for (auto i = paths.begin(); i != paths.end(); ++i) {
        vector<vector<int>> & ps = i->second;
        bool flag = false;
        int s = ps.front().front();
        int d = ps.front().back();

        for (size_t x = 0; x < ps.size(); ++x) {
            assert(ps[x].size() > 2);
            for (size_t y = 0; y < ps[x].size()-1; ++y) {
                if (Pair(ps[x][y],ps[x][y+1]) == P) {
                    counter += ps.size();
                    flag = true;
                    break;
                }
            }

            if (flag == true) {
                break;
            }
        }

        if (flag == true) {
            paths.at(Pair(s,d)).clear();
            
	    mutex_c.wait();
            std::shared_ptr<std::thread> t( new std::thread(std::bind(&Green::recompute_short_path,this,s,d,ref(paths))));
            ts.push_back(t);
        }

    }


    for (size_t i = 0; i < ts.size(); ++i) {
        ts[i]->join();
    }

    log << "re-computer paths number : " << counter <<endl;
    log << "time : " << t.elapsed() << endl;

}

void Green::recompute_shortest_path_with_changed_node( int node_id, PATHS & paths, size_t thread_num ) {

    boost::timer t;
    log << "re-computing changed paths..." << endl;
    int counter = 0;
    vector <std::shared_ptr<std::thread>> ts;

    Pair P;
    //遍历paths
    for (auto i = paths.begin(); i != paths.end(); ++i) {

        vector<vector<int>> & ps = i->second;
        bool flag = false;
        int s = ps.front().front();
        int d = ps.front().back();

        for (size_t x = 0; x < ps.size(); ++x) {

            assert(ps[x].size() > 2);

            for (size_t y = 1; y < ps[x].size() - 1; ++y) {

                if (ps[x][y] == node_id) {
                    counter += ps.size();
                    P = i->first;
                    flag = true;
                    break;
                }
            }
            if (flag == true) {
                break;
            }
        }
        if (flag == true) {
//			log << "new compute : " << s << "->" << d <<endl;
            mutex_c.wait();
            std::shared_ptr<std::thread> t( new std::thread(std::bind(&Green::recompute_short_path,this,s,d,ref(paths))));
            ts.push_back(t);
        }

    }

    for (size_t i = 0; i < ts.size(); ++i) {
        ts[i]->join();
    }

    log << "re-computer paths number : " << counter <<endl;
    log << "time : " << t.elapsed() << endl;

}

void Green::recompute_short_path( int s, int e, PATHS & paths ) {
    paths.at(Pair(s,e)).clear();
    Graph G(links_,opened_node,opened_link);
    YenTopKShortestPathsAlg yenAlg(G);
    yenAlg.SetNextCompute(G.get_vertex(s),G.get_vertex(e));
    int cost =  (std::numeric_limits<int>::max)();
    int len_c = (std::numeric_limits<int>::max)();
    int max_rout = 0;
    int counter = 0;

    while(yenAlg.has_next()) {
        shared_ptr<BasePath> BP = yenAlg.next();
        int weight_c = (int)(BP->Weight());
        if (weight_c > cost) {
            break;
        } else {
            if (BP->length() > len_c) {
                continue;
            }
            //max_rout++;
            //find k equal path and add 1 occurrence if node in  each k-path
            cost = weight_c;
            len_c = BP->length();
            counter++;
            vector<int> a_path;
            for (int k = 0; k < BP->length(); ++k) {

                a_path.push_back(BP->GetVertex(k)->getID());
            }
            //BP->PrintOut(log);
            paths.at(Pair(s,e)).push_back(a_path);
        }
    }//while
    mutex_c.post();
}



void Green::store_path( ofstream &path_cache, PATHS &paths ) {
    boost::timer t;
    log << "store paths...";

    for (auto i = paths.begin(); i != paths.end(); ++i) {
        vector<vector<int>> &s_d_paths = i->second;
        for (size_t k = 0; k < s_d_paths.size(); ++k) {
            path_cache << i->first.peer_1 << " " << i->first.peer_2 << " " << s_d_paths[k].size() << " ";
            for (size_t j = 0; j < s_d_paths[k].size(); ++j) {
                path_cache << s_d_paths[k][j] << " ";
            }
            path_cache << endl;
        }
    }
    log << "time : " << t.elapsed() << endl;
}

void Green::restore_path( ifstream &paths_cache, PATHS &paths ) {
    log << "restore paths..." << endl;
    boost::timer t;
    int s,d,num;

    while(paths_cache>>s) {
        paths_cache >> d >> num;
        vector<int> a_path;
        int node;
        for (int j = 0; j < num; ++j) {
            paths_cache >> node;
            a_path.push_back(node);
        }
        paths[Pair(s,d)].push_back(a_path);
    }
    log << "time : " << t.elapsed() << endl;
}



void Green::print_path( const vector<vector<int>> & a_s_d_path ,ostream & out ) {
    if (a_s_d_path.size() == 0) {
        cout << "path null" <<endl;
        return;
    }

    out << " S : "<<a_s_d_path.front().front()
        << " D : " << a_s_d_path.front().back()
        << " number : " << a_s_d_path.size()
        << endl;

    for (size_t i = 0; i < a_s_d_path.size(); ++i) {
        for (size_t j = 0; j < a_s_d_path[i].size(); ++j) {
            out << a_s_d_path[i][j] << "->";
        }
        out << endl;
    }
}

void Green::print_paths( PATHS & paths , ofstream & out) {
    int sum = 0;
    for(auto i = paths.begin(); i != paths.end(); ++i) {
        vector<vector<int>> &s_d_paths = i->second;
        sum += s_d_paths.size();
        print_path(s_d_paths,out);
    }
    out << "^ paths num : " << sum << endl;
}

void Green::print_load( ostream &out ) {
    //以node为遍历单位
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
        int node_id = i -> first;
        out << "nodes : " << node_id  << " : " << endl;

        for (auto j = attach.at(node_id).begin(); j != attach.at(node_id).end(); ++j) {
            int dst = *j;
            Link & L  = links_.at(Pair(node_id,dst));
            if (L.state) {
                out << "ON | " << node_id << " <-> " << dst << " | "
                    << L.Flow() << " / " << L.Max_flow() << endl;
            } else {
                out << "OFF | " << node_id << " <-> " << dst << endl;
            }

        }
    }
}

double Green::sum_nodes() {

    //TODO:
    size_t sum = 0;
    for (auto i = nodes_.begin(); i != nodes_.end(); ++i) {
        if (i->second.state) {
            sum++;
        }
    }
    return sum;
}


double Green::sum_links() {
    //TODO:
    size_t sum = 0;

    for (auto i = links_.begin(); i != links_.end(); ++i) {
        if (i->second.state == true) {
            sum = true;
        }
    }
    return sum;
}

///计算能耗
double Green::compute_power() {
    //TODO:
    double node_num = sum_nodes();
    double link_num = sum_links();

    return node_num+link_num;
}








