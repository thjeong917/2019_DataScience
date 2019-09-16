//
//  main.cpp
//  DataScience_Term
//
//  Created by 정태화 on 2019. 6. 4..
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

using namespace std;

map<int, map<int, int> > readTrain(string path);
vector< vector<int> > mapToArray(map<int, map<int, int> > &trainSet);
double cosine_similarity(int user_a, int user_b);
pair<double, int> pcc(int user_a, int user_b);
map<int, vector< pair<double, int> > > getPccNeighbor();
double predict(int user, int item, map<int, vector< pair<double, int> > > &similarity_list);
void writePrediction(string basePath, string testPath, map<int, vector< pair<double, int> > > &similarity_list);

int max_user = 0;
int max_item = 0;
vector < vector<int> > rating_table;
map<int, int> user_avg;

int main(int argc, const char * argv[]) {
    // argv[0] : current program name
    // argv[1] : training file name
    // argv[2] : test file name
    
    if(argc != 3) {
        cerr << "Input argument error." << endl;
        return -1;
    }
    
    string basePath = argv[1];
    string testPath = argv[2];
    
    map<int, map<int, int> > trainSet;
    map<int, vector< pair<double, int> > > sim_list;
    
    trainSet = readTrain(basePath);
    rating_table = mapToArray(trainSet);
    sim_list = getPccNeighbor();
    
    writePrediction(basePath, testPath, sim_list);
    
    return 0;
}

map<int, map<int, int> > readTrain(string path){
    ifstream inFile(path, ifstream::in);
    map<int, map<int, int> > rating_map;
    map<int, int> tmp;
    int user, item, rating, time;
    
    while(!inFile.eof()) {
        inFile >> user >> item >> rating >> time;
        rating_map[user].insert({item, rating});
        if(item > max_item)
            max_item = item;
        if(user > max_user)
            max_user = user;
    }
    
    cout << "user:" << max_user << "  item:" << max_item << "\n";
    
    inFile.close();
    
    return rating_map;
}

vector< vector<int> > mapToArray(map<int, map<int, int> > &trainSet){
    vector < vector<int> > t_table;
    t_table.resize(max_user+1);
    for(auto &t : t_table)
        t.resize(max_item+1, 0);
    
    for(int i=1; i<=max_user; i++){
        double count = 0;
        double sum = 0;
        for(int j=1; j<=max_item; j++){
            t_table[i][j] = trainSet[i][j];
            if(trainSet[i][j]>0){
                count++;
                sum += trainSet[i][j];
            }
        }
        
        user_avg[i] = (int)round(sum/count);
    }
    
    return t_table;
}

double cosine_similarity(int user_a, int user_b){
    vector<int> a = rating_table[user_a];
    vector<int> b = rating_table[user_b];
    
    double dot = 0;
    double denom_a = 0;
    double denom_b = 0;
    double asize = a.size();
    
    for(int i=1; i<asize; i++){
        if(a[i] > 0 && b[i] > 0){
            dot += a[i]*b[i];
            denom_a += a[i]*a[i];
            denom_b += b[i]*b[i];
        }
    }
    
    return dot / (sqrt(denom_a) * sqrt(denom_b));
}

pair<double, int> pcc(int user_a, int user_b){
    vector<int> a = rating_table[user_a];
    vector<int> b = rating_table[user_b];
    double avg_a = 0;
    double avg_b = 0;
    double count = 0;
    double asize = a.size();
    
    for(int i=1; i<asize; i++){
        if(a[i] > 0 && b[i] > 0){
            avg_a += a[i];
            avg_b += b[i];
            count++;
        }
    }
    
    avg_a = avg_a / count;
    avg_b = avg_b / count;
    
    double dot = 0;
    double denom_a = 0;
    double denom_b = 0;
    
    for(int i=1; i<asize; i++){
        if(a[i] > 0 && b[i] > 0){
            dot += ((double)a[i] - avg_a) * ((double)b[i] - avg_b);
            denom_a += ((double)a[i] - avg_a) * ((double)a[i] - avg_a);
            denom_b += ((double)b[i] - avg_b) * ((double)b[i] - avg_b);
        }
    }
    
    double result = dot / (sqrt(denom_a) * sqrt(denom_b));
    
    return {result, count};
}

map<int, vector< pair<double, int> > > getPccNeighbor(){
    map<int, vector< pair<double, int> > > similarity_list;
    double sim = 0;
    double sim2 = 0;
    int count = 0;
    
    for(int i=1; i<=max_user; i++){
        for(int j=i+1; j<=max_user; j++){
            pair<double, int> tmpPair;
            tmpPair = pcc(i, j);
            sim = tmpPair.first;
            count += tmpPair.second;
            sim2 = cosine_similarity(i, j);
            sim = sim * sim2;
            
            if(!isnan(sim)){
                similarity_list[i].push_back(make_pair(sim, j));
                similarity_list[j].push_back(make_pair(sim, i));
            }
        }
        sort(similarity_list[i].rbegin(), similarity_list[i].rend());
    }
    
    return similarity_list;
}

double predict(int user, int item, map<int, vector< pair<double, int> > > &similarity_list){
    double rating = 0;
    double sim = 0;
    double result = user_avg[user];
    
    double cur_sim = 0;
    int neighbor_id = 0;
    int neighbor_size = similarity_list[user].size();
    
    for(int i=1; i<=neighbor_size; i++){
        cur_sim = similarity_list[user][i].first;
        neighbor_id = similarity_list[user][i].second;
        
        if(neighbor_id > max_user)
            continue;
        
        if(rating_table[neighbor_id][item] == 0)
            continue;
        
        if(cur_sim <= 0)
            break;
        
        rating += (cur_sim * (rating_table[neighbor_id][item] - user_avg[neighbor_id]));
        sim += (cur_sim);
    }
    
    result += rating / sim;
    
    if(isnan(result)){
        result = user_avg[user];
    }
    
    return result;
}

void writePrediction(string basePath, string testPath, map<int, vector< pair<double, int> > > &similarity_list){
    string outPath = basePath + "_prediction.txt";
    
    ifstream inFile(testPath, ifstream::in);
    set< pair<int,int> > testItems;
    int user, item, time;
    double rating = 0;
    
    while(!inFile.eof()) {
        inFile >> user >> item >> rating >> time;
        testItems.insert({user, item});
    }
    
    inFile.close();
    
    ofstream outFile(outPath, ofstream::out);
    for(auto &i : testItems){
        rating = predict(i.first, i.second, similarity_list);
        
        if(rating == 0)
            rating = 3;
        else if(rating < 1)
            rating = 1;
        else if(rating > 5)
            rating = 5;
        
        outFile << i.first << "\t" << i.second << "\t" << rating << "\n";
    }
    outFile.close();
}

