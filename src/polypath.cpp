#include <limits>
#include "polypath.hpp"
#include "bounds.hpp"

using namespace std;

Polypath::Polypath() { num_nodes = -1; }

/*
*
*	A polypath object generates a path through the polygons in a given slice.
*	Treats each polygon as a node in a graph, uses a heuristic to gauge pairwise
*	distances between each polygon and to generate a complete graph representing
*	the slice. Polypath implements the nearest neighbor algorithm to quickly find a (not necessarily
*	optimal) tour.
*
*/
void Polypath::get_path(vector<Polygon*> *polys, vertex<int> *starting_point) {

	num_nodes = polys->size();

	adj_mat = new int*[num_nodes];
	vertex_index_mat = new int*[num_nodes];
	for (int i = 0; i < num_nodes; i++) {
		adj_mat[i] = new int[num_nodes];
		vertex_index_mat[i] = new int[num_nodes];
	}

	for (int i = 0; i < num_nodes; i++) {
		for (int j = 0; j < num_nodes; j++) {
			adj_mat[i][j] = -1;
			vertex_index_mat[i][j] = -1;
		}
	}

	visited = new bool[num_nodes];
	for (int i = 0; i < num_nodes; i++)
		visited[i] = false;

	generate_graph(polys);

	int first_vertex_index;
	int first_poly_index = get_starting_poly(polys, starting_point, &first_vertex_index);

	assert(calculate_path(first_poly_index));
	get_vertex_ids(polys, first_vertex_index);
	update_starting_point(polys, starting_point);

}

/*
*
*	Store a pointer to the ending point of the final polygon's intra-polygon path, as this
*	will be the starting point of the subsequent slice
*
*/
void Polypath::update_starting_point(vector<Polygon*> *polys, vertex<int> *starting_point) {
	Polygon *last_poly = (*polys)[order[num_nodes - 1]];
	vertex<int> *last_vertex = last_poly->vertices[last_poly->get_size() - 1];
	starting_point->x = last_vertex->x;
	starting_point->y = last_vertex->y;
}

/*
*
*	Recursive implementation of the nearest-neighbor algorithm to produce a solution to
*	the traveling salesman problem.
*
*/
int Polypath::calculate_path(int curr_index) {
	
	assert(curr_index < num_nodes);
	visited[curr_index] = true;
	order.push_back(curr_index);
	
	int min_dist = numeric_limits<int>::max();
	int min_index = -1;

	bool finished = true;
	for (int i = 0; i < num_nodes; i++) {
		if (!visited[i]) {
			finished = false;
			if (adj_mat[curr_index][i] < min_dist) {
				min_dist = adj_mat[curr_index][i];
				min_index = i;
			}
		}
	}

	if (finished) return 1;
	else {
		if (min_index < 0) return 0;
		else return calculate_path(min_index);
	}
	
}

/*
*
*	Store the indices of the starting and ending vertices for each polygon given the 
*	tour produced by calculate_path
*
*/
void Polypath::get_vertex_ids(vector<Polygon*> *polys, int first_vertex_index) {
	
	int first_poly = order[0];
	int second_poly = order[1];
	
	(*polys)[first_poly]->start_index = first_vertex_index;
	(*polys)[first_poly]->end_index = vertex_index_mat[first_poly][second_poly];
	
	check_ids((*polys)[first_poly]);

	for (int i = 1; i < num_nodes - 1; i++) {
		int prev_poly = order[i-1];
		int curr_poly = order[i];
		int next_poly = order[i+1];
		
		(*polys)[curr_poly]->start_index = vertex_index_mat[curr_poly][prev_poly];
		(*polys)[curr_poly]->end_index = vertex_index_mat[curr_poly][next_poly];
		
		check_ids((*polys)[curr_poly]);
	}

	int penult_poly = order[num_nodes - 2];
	int final_poly = order[num_nodes - 1];
	
	(*polys)[final_poly]->start_index = vertex_index_mat[final_poly][penult_poly];
	(*polys)[final_poly]->end_index = 0; // temporary -> this will ultimately be determined by the intra-polygon path
	check_ids((*polys)[final_poly]);

}

