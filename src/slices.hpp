#ifndef SLICES_H
#define SLICES_H

#include <vector>
#include <opencv2/opencv.hpp>

#include "mesh.hpp"
#include "polygons.hpp"

class Slices {
	public:
		Slices();
		void make_slices(Mesh *_mesh, float _slice_thickness, const int _mat_dim, const int _min_area);
		int get_num_planes();
		~Slices();
		std::vector<std::vector<std::vector<cv::Point> > > contours;
		std::vector<std::vector<bounds<int>* > > contour_bounds;
		std::vector<cv::Mat> slice_images;
		std::vector<Polygons*> slice_polygons;
		int mat_dim;
		float slice_thickness;
	private:
		void init_planes();
		void init_images();
		void get_points(int facet_index, int plane_index);
		void get_intersect(float *a, float *b, float *out, float z);
		void scale_vec(float *v, float s);
		void add_vec(float *u, float *v, float *w);
		void prune_contours();
		float get_max(float x, float y);
		float get_min(float x, float y);
		std::vector<std::vector<vertex<float>*> > slice_points;
		Mesh *my_mesh;
		int num_planes;
		int min_area;
};

#endif