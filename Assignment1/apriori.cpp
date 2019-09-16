//
//  main.cpp
//  DataScience_assignment1
//
//  Created by 정태화 on 2019. 3. 20..
//  Copyright © 2019년 정태화. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

vector< set<int> > readTransactions(string p); //read input file
map< set<int>, int > makefreqMap(double &m, vector< set<int> > v); //generate frequent pattern set
void dfs(int start, int length, set<int> &items ,vector< set<int> > &result); //dfs for subsets
void makeAscRule(map< set<int>, int > &fqMap, string outP); //make association Rule and fwrite

double total; // total transaction size
vector<int> vc; // vector for dfs

int main(int argc, const char * argv[]) {
    // argv[0] : current program name
    // argv[1] : min_support
    // argv[2] : input file
    // argv[3] : output file
    if(argc != 4) {
        std::cerr << "Input argument error. Please check arguments." << endl;
        return -1;
    }
    
    double min_support = stof(argv[1]);
    string inPath = argv[2];
    string outPath = argv[3];
    vector< set<int> > transactions;
    map< set<int>, int > freqMap;
    
    transactions = readTransactions(inPath);
    total = transactions.size();
    freqMap = makefreqMap(min_support, transactions);
    makeAscRule(freqMap, outPath);
    
    return 0;
}

/**
 Function to read 'input.txt' and insert into vector

 @param path : Input file path
 @return : Total transactions
 */
vector< set<int> > readTransactions(string path){
    ifstream inFile(path);
    string S;
    vector< set<int> > transactions;
    
    while(!inFile.eof()){
        set<int> cur;
        getline(inFile, S);
        // parse each item
        int pos = 0;
        string delimit = "\t";
        while((pos = (int)S.find(delimit)) != string::npos) { // find '\t' & tokenize
            string token = S.substr(0, pos);
            cur.insert(stoi(token));
            S.erase(0, pos + delimit.length());
        }
        cur.insert(stoi(S));

        transactions.push_back(cur);
    }
    
    inFile.close();
    
    return transactions;
}

/**
 Depth-first search function for generating subsets

 @param start : Index for start
 @param length : Size of subset elements
 @param items : Superset
 @param result : Vector which contains result subsets
 */
void dfs(int start, int length, set<int> &items ,vector< set<int> > &result){
    vector<int> tmpVec;
    if (vc.size() == length){
        set<int> curSet;
        for (auto i : vc){
            curSet.insert(i);
        }
        result.push_back(curSet);
        return;
    }
    
    for(auto i: items)
        tmpVec.push_back(i);
    
    for (int i=start; i<tmpVec.size(); i++){
        if (vc.size() < length){
            vc.push_back(tmpVec[i]);
            dfs(i+1, length, items, result);
            vc.pop_back();
        }
    }
}

/**
 Function for generating frequent pattern set

 @param min_sup : Minimum support
 @param transactions : Total input transactions
 @return : Final frequent pattern set
 */
