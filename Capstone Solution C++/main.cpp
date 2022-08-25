/**
 * @file main.cpp
 * @author C1C Colin Seymour
 * @brief Allow the user to input a ch10 file and interpret the data into human readable information
 * @version 0.1
 * @date 2022-08-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//basic c++ functions
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
using namespace std;

int main(){
    printf("F15 packet data analyzer");

    string fileName = "myChap10.ch10";

    //Ch10(fileName);

    string myText;

    // Read from the text file
    ifstream MyReadFile(fileName);

    // Use a while loop together with the getline() function to read the file line by line
    while (getline(MyReadFile, myText)) {
        // Output the text from the file
        cout << myText;
    }

    // Close the file
    MyReadFile.close();

    return 1;
}