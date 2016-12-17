#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

inline string remove_extension(string&);
double distance(size_t, size_t, vector< vector<double> >&);
double distance(size_t, size_t, vector< vector<double> >&);
double distance(size_t, vector<size_t>&, vector< vector<double> >&);
size_t farthest(vector< vector<double> >&, vector<size_t>&);
bool is_witnessed(size_t, size_t, vector< vector<double> >&, vector< vector<size_t> >&);
bool is_witnessed(size_t, size_t, size_t, vector< vector<double> >&, vector< vector<size_t> >&);
bool in(size_t, size_t, vector< vector< vector< vector<size_t> > > >&);
bool in(size_t, size_t, size_t, vector< vector< vector< vector<size_t> > > >&);
void reconstruction(vector< vector<double> >&, ofstream&);
void take_out(size_t, size_t, vector< vector< vector< vector<size_t> > > >*);
void take_out(size_t, size_t, size_t, vector< vector< vector< vector<size_t> > > >*);
void add_cwl(size_t, size_t, vector< vector< vector< vector<size_t> > > >*);
void add_cwl(size_t, size_t, size_t k, vector< vector< vector< vector<size_t> > > >*);
void add_rvl(size_t, vector< vector< vector<size_t> > >*);
void add_rvl(size_t, size_t, vector< vector< vector<size_t> > >*);
void add_rvl(size_t, size_t, size_t k, vector< vector< vector<size_t> > >*);
inline size_t _min(size_t, size_t);
inline size_t _max(size_t, size_t);

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
    }

    vector< vector<double> > W;
    size_t i = 0;
    while (!ifile.eof()) {
        vector<double> w(3, 0);
        W.push_back(w);
        double x, y, z;
        ifile >> x;
        ifile >> y;
        ifile >> z;
        W[i][0] = x;
        W[i][1] = y;
        W[i][2] = z;
        i++;
    }
    reconstruction(W, ofile);
    ifile.close();
    ofile.close();
    return 0;
}

