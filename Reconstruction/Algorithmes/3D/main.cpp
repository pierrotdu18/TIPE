#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "reconstruction.cpp"
#include "geometry.cpp"
#include "miscellaneous.cpp"

using namespace std;

int main(int argc, char **argv) {
    if (argc <= 1) {
        cout << "You need to specify a file";
    }
    if (argc > 2) {
        cout << "There is only one argument to be specified";
    }
    ifstream ifile;
    ifile.open(argv[1], ios::in);
    if (!ifile.is_open()) {
        return -1;
    }
    string name = argv[1];
    name = remove_extension(name);
    name = name + (string)".cplx";
    ofstream ofile(name.c_str(), ios::out | ios::trunc);
    if (!ofile.is_open()) {
        return -1;
    }

    string dim;
    getline(ifile, dim);
    if (dim != "3") {
        cout << "This algorithm does only work for dimension 3";
        return 42;
    }

    vector<Point*> W;

    size_t i = 0;
    while (!ifile.eof()) {
        double x, y, z;
        ifile >> x;
        ifile >> y;
        ifile >> z;
        W.push_back(new Point(i, x, y, z));
        W[i]->simplices = make_unique<SimplicialComplex>();
        ++i;
    }

    reconstruction(W, ofile);
    ifile.close();
    ofile.close();
    return 0;
}
