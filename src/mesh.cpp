#include <limits>
#include <assert.h>
#include "mesh.hpp"

using namespace std;

Mesh::Mesh() {
	num_facets = 0;
	mesh = NULL;

	for (int i = 0; i < 3; i++) {
		mesh_bounds[i][0] = numeric_limits<float>::max();
		mesh_bounds[i][1] = numeric_limits<float>::lowest();
	}
}

/**
*
*	Takes an STL file as input and generates a triangle mesh
*	Each triangle face is represented by a facet struct
*
**/
int Mesh::load_STL(string filename) {
	
	ifstream stl_file;
	stl_file.open(filename, ios::in | ios::binary);
	if (!stl_file) return 0;

	long facets_raw;
	stl_file.seekg(80, ios::beg);
	stl_file.read((char *)&facets_raw, 4);
	num_facets = (int) facets_raw;
	mesh = new facet[num_facets];
	if (!mesh) return 0;
	
	get_facets(&stl_file);
	get_bounds();
	center_mesh();
	
	return 1;

}

void Mesh::get_facets(ifstream *file_p) {
	for (int i = 0; i < num_facets; i++) {
		file_p->seekg(12, ios::cur);
		file_p->read((char *)mesh[i].a, 12);
		file_p->read((char *)mesh[i].b, 12);
		file_p->read((char *)mesh[i].c, 12);
		file_p->seekg(2, ios::cur);
	}
}

void Mesh::get_bounds() {
	
	for (int i = 0; i < num_facets; i++) {
		for (int j = 0; j < 3; j++) {	
			if (mesh[i].a[j] < mesh_bounds[j][0]) mesh_bounds[j][0] = mesh[i].a[j];
			if (mesh[i].b[j] < mesh_bounds[j][0]) mesh_bounds[j][0] = mesh[i].b[j];
			if (mesh[i].c[j] < mesh_bounds[j][0]) mesh_bounds[j][0] = mesh[i].c[j];
			if (mesh[i].a[j] > mesh_bounds[j][1]) mesh_bounds[j][1] = mesh[i].a[j];
			if (mesh[i].b[j] > mesh_bounds[j][1]) mesh_bounds[j][1] = mesh[i].b[j];
			if (mesh[i].c[j] > mesh_bounds[j][1]) mesh_bounds[j][1] = mesh[i].c[j];
		}
	}

	mesh_shift[0] = (mesh_bounds[0][1] + mesh_bounds[0][0]) / 2.0f;
	mesh_shift[1] = (mesh_bounds[1][1] + mesh_bounds[1][0]) / 2.0f;
	mesh_shift[2] = mesh_bounds[2][0];
}

void Mesh::scale_mesh(float scale) {
	assert(num_facets && mesh);
	for (int i = 0; i < num_facets; i++) {
		for (int j = 0; j < 3; j++) {
			mesh[i].a[j] *= scale;
			mesh[i].b[j] *= scale;
			mesh[i].c[j] *= scale;	
		}
	}
	get_bounds();
}

void Mesh::center_mesh() {
	for (int i = 0; i < num_facets; i++) {
		for (int j = 0; j < 3; j++) {	
			mesh[i].a[j] -= mesh_shift[j];
			mesh[i].b[j] -= mesh_shift[j];
			mesh[i].c[j] -= mesh_shift[j];	
		}
	}
	get_bounds();
}

int Mesh::get_numFacets() {
	return num_facets;
}

Mesh::~Mesh() {
	delete[] mesh;
}