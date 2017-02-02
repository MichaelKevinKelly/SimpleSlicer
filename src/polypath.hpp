#ifndef POLYPATH_H
#define POLYPATH_H

#include <vector>
#include "polygon.hpp"

class Polypath {	
	public:
		Polypath();
		void get_path(std::vector<Polygon*> *polys, vertex<int> *starting_point);
		bool is_init();
		~Polypath();
		std::vector<int> order;
	private:
		int calculate_path(int curr_index);
		void get_vertex_ids(std::vector<Polygon*> *polys, int first_vertex_index);
		void generate_graph(std::vector<Polygon*> *polys);
		int get_min_dist(Polygon *a, Polygon *b, int *a_vert_ind, int *b_vert_ind);
		int get_closest_vert(vertex<int> *rect_vert, Polygon *p);
		int get_starting_poly(std::vector<Polygon*> *polys, vertex<int> *starting_point, int *starting_vert);
		void update_starting_point(std::vector<Polygon*> *polys, vertex<int> *starting_point);
		void check_ids(Polygon *p);
		int **adj_mat;
		int **vertex_index_mat;
		bool *visited;
		int num_nodes;
};

#endif