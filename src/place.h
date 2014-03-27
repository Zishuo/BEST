#pragma once
#include <map>
#include <vector>
#include "Pair.h"
#include "Graph.hpp"
#include "DijkstraShortestPathAlg.hpp"
//#include "lp_lib.h"
#include <lp_solve/lp_lib.h>

double place(
    std::map<int,multimap<int,double> > & service_flow,
    std::vector<std::vector<int> > & blocks,
    std::multimap<Pair,double> & s_d_flow_,
    std::map<Pair,Link> & links,
    std::map<int,int> & datacenter,
    int service_number,
    int node_num,
    int core_num,
    int aggt_num,
    int accs_num,
    double background_flow,
    const std::string & max_min,
    const std::string & out_prefix,
    const std::string & out_sd_flow,
    const std::string & out_place) {


    std::shared_ptr<Graph> G(new Graph(links,node_num,links.size()));
    DijkstraShortestPathAlg Dijk(G);
    map<int,int> S2J;
    map<Pair,double> Djk;
    int seq = 0;
    for(auto it = datacenter.begin(); it != datacenter.end(); ++it,++seq) {
        S2J[seq] = it->first;
    }

//		cout << "node_num : " <<node_num <<endl;
    for (int i = core_num + aggt_num; i < node_num; ++i) {
        for (int j = i + 1; j < node_num; ++j) {
            std::shared_ptr<BasePath> result = Dijk.get_shortest_path(G->get_vertex(i), G->get_vertex(j));
            Djk[Pair(i,j)] = result->Weight();
        }
    }

    ///* We will build the model row by row
    //So we start with creating a model with 0 rows and 2 columns */

    int I = service_number;
    int J = datacenter.size();
    map<int,multimap<int,double> > & Fik = service_flow;
    int Ncol = I*J; /* Xij so there are i*j var */
    lprec *lp = make_lp(0, Ncol);

    if(lp == NULL) {
        cerr << "couldn't construct a new model... " << endl;
        exit (1);
    }

    /////* let us name our variables. Not required, but can be useful for debugging */
    //set_col_name(lp, 1, "x");
    //set_col_name(lp, 2, "y");

    ///* create space large enough for one row */
    int * colno = (int *) malloc((Ncol+1) * sizeof(*colno));
    double * row = (double *) malloc((Ncol+1)* sizeof(*row));

    if((colno == NULL) || (row == NULL)) {
        cerr << "couldn't malloc ... " << endl;
        exit (2);
    }

    ///* makes building the model faster if it is done rows by row */
    set_add_rowmode(lp, TRUE);

    //set 0 <= x <= 1
    for (int c = 1; c <= Ncol; ++c) {
        set_upbo(lp,c,1);
    }



    ///* Service will be arranged in single Data Center*/
    for (int i = 0; i < I; ++i) {
        int j = 0;
        for (; j < J; ++j) {
            colno[j] = i*J + j + 1;
            row[j] = 1;
        }
        add_constraintex(lp,j, row, colno, EQ, 1);
        memset(colno,0,Ncol * sizeof(int));
        memset(row,0,Ncol*sizeof(double));
    }

    ///* Data Center only contain Cj Service*/
    for (int j = 0; j < J; ++j) {
        int i = 0;
        for ( ; i < I; ++i) {
            colno[i] = i * J + j + 1;
            row[i] = 1;
        }
        add_constraintex(lp,i, row, colno, LE, datacenter[S2J[j]]);
        memset(colno,0,Ncol * sizeof(int));
        memset(row,0,Ncol*sizeof(double));

    }


    /* rowmode should be turned off again when done building the model */


    /* Objective function */
    int c_ = 0;
    for (; c_ < Ncol ; ++c_) {
        double factor = 0;
        for (int k = aggt_num + core_num; k < node_num; ++k) {

            for (auto x = Fik[c_/J].lower_bound(k); x != Fik[c_/J].upper_bound(k); ++x) {
                factor+= x->second * Djk.at(Pair(S2J[c_%J],k));
            }

            //cout <<"====================================="<<endl;
            //cout <<" i : " << c_/J << endl;
            //cout <<" j : " << S2J[c_%J] << endl;
            //cout <<" k : " << k << endl;
            //cout <<"Fik: "<< Fik[c_/J][k] << endl;
            //cout <<"Djk: "<< Djk[S2J[c_%J]][k] << endl;
            //factor += Fik[c_/J][k] *  Djk[S2J[c_%J]][k];
            //cout <<"Cal: "<< Fik[c_/J][S2K[k]] *  Djk[S2J[c_%J]][S2K[k]] << endl;
        }
//			cout <<  "factor" << factor <<endl;
        colno[c_] = c_ + 1;
        row[c_] = factor;

    }
    if(!set_obj_fnex(lp, c_, row, colno)) {
        cerr << "Can not set objective function" << endl;
        return 4;
    }
    memset(colno,0,Ncol * sizeof(int));
    memset(row,0,Ncol*sizeof(double));


    ///* set the object direction to maximize */
    if (max_min == "max") {
        set_maxim(lp);
    } else if(max_min == "min") {
        set_minim(lp);
    } else {
        cerr << "para 1 is  limited in max/min" << endl;
        exit(3);
    }

    /* I only want to see important messages on screen while solving */
    set_verbose(lp, IMPORTANT);


    /* just out of curiosity, now show the model in lp format on screen */
    /* this only works if this is a console application. If not, use write_lp and a filename */
    string log_file_name = out_place+out_prefix + "_" + max_min + "_" + "place_log.txt";
    string place_file_name = out_place+out_prefix + "_" + max_min + "_" + "place.txt";
    ofstream place_stream(place_file_name);

    FILE * log_file = fopen(log_file_name.c_str(),"w");

    write_LP(lp,log_file);


    /* Now let lpsolve calculate a solution */
    solve(lp);


    /* objective value */
    double object_value = get_objective(lp);
    fprintf(log_file,"Objective value: %lf\n",object_value);
    fclose(log_file);
    /* variable values */
    get_variables(lp, row);

    //添加背景流量(用户->用户)


    ofstream log(log_file_name,ios_base::app);

    for(int j = 0; j < Ncol; j++) {

        int service_id = j/J;
        int service_loc = S2J[j%J];

        if (fabs(row[j] - 0) < 0.000001) {
            continue;
        }
        log << "service : " << service_id << " place : " << service_loc <<endl;


        for(auto i = Fik.at(service_id).begin(); i != Fik.at(service_id).end(); i++) {
            int user = i->first;
            double flow = i->second;

            s_d_flow_.insert(std::make_pair(Pair(service_loc,user),flow));
        }
    }

    for (int x = core_num + aggt_num; x != node_num; ++x) {
        for (int y = x + 1; y != node_num; ++y) {
            if (s_d_flow_.lower_bound(Pair(x,y)) != s_d_flow_.upper_bound(Pair(x,y))) {
                continue;
            } else {
                s_d_flow_.insert(std::make_pair(Pair(x,y),background_flow));
            }
        }
    }

    for(auto i = s_d_flow_.begin(); i != s_d_flow_.end(); ++i) {
        place_stream << i->first.peer_1  << " " << i->first.peer_2  << " " << i->second <<std::endl;
    }

    /* free allocated memory */
    if(row != NULL)
        free(row);
    if(colno != NULL)
        free(colno);

    if(lp != NULL) {
        /* clean up such that all used memory by lpsolve is freed */
        delete_lp(lp);
    }
    return object_value;

};



