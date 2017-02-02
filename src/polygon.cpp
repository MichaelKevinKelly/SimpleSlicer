#include <math.h>
#include <assert.h>
#include "polygon.hpp"
#include <limits>

#define MAX_DIST 5
#define MAX_SMOOTH_DIST 0.3

using namespace std;

/*
*
* 	Polygon constructor converts openCV contour into Polygon-class format, 
*	by translating openCV points to a vector of vertex<int>'s
*
*/

Polygon::Polygon(std::vector<cv::Point> *contour) {
	int n = (int) contour->size();
	for (int i = 1; i < n; i++) {
		vertex<int> *v = new vertex<int>();
		v->x = (*contour)[i].x;
		v->y = (*contour)[i].y;
		vertices.push_back(v);
	}
	start_index = end_index = -1;
	update_bounds();
}

bool Polygon::is_open() {
	double dist = get_dist(vertices[0], vertices[(int) vertices.size() - 1]);
	return dist > MAX_DIST;
}

double Polygon::get_dist(vertex<int> *a, vertex<int> *b) {
	double x_dist = (double) (a->x - b->x);
	double y_dist = (double) (a->y - b->y);
	x_dist *= x_dist;
	y_dist *= y_dist;
	return sqrt(x_dist + y_dist);
}

/*
*	
*	Converts chains of contiguous line segments that closely approximate a line segment
*	into a single line segment
*
*/
void Polygon::smooth() {
	
	for (int i = 0; i < (int) vertices.size() - 1; i++) {
		int j = i + 2;
		while (can_compress(i,j)) j++;
		for (int k = i + 1; k < j - 1; k++) delete vertices[k];
		vertices.erase(vertices.begin() + i + 1, vertices.begin() + j - 1);
	}
	update_bounds();
	
}

bool Polygon::can_compress(int i, int j) {
	
	if (j >= (int) vertices.size()) return false;
	double dist_ij = get_dist(vertices[i], vertices[j]);
	double line[2] = { (double) (vertices[j]->x - vertices[i]->x), (double) (vertices[j]->y - vertices[i]->y) };
	line[0] /= dist_ij;
	line[1] /= dist_ij;

	assert(i+1 < j);
	for (int k = i + 1; k < j; k++) {
		double intersect[2] = { (double) (vertices[i]->x - vertices[k]->x), (double) (vertices[i]->y - vertices[k]->y) };
		double proj_length = line[0] * intersect[0] + line[1] * intersect[1];
		double proj[2] = { proj_length * line[0], proj_length * line[1] };
		double perp[2] = { intersect[0] - proj[0], intersect[1] - proj[1] };
		double dist = sqrt(perp[0] * perp[0] + perp[1] * perp[1]);
		if (dist > MAX_SMOOTH_DIST) return false;
	}
	
	return true;

}

void Polygon::reverse_vertices() { reverse(vertices.begin(), vertices.end()); }

bool Polygon::is_cw() {
	int sum = 0;
	for (int i = 0; i < (int) vertices.size() - 1; i++)
		sum += (vertices[i+1]->x - vertices[i]->x) * (vertices[i+1]->y + vertices[i]->y);
	return sum > 0;
};

int Polygon::get_size() { return (int) vertices.size(); }

void Polygon::update_bounds() {

	poly_bounds.x[0] = numeric_limits<int>::max();
	poly_bounds.x[1] = numeric_limits<int>::lowest();
	poly_bounds.y[0] = numeric_limits<int>::max();
	poly_bounds.y[1] = numeric_limits<int>::lowest();

	int num_vertices = this->get_size();
	for (int i = 0; i < num_vertices; i++) {
		if (vertices[i]->x < poly_bounds.x[0]) poly_bounds.x[0] = vertices[i]->x;
		if (vertices[i]->x > poly_bounds.x[1]) poly_bounds.x[1] = vertices[i]->x;
		if (vertices[i]->y < poly_bounds.y[0]) poly_bounds.y[0] = vertices[i]->y;
		if (vertices[i]->y > poly_bounds.y[1]) poly_bounds.y[1] = vertices[i]->y;
	}

	bounding_rect[0].x = bounding_rect[1].x = poly_bounds.x[0];
	bounding_rect[2].x = bounding_rect[3].x = poly_bounds.x[1];
	bounding_rect[0].y = bounding_rect[3].y = poly_bounds.y[0];
	bounding_rect[2].y = bounding_rect[1].y = poly_bounds.y[1];

}

Polygon::~Polygon() {
	for (int i = 0; i < (int) vertices.size(); i++) {
		delete vertices[i];
	}
}