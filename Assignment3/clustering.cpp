//
//  main.cpp
//  DataScience_assignment3
//
//  Created by 정태화 on 2019. 5. 25..
//  Copyright © 2019년 정태화. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <cmath>
#include <typeinfo>

using namespace std;

map<int, pair<bool, pair<double, double> > > readInput(string &);
double getDistance(pair<double, double>, pair<double, double>);
set<int> getNeighbor(map<int, pair<bool, pair<double, double> > > &, int);
vector< set<int> > DBSCAN(map<int, pair<bool, pair<double, double> > > &);
vector< set<int> > map_to_set(map<int, int>, int);
void writeOutput(string &, vector< set<int> >);

int n;
double eps;
int minpts;

int main(int argc, const char * argv[]) {
    // argv[0] : current program name
    // argv[1] : input file name
    // argv[2] : # of clusters
    // argv[3] : eps
    // argv[4] : min_pts
    
    if(argc != 5) {
        cerr << "Input argument error." << endl;
        return -1;
    }
    string inputPath = argv[1];
    n = stoi(argv[2]);
    eps = stod(argv[3]);
    minpts = stoi(argv[4]);
    
    map< int, pair< bool, pair< double, double > > > points;
    vector< set<int> > clusterSet;
    
    points = readInput(inputPath);
    clusterSet = DBSCAN(points);
    writeOutput(inputPath, clusterSet);
    
    return 0;
}

map<int, pair<bool, pair<double, double> > > readInput(string &path){
    ifstream inFile(path, ifstream::in);
    map< int, pair< bool, pair< double, double > > > points;
    
    while(!inFile.eof()) {
        int id;
        double x, y;
        inFile >> id >> x >> y;
        points.insert({ id, {false, {x, y}} });
    }
    
    inFile.close();
    
    return points;
}

double getDistance(pair<double, double> a, pair<double, double> b){
    double x2 = (a.first - b.first) * (a.first - b.first);
    double y2 = (a.second - b.second) * (a.second - b.second);
    
    return sqrt(x2 + y2);
}

set<int> getNeighbor(map<int, pair<bool, pair<double, double> > > &points, int id){
    pair<double, double> core = points[id].second;
    set<int> N;
    
    for(auto &point : points){
        pair<double, double> candidate = point.second.second;
        if(getDistance(core, candidate) <= eps)
            N.insert(point.first);
    }
    
    return N;
}

vector< set<int> > DBSCAN(map<int, pair<bool, pair<double, double> > > &points){
    map<int, int> clusterMap;
    vector< set<int> > clusterSet;
    int cid = 0; // cluster id
    
    for(auto &point : points){
        if(point.second.first == true)
            continue;
        
        point.second.first = true;
        set<int> N = getNeighbor(points, point.first);
        
        if(N.size() >= minpts){
            clusterMap.insert({point.first, cid});
            
            while(1){
                int added_flag = 0;
                
                for(auto &pid : N){
                    auto p = points[pid];
                    // core point를 cluster에 등록
                    if(clusterMap.find(pid) == clusterMap.end())
                        clusterMap.insert({pid, cid});
                    // core point neighbor 찾기
                    if(p.first==false){
                        points[pid].first = true;
                        set<int> NN = getNeighbor(points, pid);
                        // NN을 따라가며 density-reachable일 경우 계속 cluster 넓혀갈 수 있게 N에 추가
                        if(NN.size() >= minpts){ // border point일 경우는 여기서 걸림
                            added_flag = 1;
                            for(auto &tmpid : NN)
                                N.insert(tmpid);
                        }
                    }
                    if(added_flag)
                        break;
                }
                
                if(added_flag == 0)
                    break;
            }
            cid++;
        }
    }
    
    clusterSet = map_to_set(clusterMap, cid);

    return clusterSet;
}

vector< set<int> > map_to_set(map<int, int> cMap, int cNum){
    vector< set<int> > clusterSet(cNum);
    int pid, cid;
    
    for(auto item : cMap){
        pid = item.first;
        cid = item.second;
        clusterSet[cid].insert(pid);
    }
    
    for(auto it = clusterSet.begin(); it != clusterSet.end();){
        auto tmp = *it;
        cout << tmp.size() << " ";
        if(tmp.size() < minpts)
            it = clusterSet.erase(it);
        else
            it++;
    }
    cout << endl;
    
    sort(clusterSet.rbegin(), clusterSet.rend(), [](set<int> &A, set<int> &B) {
        return A.size() < B.size();
    });
    
    while(clusterSet.size() > n)
        clusterSet.pop_back();
    
//    for(auto t : clusterSet){
//        cout << t.size() << " ";
//    }
//    cout << endl;
    
    return clusterSet;
}

void writeOutput(string &path, vector< set<int> > clusterSet){
    string input_file;
    input_file = path.substr(0, path.find("."));
    
    for(int i=0; i<clusterSet.size(); i++){
        string outputPath = input_file + "_cluster_" + to_string(i) + ".txt";
//        cout << output << "\n";
        ofstream outFile(outputPath);
        
        for(auto pid : clusterSet[i])
            outFile << pid << endl;
        
        outFile.close();
    }
}
