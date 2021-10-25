#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char** argv){
    string path = "", path2 = "", str;
    path = argv[1]; path2 = argv[1];
    path2[0] = 'T';
    ifstream file(path);
    ofstream file2(path2);
    getline(file,str);

    file2 << str << endl;
    while(getline(file,str)){
        
        int start=0;
        int end = str.find(" ");
        if(end != -1){
            string A; string B; string C;
            A = str.substr(start, end - start);
            start = end + 1;
            end = str.find(" ", start);
            B = str.substr(start, end - start);
            start = end + 1;
            end = str.find(" ", start);
            C = str.substr(start, end - start);
            C = C.substr(0, C.size()-1);
            file2 << C << " -> " << A << ";" << endl;
        } else {
            file2 << str << endl;
        }
    }
    return 0;
}