#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <string>

typedef struct facet {
	float a[3];
	float b[3];
	float c[3];
} facet;

class Mesh {
	public:
		Mesh();
		int load_STL(std::string filename);
		int get_numFacets();
		void scale_mesh(float f);
		~Mesh();
		facet *mesh;
	private:
		void get_facets(std::ifstream *stl_file);
		int num_facets;
};

#endif