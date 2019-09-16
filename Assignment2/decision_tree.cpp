//
//  main.cpp
//  DataScience_assignment2
//
//  Created by 정태화 on 2019. 4. 11..
//  Copyright © 2019년 정태화. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;

class Node {
public:
    string name;
    map<string, Node*> nextNode; // multiple nextNodes
};

vector< vector< pair<string, string> > > readTraining(string);
string deleteCR(string);
Node* initTree(vector< vector< pair<string, string> > > trainingData);
Node* learning(string, vector< map<string, string> >, set<string>, map<string, set<string> > &);
double getInfo(string className, vector< map<string, string> > &trainingMap);
double getSelectedInfo(string className, vector< map<string, string> > &trainingMap, string selected_attr);
string getGainRatio(string className, vector< map<string, string> > &trainingMap, set<string> attrs);
pair<bool, string> isAllSame(string className, vector< map<string, string> > &trainingMap);
string majorityVote(string className, vector< map<string, string> > &trainingMap);
void makeResult(Node* cur, vector< vector< pair<string, string> > > &testData, string className, string path);
string classLabel(Node* cur, map<string, string> data);

int main(int argc, const char * argv[]) {
    // argv[0] : current program name
    // argv[1] : training file
    // argv[2] : test file
    // argv[3] : output file
    
    if(argc != 4) {
        cerr << "Input argument error." << endl;
        return -1;
    }
    string trainPath = argv[1];
    string testPath = argv[2];
    string resultPath = argv[3];
    
    vector< vector< pair<string, string> > > trainingData;
    vector< vector< pair<string, string> > > testData;
    
    trainingData = readTraining(trainPath);
    testData = readTraining(testPath);
    
    string className = trainingData[0].back().first;
    Node* decisionTree = initTree(trainingData);
    makeResult(decisionTree, testData, className, resultPath);
    
    return 0;
}

vector< vector< pair<string, string> > > readTraining(string path){
    ifstream inFile(path);
    string S;
    string token;
    vector< vector< pair<string, string> > > training_data;
    vector<string> attrs;
    vector< vector<string> > values;
    
    getline(inFile, S);
    S = deleteCR(S);
    
    // attr 이름 저장
    int pos = 0;
    string delimit = "\t";
    while((pos = (int)S.find(delimit)) != string::npos) { // find '\t' & tokenize
        token = S.substr(0, pos);
        attrs.push_back(token);
        S.erase(0, pos + delimit.length());
    }
    attrs.push_back(S);
    
    // attr value 저장
    while(!inFile.eof()){
        vector<string> tmp;
        getline(inFile, S);
        if(S.size() == 0)
            continue;

        S = deleteCR(S);
        
        while((pos = (int)S.find(delimit)) != string::npos) { // find '\t' & tokenize
            token = S.substr(0, pos);
            tmp.push_back(token);
            S.erase(0, pos + delimit.length());
        }
        tmp.push_back(S);
        values.push_back(tmp);
    }
    
    inFile.close();
    
    for(auto e : values){
        vector< pair<string,string> > tmpV;
        pair<string,string> tmpP;
        for(int i=0; i<e.size(); i++){
            tmpP = make_pair(attrs[i], e[i]);
            tmpV.push_back(tmpP);
        }
        training_data.push_back(tmpV);
    }
    
    return training_data;
}

/**
 Function to delete carriage return '\r'

 @param str : string with '\r'
 @return : string without '\r'
 */
string deleteCR(string str){
    string CR = "\r";
    auto pos = string::npos;
    
    while((pos = (int)str.find(CR)) != string::npos) {
//        cout << "found CR " << pos << "\n";
        str.erase(pos, pos + 1);
    }
    
    return str;
}

/**
 Initiation for building decision-tree.
 Transform data set vector into map, and make attribute name sets & values of each attribute

 @param trainingData : training data set
 @return : start learning process
 */
Node* initTree(vector< vector< pair<string, string> > > trainingData){
    string className = trainingData[0].back().first;
    
    vector< map<string, string> > training_map;
    set<string> attrs; // attribute's name
    map<string, set<string> > attr_values; // values of each attr
    
    /*
     ex) attr name -> { values(set) }
     income -> {high, med, low}
     age -> {under 31, 31~40, over 40}
     */
    
    for(auto data : trainingData){
        map<string, string> tmp;
        for(auto d : data){
            tmp.insert(d);
            attrs.insert(d.first);
            if(attr_values.find(d.first)!=attr_values.end())
                attr_values[d.first].insert(d.second);
            else{
                set<string> tmpS;
                tmpS.insert(d.second);
                attr_values.insert({d.first, tmpS});
            }
        }
        training_map.push_back(tmp);
    }
    attrs.erase(className);
//    cout << className << " erased!\n";

    return learning(className, training_map, attrs, attr_values);
}