void reconstruction(vector< vector<double> > &W, ofstream &ofile) {
    size_t n = W.size();
    for (size_t k = 0; k < n; ++k) {
        ofile << "\t" << W[k][0] << " " << W[k][1] << " " << W[k][2] << endl;
    }

    vector<size_t> L;
    L.push_back(0);

    vector< vector<size_t> > neighbours(n, vector<size_t>(3, 0));

    vector< vector < vector< vector<size_t> > > > CWL(3, vector < vector< vector<size_t> > >(n));

    size_t faces = 0;

    size_t i = 1;
    while (i < n) {
        size_t p = farthest(W, L);

        L.push_back(p);

        vector< vector< vector<size_t> > > reverse_landmarks(3);

        for (size_t k = 0; k < n; ++k) {
            size_t neigh1 = neighbours[k][0];
            size_t neigh2 = neighbours[k][1];
            size_t neigh3 = neighbours[k][2];
            double d1 = distance(k, neigh1, W);
            double d2 = distance(k, neigh2, W);
            double d3 = distance(k, neigh3, W);

            size_t n1 = _min(_min(neigh1, neigh2), neigh3);
            size_t n3 = _max(_max(neigh1, neigh2), neigh3);
            size_t n2 = neigh1 + neigh2 + neigh3 - n1 - n3;

            double dp = distance(k, p, W);
            if (d3 > dp || (n1 == n2 && n2 == n3)) {
                if (i - 1) {
                    add_rvl(n1, &reverse_landmarks);
                    add_rvl(n2, &reverse_landmarks);
                    add_rvl(n3, &reverse_landmarks);

                    add_rvl(n1, n2, &reverse_landmarks);
                    add_rvl(n2, n3, &reverse_landmarks);

                    add_rvl(n1, n2, n3, &reverse_landmarks);
                }
                neighbours[k][2] = p;
                if (d2 > d3) {
                    size_t t = neighbours[k][1];
                    neighbours[k][1] = neighbours[k][2];
                    neighbours[k][2] = t;
                }
                if (d1 > d2) {
                    size_t t = neighbours[k][0];
                    neighbours[k][0] = neighbours[k][1];
                    neighbours[k][1] = t;
                }
            }
        }

        if (i == 1) {
            add_cwl(L[0], p, &CWL);
        }
        else if (i == 2) {
            add_cwl(L[0], p, &CWL);
            add_cwl(L[1], p, &CWL);

            add_cwl(L[0], L[1], p, &CWL);
        }
        else {
            size_t q = reverse_landmarks[0].size();
            for (size_t k = 0; k < q; ++k) {
                if (is_witnessed(reverse_landmarks[0][k][0], p, W, neighbours)) {
                    add_cwl(reverse_landmarks[0][k][0], p, &CWL);
                }
            }

            q = reverse_landmarks[1].size();
            for (size_t k = 0; k < q; ++k) {
                if (in(reverse_landmarks[1][k][0], reverse_landmarks[1][k][1], CWL)) {
                    if (!is_witnessed(reverse_landmarks[1][k][0], reverse_landmarks[1][k][1], W, neighbours)) {
                        take_out(reverse_landmarks[1][k][0], reverse_landmarks[1][k][1], &CWL);
                    }
                }
                else {
                    if (is_witnessed(reverse_landmarks[1][k][0], p, W, neighbours)) {
                        add_cwl(reverse_landmarks[1][k][0], p, &CWL);
                    }
                    if (is_witnessed(reverse_landmarks[1][k][1], p, W, neighbours)) {
                        add_cwl(reverse_landmarks[1][k][1], p, &CWL);
                    }
                }
            }

            q = reverse_landmarks[2].size();
            for (size_t k = 0; k < q; ++k) {
                if (in(reverse_landmarks[2][k][0], reverse_landmarks[2][k][1], reverse_landmarks[2][k][2], CWL)) {
                    if (!is_witnessed(reverse_landmarks[2][k][0], reverse_landmarks[2][k][1], reverse_landmarks[2][k][2], W, neighbours)) {
                        take_out(reverse_landmarks[2][k][0], reverse_landmarks[2][k][1], reverse_landmarks[2][k][2], &CWL);
                        faces--;
                    }
                }
                else {
                    if (is_witnessed(reverse_landmarks[2][k][0], reverse_landmarks[2][k][1], p, W, neighbours)) {
                        add_cwl(reverse_landmarks[2][k][0], reverse_landmarks[2][k][1], p, &CWL);
                        faces++;
                    }
                    if (is_witnessed(reverse_landmarks[2][k][1], reverse_landmarks[2][k][2], p, W, neighbours)) {
                        add_cwl(reverse_landmarks[2][k][1], reverse_landmarks[2][k][2], p, &CWL);
                        faces++;
                    }
                }
            }
        }

        cout << "Reconstruction n. " << i << " : " << endl;
        ofile << "Reconstruction n. " << i << " : " << endl;


        /* Geomview OFF format */

        // Syntax : "OFF"
        ofile << "OFF" << endl;

        // Syntax : "number of vertices" "number of faces" "number of edges"
        ofile << i << " " << faces << endl;

        // Syntax : x y z for each vertex
        /*for (size_t k = 0; k < i; ++k) {
            ofile << "\t" << W[k][0] << " " << W[k][1] << " " << W[k][2] << endl;
        }*/

        // Syntax : n i j k ... for each face of n vertices which indexes are i j k...
        for (size_t k = 0; k < n; ++k) {
            size_t s = CWL[1][k].size();
            for (size_t l = 0; l < s; ++l) {
                ofile << 4 << " " << k  << " " << CWL[1][k][l][0]  << " "  << CWL[1][k][l][0] << endl;
            }
        }


        /*size_t m0 = CWL[0].size();
        size_t m1 = CWL[1].size();
        for (size_t k = 0; k < m0; ++k) {
            size_t s = CWL[0][k].size();
            for (size_t l = 0; l < s; ++l) {
                double x1 = W[k][0], y1 = W[k][1], z1 = W[k][2];
                double x2 = W[CWL[0][k][l][0]][0], y2 = W[CWL[0][k][l][0]][1], z2 = W[CWL[0][k][l][0]][2];
                ofile << "plt.plot((" << x1 << ", " << x2 << "), " << "(" << y1 << ", " << y2 << "), " << "(" << z1 << ", " << z2 << "))" << endl;

        }
        for (size_t k = 0; k < m1; ++k) {
            size_t s = CWL[1][k].size();
            for (size_t l = 0; l < s; ++l) {
                double x1 = W[k][0], y1 = W[k][1], z1 = W[k][2];
                double x2 = W[CWL[1][k][l][0]][0], y2 = W[CWL[1][k][l][0]][1], z2 = W[CWL[1][k][l][0]][2];
                double x3 = W[CWL[1][k][l][1]][0], y3 = W[CWL[1][k][l][1]][1], z3 = W[CWL[1][k][l][1]][2];
                ofile << "plt.plot((" << x1 << ", " << x2 << ", " << x3 << "), " << "(" << y1 << ", " << y2 << ", " << y3 << "), " << "(" << z1 << ", " << z2 << ", " << z3 << "))" << endl;

        }*/
        i++;
    }
}

inline string remove_extension(string &s) {
    size_t lastdot = s.find_last_of('.');
    if (lastdot == string::npos) return s;
    return s.substr(0, lastdot);
}