map< set<int>, int > makefreqMap(double &min_sup, vector< set<int> > transactions){
    map< set<int>, int > freqMap; // final frequent pattern set
    map< set<int>, int > prevMap; // (L) set
    map< set<int>, int > nextMap; // (L+1) set
    
    // generate (L=1) candidate set
    for(auto trans:transactions){
        for(auto t: trans){
            set<int> curSet;
            curSet.insert(t);
            
            if(freqMap.find(curSet)!=freqMap.end()) continue;

            int cnt=0;
            for(auto j:transactions){ // j는 set
                if(j.find(t)!=j.end())
                    cnt++;
            }
            double pct = (double)cnt/total*100;
            if(pct < min_sup) continue;
            
            freqMap.insert(pair< set<int>, int >(curSet, cnt));
        }
    }
    
    // generate candidate set
    /*
     procedure for generating (L+1) candidate set
     1. Self-joining
     2. Pruning
     */
    prevMap=freqMap;
    int L=2; // itemset length starts with 2
    while(1){
        for(auto iter=prevMap.begin(); iter!=prevMap.end(); iter++){
            /*
             newSet is each set of prevMap.
             if newSet passes every condition, it goes into nextMap.
             */
            set<int> newSet;
            
            for(auto iter2=prevMap.begin(); iter2!=prevMap.end(); iter2++){
                newSet = (*iter).first;
                set<int> curSet = (*iter2).first;
                
                // Self-join
                for(auto it=curSet.begin(); it!=curSet.end(); it++){
                    if(*it > *newSet.begin()){
                        newSet.insert(*it);
                    }
                }
                
                // skip this set if length is over/under L
                if(newSet.size()!=L) continue;
                
                // Pruning
                vector< set<int> > subsets;
                dfs(0, (int)newSet.size()-1, newSet, subsets); // generate every (L-1) subsets
                
                int mapflag=0; // if any of its (L-1) subset is infrequent, mapflag becomes 1
                for(int i=0; i<subsets.size(); i++){
                    if(prevMap.find(subsets[i])==prevMap.end())
                        continue;
                    
                    double pct = (double)(prevMap[subsets[i]])/total*100;
                    if(pct < min_sup){
                        mapflag=1;
                        break;
                    }
                }
                if(mapflag) continue;
                
                // check whether this set(newSet)'s support is higher than minimum support.
                int cnt=0;
                for(auto j:transactions){
                    int cntflag=1;
                    for(auto k:newSet){
                        // check whether this set(j) has all elements of newSet
                        if(j.find(k)==j.end()){
                            cntflag=0;
                            break;
                        }
                    }
                    if(cntflag) cnt++;
                }
                double pct = (double)cnt/total*100;
                if(pct < min_sup) continue;
                
                nextMap.insert(pair< set<int>, int >(newSet, cnt));
                freqMap.insert(pair< set<int>, int >(newSet, cnt));
            }
        }
        
        // if there is no more frequent pattern, loop stops
        if(nextMap.size()==0) break;
        
        // preparing prevMap for (L+1) set
        prevMap.clear();
        prevMap = nextMap;
        nextMap.clear();
        L++;
    }
    return freqMap;
}

/**
 Function generating association rule using frequent pattern set

 @param freqMap : Final frequent pattern set
 @param outPath : File path for 'output.txt'
 */
void makeAscRule(map< set<int>, int > &freqMap, string outPath){
    ofstream outFile(outPath);
    
    for(auto iter=freqMap.begin(); iter!=freqMap.end(); iter++){
        set<int> curSet = (*iter).first;
        
        if(curSet.size()==1) continue;
        
        for(int i=1; i<curSet.size(); i++){
            vector< set<int> > subsets; // every subsets of curSet
            dfs(0, i, curSet, subsets); // dfs for generating subsets
            
            /*
             curSet divided into sbset & rest
             - Support=sup{curSet}
             - Confidence=sup{curSet}/sup{sbset}
             */
            for(auto sbSet:subsets){
                int sup_curSet = freqMap[curSet];
                int sup_sbSet = freqMap[sbSet];
                string str_sbSet="{";
                string str_rest="{";
                double support, confidence;
                
                set<int> rest;
                set_difference(curSet.begin(), curSet.end(), sbSet.begin(), sbSet.end(), std::inserter(rest, rest.end()));
                
                for(auto i1: sbSet){
                    str_sbSet.append(to_string(i1));
                    str_sbSet+=",";
                }
                str_sbSet=str_sbSet.substr(0, str_sbSet.length()-1);
                str_sbSet+="}";
                for(auto i1: rest){
                    str_rest.append(to_string(i1));
                    str_rest+=",";
                }
                str_rest=str_rest.substr(0, str_rest.length()-1);
                str_rest+="}";
                
                support = (double)sup_curSet/total*100;
                confidence = (double)sup_curSet/(double)sup_sbSet*100;
                
                outFile << str_sbSet << "\t" << str_rest << "\t";
                outFile << fixed;
                outFile.precision(2);
                outFile << support << "\t" << confidence << "\n";
                outFile.unsetf(ios::fixed);
            }
        }
    }
    outFile.close();
    
    return;
}