/**
 Start learning process using training set.
 Build decision tree and finish with each leaf node as class label.

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @param attrs : attribute name list
 @param attr_values : value names of each attribute
 @return each tree's head
 */
Node* learning(string className, vector< map<string, string> > trainingMap, set<string> attrs, map< string, set<string> > &attr_values){
    /*
     Choose attribute with GainRatio measure.
     Build tree recursively till we meet 2 conditions to stop partitioning
     1. if all class labels are same in this specific node
     2. if there is no more attributes left
     */
    Node* node = new Node;
    
    // check if all classes are same
    pair<bool, string> allSameClass;
    allSameClass = isAllSame(className, trainingMap);
    
    if(allSameClass.first){
        node->name = allSameClass.second;
        return node;
    }
    
    // no more attrs left
    if(attrs.empty()){
        node->name = majorityVote(className, trainingMap);
        return node;
    }
    
    string selected_attr = getGainRatio(className, trainingMap, attrs);
//    cout << selected_attr << "\n";
    
    node->name = selected_attr;
    attrs.erase(selected_attr);
    
    for(auto v : attr_values[selected_attr]){ // partition db with selected attribute
        vector< map<string, string> > partition;
        for(auto data : trainingMap){
            if(data[selected_attr] == v)
                partition.push_back(data);
        }
        
        if(partition.size()==0){
            Node* next = new Node;
            next->name = majorityVote(className, trainingMap);
        }
        else{
            Node* next = learning(className, partition, attrs, attr_values);
            node->nextNode.insert({v, next});
        }
    }
    
    return node;
}

/**
 To identify class label when its situation is ambiguous.
 Used in case one partition of current node DB is 0.
 
 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @return : label with majority vote
 */
string majorityVote(string className, vector< map<string, string> > &trainingMap){
    string result;
    map<string, int> class_values;
    int maxx=0;
    
    for(auto data : trainingMap){
        string value = data[className];
        if(class_values.find(value) != class_values.end())
            class_values[value]++;
        else
            class_values.insert({value, 1});
    }
    
    for(auto v : class_values){
        if(maxx < v.second){
            maxx = v.second;
            result = v.first;
        }
    }
    
    return result;
}

/**
 Check if all class labels are same in this particular node.

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @return : True or False with name of value
 */
pair<bool, string> isAllSame(string className, vector< map<string, string> > &trainingMap){
    pair<bool, string> result;
    string first = trainingMap[0][className];
    for(auto data : trainingMap){
        string value = data[className];
        if(value != first){
            result = {false, value};
            return result;
        }
    }
    result = {true, first};
    
    return result;
}

/**
 Calculates expected information (entropy) needed to classify a tuple

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @return : value of expected information(entropy)
 */
double getInfo(string className, vector< map<string, string> > &trainingMap){
    double result = 0;
    int total = trainingMap.size();
    map<string, int> class_values; // value name : num
//    cout << className << "\n";
    
    for(auto data : trainingMap){
        string value = data[className];
//        cout << value << "\n";
        if(class_values.find(value) != class_values.end())
            class_values[value]++;
        else
            class_values.insert({value, 1});
    }
    
    for(auto i : class_values){
        double p = (double)i.second / (double)total;
//        cout << i.first << ":" << i.second << " ";
        result = result - (p*log2(p));
    }
//    cout << "\n";
    return result;
}

/**
 Calculates Expected information needed after using specific attribute to split current node.

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @param selected_attr : specific attribute name to split node
 @return : value of expected information selected
 */
double getSelectedInfo(string className, vector< map<string, string> > &trainingMap, string selected_attr){
    // selected is attribute
    double result = 0;
    int total = trainingMap.size();
    map<string, int> class_values; // 해당 클래스 value 종류 : 개수
    
    for(auto data : trainingMap){
        string value = data[selected_attr];
        if(class_values.find(value) != class_values.end())
            class_values[value]++;
        else
            class_values.insert({value, 1});
    }

    for(auto i : class_values){
        double p = (double)i.second / (double)total;
        vector< map<string,string> > partition;
        
        for(auto data : trainingMap){
            if(data[selected_attr] == i.first)
                partition.push_back(data);
        }
        
//        cout << "case [" << i.first << "]\n";
        double info = getInfo(className, partition);
        p = p*info; // already minus
        result = result + p;
    }
    
    return result;
}

