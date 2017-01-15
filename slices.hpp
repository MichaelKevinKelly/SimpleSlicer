#ifndef SLICES_H
#define SLICES_H

#include <vector>
#include <opencv2/opencv.hpp>

typedef struct lineSeg {
	float start[3];
	float end[3];
	lineSeg *next;
} lineSeg;

class Slices {
	public:
		Slices();
		void make_slices(Mesh *_mesh, float _slice_thickness);
		std::vector< std::vector< std::vector<cv::Point> > > contours;
		std::vector<cv::Mat> slice_images;
		std::vector< std::vector<lineSeg *> > planes;
		int get_num_planes();
		~Slices();
	private:
		void init_planes();
		void init_images();
		void get_dimensions(int num_facets);
		lineSeg* get_lineSeg(int facet_index, float z);
		void get_intersect(float *a, float *b, float *out, float z, int _case);
		float dot_product(float *u, float *v);
		void scale_vec(float *v, float s);
		void add_vec(float *u, float *v, float *w);
		Mesh *my_mesh;
		int num_planes;
		float dimensions[3][2];
		float slice_thickness;
		float normal[3];
};

#endif