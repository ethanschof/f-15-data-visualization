#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;
class Ch10 {
    private:
        string filename;


    public:

        Ch10(string fileName) {
            ifstream infile(fileName);

            infile >> hex;


        }
};