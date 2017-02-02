#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "vertex.hpp"
#include "bounds.hpp"

class Polygon {	
	public:
		Polygon(std::vector<cv::Point> *contour);
		void smooth();
		void reverse_vertices();
		bool is_open();
		int get_size();
		static double get_dist(vertex<int> *a, vertex<int> *b);
		~Polygon();
		std::vector<vertex<int>*> vertices;
		bounds<int> poly_bounds;
		vertex<int> bounding_rect[4];
		int start_index;
		int end_index;
	private:
		bool is_cw();
		bool can_compress(int i, int j);
		void update_bounds();
};

#endif