double distance(size_t p1, size_t p2, vector< vector<double> > &W) {
    return (W[p1][0] - W[p2][0]) * (W[p1][0] - W[p2][0]) + (W[p1][1] - W[p2][1]) * (W[p1][1] - W[p2][1]) + (W[p1][2] - W[p2][2]) * (W[p1][2] - W[p2][2]);
}

double distance(size_t p, vector<size_t> &L, vector< vector<double> > &W) {
    size_t l = L.size();
    double d = distance(p, L[0], W);
    for (size_t k = 0; k < l; ++k) {
        double dk = distance(p, L[k], W);
        if (dk < d) {
            d = dk;
        }
    }
    return d;
}

size_t farthest(vector< vector<double> > &W, vector<size_t> &L) {
    size_t n = W.size();
    size_t m = 0;
    double dm = distance(m, L, W);
    for (size_t j = 1; j < n; ++j) {
        double dj = distance(j, L, W);
        if (dm < dj) {
            dm = dj;
            m = j;
        }
    }
    return m;
}

bool is_witnessed(size_t i, size_t j, vector< vector<double> > &W, vector< vector<size_t> > &neighbours) {
    size_t n = W.size();
    for (size_t k = 0; k < n; ++k) {
        vector<size_t> neigh = neighbours[k];
        if ((neigh[0] == i && neigh[1] == j) || (neigh[0] == j && neigh[1] == i)) {
            return true;
        }
    }
    return false;
}

bool is_witnessed(size_t i, size_t j, size_t k, vector< vector<double> > &W, vector< vector<size_t> > &neighbours) {
    size_t n = W.size();
    for (size_t k = 0; k < n; ++k) {
        vector<size_t> neigh = neighbours[k];
        if ((neigh[0] == i && neigh[1] == j && neigh[2] == k) || (neigh[0] == i && neigh[1] == k && neigh[2] == j) ||
            (neigh[0] == j && neigh[1] == i && neigh[2] == k) || (neigh[0] == j && neigh[1] == k && neigh[2] == i)) {
            return true;
        }
    }
    return false;
}

bool in(size_t i, size_t j, vector< vector< vector< vector<size_t> > > > &CWL) {
    return find(CWL[0][i].begin(), CWL[0][i].end(), vector<size_t>{j}) != CWL[0][i].end();
}

bool in(size_t i, size_t j, size_t k, vector< vector< vector< vector<size_t> > > > &CWL) {
    return find(CWL[1][i].begin(), CWL[1][i].end(), vector<size_t>{j, k}) != CWL[1][i].end();
}

void take_out(size_t i, size_t j, vector< vector< vector< vector<size_t> > > > *CWL) {
    ((*CWL)[0][i]).erase(remove(((*CWL)[0][i]).begin(), ((*CWL)[0][i]).end(), vector<size_t>{j}), ((*CWL)[0][i]).end());
}

void take_out(size_t i, size_t j, size_t k, vector< vector< vector< vector<size_t> > > > *CWL) {
    ((*CWL)[1][i]).erase(remove(((*CWL)[1][i]).begin(), ((*CWL)[1][i]).end(), vector<size_t>{j, k}), ((*CWL)[1][i]).end());
}

void add_cwl(size_t i, size_t j, vector< vector< vector< vector<size_t> > > > *CWL) {
    ((*CWL)[0][i]).push_back({i});
}

void add_cwl(size_t i, size_t j, size_t k, vector< vector< vector< vector<size_t> > > > *CWL) {
    ((*CWL)[1][i]).push_back({j, k});
}

void add_rvl(size_t i, vector< vector< vector<size_t> > > *reverse_landmarks) {
    if (find(((*reverse_landmarks)[0]).begin(), ((*reverse_landmarks)[0]).end(), vector<size_t>{i}) == ((*reverse_landmarks)[0]).end()) {
        ((*reverse_landmarks)[0]).push_back({i});
    }
}

void add_rvl(size_t i, size_t j, vector< vector< vector<size_t> > > *reverse_landmarks) {
    if (find(((*reverse_landmarks)[1]).begin(), ((*reverse_landmarks)[1]).end(), vector<size_t>{i, j}) == ((*reverse_landmarks)[1]).end())
        ((*reverse_landmarks)[1]).push_back({i, j});
}

void add_rvl(size_t i, size_t j, size_t k, vector< vector< vector<size_t> > > *reverse_landmarks) {
    if (find(((*reverse_landmarks)[2]).begin(), ((*reverse_landmarks)[2]).end(), vector<size_t>{i, j, k}) == ((*reverse_landmarks)[2]).end())
        ((*reverse_landmarks)[2]).push_back({i, j, k});
}

inline size_t _min(size_t a, size_t b) {
    return a < b ? a : b;
}

inline size_t _max(size_t a, size_t b) {
    return a < b ? b : a;
}