#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
template<int n> struct vec {
    double data[n] = {0};
    double& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    double  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
};

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i=0; i<n; i++) out << v[i] << " ";
    return out;
}

template<> struct vec<3> {
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

typedef vec<3> vec3;

class Model {
private:
    std::string file;
    std::vector<vec3> verts = {}; //arr of vertices
    std::vector<int> facet_vrt = {}; // per triangle index

    void parseModel()  {
        using namespace std;
        ifstream in(file);
        if (in.fail()) {
            std::cerr << "Failed to open file: " << file << std::endl;
            return;
        }
        string line;
        while (getline(in, line)) {
            istringstream iss(line);
            string type;
            iss>>type;
            if (type == "v") {
                vec3 v;
                iss >> v.x >> v.y >> v.z;
                verts.push_back(v); // pushing to the verts array all v[x,y,z]
            }else if (type == "f") {
                std::vector<int> f;
                int idx;
                while (iss >> idx) {
                    idx--; // OBJ is 1-based, C++ is 0-based
                    f.push_back(idx);
                    
                    // consume trailing texture/normal info (e.g., /12/34)
                    char c = iss.peek();
                    while (c == '/') {
                        iss.get(); // eat '/'
                        if (iss.peek() != '/') {
                             int trash;
                             iss >> trash; // eat the next number
                        }
                        c = iss.peek();
                    }
                }
                // Triangulate faces (handles triangles, quads, polygons)
                for (int i = 0; i < (int)f.size() - 2; i++) {
                     facet_vrt.push_back(f[0]);
                     facet_vrt.push_back(f[i+1]);
                     facet_vrt.push_back(f[i+2]);
                }
            }
        }
        std::cerr << "# v# " << nverts() << " f# "  << nfaces()/3 << std::endl;
        std::vector<int> idx(nfaces());
        for(int i = 0; i < nfaces();i++) idx[i] = i;

        std::sort(idx.begin(),idx.end(), 
            [&](const int& a, const int& b) { //given two traingles compare their min z coord
                float aminz = std::min(vert(a,0).z,std::min(vert(a,1).z,vert(a,2).z));
                float bminz = std::min(vert(b,0).z,std::min(vert(b,1).z,vert(b,2).z));
                return aminz < bminz;    
            });
        std::vector<int> facet_vrt2(nfaces()*3);//allocate arr to store permutated facets
        for(int i = 0; i < nfaces(); i++) //for each new facet
            for(int j = 0; j < 3; j++)  // copy its three vertices ftom the old array
                facet_vrt2[i*3+j] = facet_vrt[idx[i]*3+j];
        
        facet_vrt = facet_vrt2; //store the sorted triangles
    }

public:
    Model() : file("") {}

    Model(char* fileName) : file(fileName) { parseModel();}
    
    Model& operator=(const std::string& fileStr) {
        if(file == fileStr) return *this; 
        
        file = fileStr;
        verts.clear();
        facet_vrt.clear();
        parseModel();
        return *this;
    }
    inline int nverts() const {return verts.size();} // num. of vertices
    inline int nfaces() const {return facet_vrt.size()/3;} // num. of triangles
    vec3 vert(const int i) const {return verts[i];}
    vec3 vert(const int iface, const int nthvert) const {return verts[facet_vrt[iface*3+nthvert]];}
    ~Model() {}
};