/**
 Choose split point with the mininum expected information requirement
 to get the Gain Ratio.

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @param selected_attr : specific attribute name to split node
 @return : value of split info
 */
double getSplitInfo(string className, vector< map<string,string> > &trainingMap, string selected_attr){
    double result = 0;
    int total = trainingMap.size();
    map<string, int> class_values; // 해당 클래스 value 종류 : 개수
    
    for(auto data : trainingMap){
        string value = data[selected_attr];
        if(class_values.find(value) != class_values.end())
            class_values[value]++;
        else
            class_values.insert({value, 1});
    }
    
    for(auto i : class_values){
        double p = (double)i.second / (double)total;
        //        cout << i.first << ":" << i.second << " ";
        result = result - (p*log2(p));
    }
    
    return result;
}

/**
 Calculate Gain Ratio value to select the attribute to split DB.

 @param className : name of class label attribute
 @param trainingMap : training data set in the form of map
 @param attrs : attribute name list
 @return : name of selected attribute
 */
string getGainRatio(string className, vector< map<string, string> > &trainingMap, set<string> attrs){
    string result;
    double info, selectedInfo, splitInfo, info_gain, gain_ratio;
    double maxx = 0;
    
    for(auto a : attrs){
//        cout << "selected <" << a << "> case:\n";
        info = getInfo(className, trainingMap);
        selectedInfo = getSelectedInfo(className, trainingMap, a);
        splitInfo = getSplitInfo(className, trainingMap, a);
        info_gain = info - selectedInfo;
        gain_ratio = info_gain / splitInfo;
//        cout << "attr:" << a << " info:" << info << " Sinfo:" << Sinfo << " info_gain:" << info_gain << "\n";
        if(maxx < gain_ratio){
            maxx = gain_ratio;
            result = a;
        }
    }
    
    return result;
}

/**
 Write classification result data into output file 'result1.txt'.

 @param cur : current node
 @param testData : test data set
 @param className : name of class label attribute
 @param path : output file path
 */
void makeResult(Node* cur, vector< vector< pair<string, string> > > &testData, string className, string path){
    ofstream outFile(path);
    vector< map<string, string> > test_map; // every data set in the form of map
    vector<string> attrs;
    
    // transform all data into map type
    int i=0;
    for(auto data : testData){
        map<string, string> tmp;
        for(auto d : data){
            tmp.insert(d);
            if(i==0)
                attrs.push_back(d.first);
        }
        i++;
        test_map.push_back(tmp);
    }
    
    attrs.push_back(className);
    
    for(int i=0; i<attrs.size()-1; i++){
        outFile << attrs[i] << "\t";
    }
    outFile << attrs.back() << "\n";
    
    for(int i=0; i<testData.size(); i++){
        for(auto d : testData[i]){
            outFile << d.second << "\t";
        }
        outFile << classLabel(cur, test_map[i]) << "\n"; // test_map[i]
    }
    
    outFile.close();
}

/**
 Decide class label for each data.
 If there are no specific attribute values while traversing through the tree,
 use (kind of) majority voting method to decide its class label.

 @param cur : Current node
 @param data : each data map
 @return : class label result
 */
string classLabel(Node* cur, map<string,string> data){
    string cur_attr = cur->name;
    string cur_value = data[cur_attr];
    
    if(cur->nextNode.empty()){
//        cout << "classify finished!\n";
        return cur_attr;
    }
    else{
        if(cur->nextNode.find(cur_value) != cur->nextNode.end()){
            Node* next = cur->nextNode[cur_value];
            return classLabel(next, data);
        }
        else{ // if current attr_value is not in this node, use majority voting method to decide its class label
            map<string, int> class_values;
            // cout << "unexpected value inserted!!! -> (" << cur_attr << ":" << cur_value << ")\n";
            
            // follow down the tree from current node, and count each values of class label.
            for(auto &elem: cur->nextNode) {
                Node *next = (cur->nextNode)[elem.first];
                string val = classLabel(next, data);
                if(class_values.find(val) == class_values.end()) {
                    class_values.insert({val, 1});
                }
                else {
                    class_values[val]++;
                }
            }
            
            // and then return the class label with majority
            int maxx = 0;
            string majority;
            for(auto &elem: class_values){
                if(maxx < elem.second) {
                    maxx = elem.second;
                    majority = elem.first;
                }
            }
            
            return majority;
        }
    }
}
