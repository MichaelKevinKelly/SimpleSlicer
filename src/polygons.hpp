#ifndef POLYGONS_H
#define POLYGONS_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "vertex.hpp"
#include "bounds.hpp"
#include "polygon.hpp"
#include "polypath.hpp"

class Polygons {	
	public:
		Polygons(std::vector<std::vector<cv::Point> > *contours);
		void process_polygons();
		int get_num_polys();
		Polygon* get_polygon(int i);
		~Polygons();
		Polypath *path;
		static vertex<int> *starting_point;
		vertex<int> test_point;
	private:
		void smooth_polygons();
		void get_bounds();
		void get_polypath();
		std::vector<Polygon*> polys;
		bounds<int> slice_bounds;
};

#endif