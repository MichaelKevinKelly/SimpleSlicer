#include <limits>
#include <assert.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mesh.hpp"
#include "slices.hpp"

#define IMG_DIM 600

using namespace std;
using namespace cv;

Slices::Slices() {
	
	num_planes = 0;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 1;
	
	for (int i = 0; i < 3; i++) {
		dimensions[i][0] = numeric_limits<float>::max();
		dimensions[i][1] = numeric_limits<float>::lowest();
	}

}

void Slices::make_slices(Mesh *_mesh, float _slice_thickness) {

	my_mesh = _mesh;	
	slice_thickness = _slice_thickness;
	int num_facets = my_mesh->get_numFacets();

	get_dimensions(num_facets);
	init_planes();
	init_images();
	
	// Get line segments for each plane
	for (int i = 0; i < num_planes; i++) {
		
		float z = ((float) i) * slice_thickness;
		bool added = false;

		for (int j = 0; j < num_facets; j++) {
			lineSeg *curr = get_lineSeg(j, z);
			if (curr) {
				planes[i].push_back(curr);
				added = true;
			}
		}

	}

	// Get contours for each plane
	for (int i = 0; i < num_planes; i++) {
		
		int size = (int) planes[i].size();	
		
		for (int j = 0; j < size; j++) {
			Point start = Point( ((int)planes[i][j]->start[0]) + 300, ((int)planes[i][j]->start[1]) + 300);
			Point end = Point( ((int)planes[i][j]->end[0]) + 300, ((int)planes[i][j]->end[1]) + 300);
			line(slice_images[i], start, end, Scalar(255,0,0), 1);
		}

		Mat img_gray, invert_gray;
		vector< vector<Point> > curr_contours;
		
		cvtColor(slice_images[i], img_gray, CV_BGR2GRAY);
		bitwise_not(img_gray, invert_gray);
		findContours(invert_gray, curr_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		contours.push_back(curr_contours);

	}

}

lineSeg* Slices::get_lineSeg(int facet_index, float z) {

	facet *curr_facet = &my_mesh->mesh[facet_index];
	
	bool a_below = false, b_below = false, c_below = false;

	float a_dist = curr_facet->a[2] - z;
	float b_dist = curr_facet->b[2] - z;
	float c_dist = curr_facet->c[2] - z;

	if (a_dist < 0) a_below = true;
	if (b_dist < 0) b_below = true;
	if (c_dist < 0) c_below = true;
	
	lineSeg *line_seg = NULL;
	bool valid_intersect = false;
	int count = 0;

	if (a_below && b_below && c_below) return NULL;
	else if (!a_below && !b_below && !c_below) {
		
		if (!a_dist) count++;
		if (!b_dist) count++;
		if (!c_dist) count++;

		if (count == 2) valid_intersect = true;
		else if (count == 3) assert(-1 && "Triangle completely in plane....\n");

	} else valid_intersect = true;
	

	if (valid_intersect) {

		line_seg = new lineSeg();
	
		if (count) {
			
			if (!a_dist && !b_dist) {
				memcpy((void *)line_seg->start, (void*) curr_facet->a, (size_t) 12);
				memcpy((void *)line_seg->end, (void*) curr_facet->b, (size_t) 12);
			} else if (!b_dist && !c_dist) {
				memcpy((void *)line_seg->start, (void*) curr_facet->b, (size_t) 12);
				memcpy((void *)line_seg->end, (void*) curr_facet->c, (size_t) 12);
			} else if (!a_dist && !c_dist) {
				memcpy((void *)line_seg->start, (void*) curr_facet->a, (size_t) 12);
				memcpy((void *)line_seg->end, (void*) curr_facet->c, (size_t) 12);
			}

		} else {
			
			bool first_pt = true;

			if(a_below ^ b_below) {
				get_intersect(curr_facet->a, curr_facet->b, line_seg->start, z, 1);
				first_pt = false;
			}

			if (b_below ^ c_below) {
				if (first_pt) get_intersect(curr_facet->b, curr_facet->c, line_seg->start, z, 2);
				else get_intersect(curr_facet->b, curr_facet->c, line_seg->end, z, 2);
			}

			if (a_below ^ c_below) {
				float s = a_dist / (a_dist - c_dist);
				get_intersect(curr_facet->a, curr_facet->c, line_seg->end, z, 3);
			}

		}

	}

	return line_seg;
		
}

void Slices::get_intersect(float *a, float *b, float *out, float z, int _case) {
		
	float s_a = a[2] - z;
	float s_b = b[2] - z;
	float s = (s_a) / (s_a - s_b);
	
	memcpy((void *)out, (void *)a, (size_t)12);
	scale_vec(out, -1);
	add_vec(b, out, out);
	
	scale_vec(out, s);
	add_vec(a, out, out);

}

void Slices::get_dimensions(int num_facets) {
	assert(my_mesh->mesh && num_facets);
	
	for (int i = 0; i < num_facets; i++) {
		
		facet *curr = &my_mesh->mesh[i];
		
		for (int i = 0; i < 3; i++) {
			
			if (curr->a[i] < dimensions[i][0]) dimensions[i][0] = curr->a[i];
			if (curr->b[i] < dimensions[i][0]) dimensions[i][0] = curr->b[i];
			if (curr->c[i] < dimensions[i][0]) dimensions[i][0] = curr->c[i];

			if (curr->a[i] > dimensions[i][1]) dimensions[i][1] = curr->a[i];
			if (curr->b[i] > dimensions[i][1]) dimensions[i][1] = curr->b[i];
			if (curr->c[i] > dimensions[i][1]) dimensions[i][1] = curr->c[i];

		}

	}
}

void Slices::init_planes() {
	float height = dimensions[2][1] - dimensions[2][0];
	assert(height > 0);
	num_planes = ((int) (height / slice_thickness)) + 2;
	
	for (int i = 0; i < num_planes; i++) {
		vector<lineSeg *> curr_plane;
		planes.push_back(curr_plane);
	}

	assert(planes.size() == num_planes);
}

void Slices::init_images() {
	for (int i = 0; i < num_planes; i++) 
		slice_images.push_back(Mat(IMG_DIM, IMG_DIM, CV_8UC3, Scalar(255, 255, 255)));
}

int Slices::get_num_planes() {
	return num_planes;
}

float Slices::dot_product(float *u, float *v) {
	float ret = u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
	return ret;
}

void Slices::scale_vec(float *v, float s) {
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
}

void Slices::add_vec(float *u, float *v, float *w) {
	w[0] = v[0] + u[0];
	w[1] = v[1] + u[1];
	w[2] = v[2] + u[2];
}

Slices::~Slices() {
	
	for (int i = 0; i < num_planes; i++) {
		int size = (int) planes[i].size();
		for (int j = 0; j < size; j++) {
			delete planes[i][j];
		}
	}

}