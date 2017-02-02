#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <string>

struct facet {
	float a[3];
	float b[3];
	float c[3];
};

class Mesh {
	public:
		Mesh();
		int load_STL(std::string filename);
		int get_numFacets();
		void scale_mesh(float f);
		~Mesh();
		facet *mesh;
		float mesh_bounds[3][2];
	private:
		void get_facets(std::ifstream *stl_file);
		void get_bounds();
		void center_mesh();
		int num_facets;
		float mesh_shift[3];
};

#endif