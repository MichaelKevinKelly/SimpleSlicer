#include <limits>
#include <string>
#include <iostream>
#include <assert.h>
#include "polygons.hpp"

#define STITCHED_CLOSED -1
#define NO_STITCH 0
#define STITCHED_OPEN 1
#define MAX_DIST 1

using namespace std;

vertex<int>* Polygons::starting_point = nullptr; 
// final position of the extruder head in current slice, will be the starting point in the subsequent slice

/*
*
*	Creates a Polygons object, which stores a set of Polygon objects
*
*/
Polygons::Polygons(vector<vector<cv::Point> > *contours) {
	
	int num_polys = (int) contours->size();

	for (int i = 0; i < num_polys; i++) {
		Polygon *p = new Polygon(&(*contours)[i]);
		polys.push_back(p);
	}

	path = new Polypath();

	get_bounds();

}

Polygon* Polygons::get_polygon(int i) {
	return polys[i];
}

/*
*
*	Erase empty polygons, smooth polygons, and generate a path through the polygons
*	(treating each polygon as a node in a graph - intra-polygon paths will be handled 
*	separately)
*
*/

void Polygons::process_polygons() {
	
	test_point.x = Polygons::starting_point->x;
	test_point.y = Polygons::starting_point->y;

	for (int i = 0; i < this->get_num_polys(); i++) {
		if (!polys[i]->get_size()) {
			polys.erase(polys.begin() + i);
			i--;
			continue;
		} else {
			polys[i]->smooth();
		}
	}

	get_bounds();
	get_polypath();


}

int Polygons::get_num_polys() { return (int) polys.size(); }

void Polygons::get_polypath() {

	if (this->get_num_polys() > 1) {
		path->get_path(&polys, Polygons::starting_point);		
	}
}

void Polygons::get_bounds() {
	
	slice_bounds.x[0] = numeric_limits<int>::max();
	slice_bounds.x[1] = numeric_limits<int>::lowest();
	slice_bounds.y[0] = numeric_limits<int>::max();
	slice_bounds.y[1] = numeric_limits<int>::lowest();

	int num_polys = this->get_num_polys();
	for (int i = 0; i < num_polys; i++) {
		int num_points = polys[i]->get_size();
		for (int j = 0; j < num_points; j++) {
			if (polys[i]->poly_bounds.x[0] < slice_bounds.x[0]) slice_bounds.x[0] = polys[i]->poly_bounds.x[0];
			if (polys[i]->poly_bounds.x[1] > slice_bounds.x[1]) slice_bounds.x[1] = polys[i]->poly_bounds.x[1];
			if (polys[i]->poly_bounds.y[0] < slice_bounds.y[0]) slice_bounds.y[0] = polys[i]->poly_bounds.y[0];
			if (polys[i]->poly_bounds.y[1] > slice_bounds.y[1]) slice_bounds.y[1] = polys[i]->poly_bounds.y[1];
		}
	}

}

Polygons::~Polygons() {
	int num_polys = this->get_num_polys();
	for (int i = 0; i < num_polys; i++)
		delete polys[i];
	delete path;
}

