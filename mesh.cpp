#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include "mesh.hpp"

using namespace std;

Mesh::Mesh() {
	num_facets = 0;
	mesh = NULL;
}

int Mesh::load_STL(string filename) {
	
	ifstream stl_file;
	stl_file.open(filename, ios::in | ios::binary);
	if (!stl_file) return 0;

	long facets_raw;
	stl_file.seekg(80, ios::beg);
	stl_file.read((char *)&facets_raw, 4);
	num_facets = (int) facets_raw;
	mesh = new facet[num_facets];
	
	get_facets(&stl_file);
	
	return 1;

}

void Mesh::scale_mesh(float scale) {
	for (int i = 0; i < num_facets; i++) {
		for (int j = 0; j < 3; j++) {
			mesh[i].a[j] *= scale;
			mesh[i].b[j] *= scale;
			mesh[i].c[j] *= scale;	
		}
	}
}

void Mesh::get_facets(ifstream *file_p) {
	assert(num_facets && mesh);
	for (int i = 0; i < num_facets; i++) {
		file_p->seekg(12, ios::cur);
		file_p->read((char *)mesh[i].a, 12);
		file_p->read((char *)mesh[i].b, 12);
		file_p->read((char *)mesh[i].c, 12);
		file_p->seekg(2, ios::cur);
	}
}

int Mesh::get_numFacets() {
	return num_facets;
}

Mesh::~Mesh() {
	delete[] mesh;
}