void Polypath::check_ids(Polygon *p) {
	assert(p->start_index >= 0);
	assert(p->end_index >= 0);
	assert(p->start_index < p->get_size());
	assert(p->end_index < p->get_size());
}

void Polypath::generate_graph(vector<Polygon*> *polys) {
	
	for (int i = 0; i < num_nodes; i++) {
		for (int j = i + 1; j < num_nodes; j++) {
			
			int i_vert_ind = -1, j_vert_ind = -1;
			assert(i < (int)polys->size());
			assert(j < (int)polys->size());
			
			Polygon *a = (*polys)[i];
			Polygon *b = (*polys)[j];
		
			adj_mat[i][j] = get_min_dist(a, b, &i_vert_ind, &j_vert_ind);
			adj_mat[j][i] = adj_mat[i][j];
			
			/*
			*	Given the edge from node/polygon i to j, vertex_index_mat[i][j] stores the index of the 
			*	vertex in polygon i associated with the edge, while vertex_index_mat[j][i] stores
			*	the corresponding vertex index for polygon j
			*/
			vertex_index_mat[i][j] = i_vert_ind;
			vertex_index_mat[j][i] = j_vert_ind;
		}
	}

}

/*
*
*	Min distance between two polygons is calculated as follows: First, find min distance between
*	each corner of the bounding rectangles of each polygon. Then, find the closest point in each polygon
*	to the corner of the bounding rect identified in the previous step. Finally, return the distance
*	between each point identified in part 2, and store the index vertices using the int pointer args
*
*/
int Polypath::get_min_dist(Polygon *a, Polygon *b, int *a_vert_ind, int *b_vert_ind) {
	
	assert(a != nullptr);
	assert(b != nullptr);

	double min_dist = numeric_limits<double>::max();
	int rect_point_a = -1;
	int rect_point_b = -1;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double curr_dist = Polygon::get_dist(&a->bounding_rect[i], &b->bounding_rect[j]); 
			assert(curr_dist >= 0);
			if ( curr_dist < min_dist ) {
				min_dist = curr_dist;
				rect_point_a = i;
				rect_point_b = j;
			}
		}
	}

	assert(rect_point_a >= 0);
	assert(rect_point_b >= 0);
	assert(min_dist >= 0);	

	*a_vert_ind = get_closest_vert(&a->bounding_rect[rect_point_a], a);
	*b_vert_ind = get_closest_vert(&b->bounding_rect[rect_point_b], b);

	assert(min_dist >= 0);
	return (int) min_dist;

}

/*
*
*	Returns the index of the vertex in Polygon p that is closest to the vertex point
*
*/
int Polypath::get_closest_vert(vertex<int> *point, Polygon *p) {
	
	int num_vertices = p->get_size();
	double min_dist = numeric_limits<double>::max();
	int closest_index = -1;

	for (int i = 0; i < num_vertices; i++) {
		double curr_dist = Polygon::get_dist(point, p->vertices[i]);
		if (curr_dist < min_dist) {
			min_dist = curr_dist;
			closest_index = i; 
		}
	}

	assert(closest_index >= 0);
	assert(min_dist >= 0);

	return closest_index;

}

/*
*
*	Returns the index of the polygon with the lowest heuristic distance from the slice's starting point
*
*/
int Polypath::get_starting_poly(vector<Polygon*> *polys, vertex<int> *starting_point, int *starting_vert) {
	
	assert(starting_point);
	int poly_index = -1;
	int rect_index = -1;
	double min_dist = numeric_limits<double>::max();

	for (int i = 0; i < num_nodes; i++) {
		for (int j = 0; j < 4; j++) {
			double curr_dist = Polygon::get_dist(starting_point, &(*polys)[i]->bounding_rect[j]);
			if (curr_dist < min_dist) {
				min_dist = curr_dist;
				poly_index = i;
				rect_index = j;
			} 
		}
	}

	assert(poly_index >= 0);
	assert(rect_index >= 0);

	*starting_vert = get_closest_vert(starting_point, (*polys)[poly_index]);

	return poly_index;
}

Polypath::~Polypath() {
	for (int i = 0; i < num_nodes; i++) {
		delete adj_mat[i];
		delete vertex_index_mat[i];
	}
	delete adj_mat;
	delete vertex_index_mat;
	delete visited;
}
