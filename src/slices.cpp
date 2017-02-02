#include <math.h>
#include <assert.h>
#include <limits>
#include <opencv2/highgui/highgui.hpp>

#include "slices.hpp"
#include "vertex.hpp"

#define EPSILON_FRAC 20
#define MIN_AREA_FRAC 100
#define BOUNDARY_EPSILON 2

using namespace std;

Slices::Slices() { num_planes = 0; }

/**
*
*	Slices the triangle mesh stored in a mesh object. Determines intersections between each z-slice
*	and the mesh, draws them to an openCV Mat, and then uses openCV findContours to extract contours.
*	Then, prunes invalid contours, generates polygons from the valid contours, and finally attempts to
*	generate a short path that visits all polygons. 
*
**/
void Slices::make_slices(Mesh *_mesh, float _slice_thickness, const int _mat_dim, const int _min_area) {

	my_mesh = _mesh;
	slice_thickness = _slice_thickness;
	mat_dim = _mat_dim;
	min_area = _min_area;
	int num_facets = my_mesh->get_numFacets();

	init_planes();
	init_images();
	
	// Get intersections between each plane/ slice and the mesh
	for (int i = 0; i < num_facets; i++) {

		facet *curr = &my_mesh->mesh[i];
		float z_min = get_min(curr->a[2], get_min(curr->b[2], curr->c[2]));
		float z_max = get_max(curr->a[2], get_max(curr->b[2], curr->c[2]));

		int low_plane = ceilf(z_min / slice_thickness);
		int high_plane = floorf(z_max / slice_thickness);

		assert(low_plane >= 0 && high_plane >= 0);
		for (int j = low_plane; j <= high_plane; j++)
			get_points(i, j);

	}
	
	// Get contours for each slice
	for (int i = 0; i < num_planes; i++) {
		
		int size = (int) slice_points[i].size();
		
		for (int j = 0; j < size; j++) {
			cv::Point start = cv::Point( ((int)slice_points[i][j]->x) + mat_dim/2, ((int)slice_points[i][j]->y) + mat_dim/2);
			j++;
			cv::Point end = cv::Point( ((int)slice_points[i][j]->x) + mat_dim/2, ((int)slice_points[i][j]->y) + mat_dim/2);
			cv::line(slice_images[i], start, end, cv::Scalar(255,0,0), 1);
		}

		cv::Mat img_gray, invert_gray;
		vector< vector<cv::Point> > curr_contours;
		
		cv::cvtColor(slice_images[i], img_gray, CV_BGR2GRAY);
		cv::bitwise_not(img_gray, invert_gray);
		cv::findContours(invert_gray, curr_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		contours.push_back(curr_contours);
		
	}

	// Remove invalid contours (e.g. duplicate contours, contours that are too small, etc.)
	prune_contours();

	vertex<int> origin;
	origin.x = 0;
	origin.y = 0;
	Polygons::starting_point = &origin;

	// Generate polygons and paths
	cout << "Making polygons....\n";
	for (int i = 0; i < num_planes; i++) {		
		cout << "Generating polys for plane " + to_string(i) + " (" + to_string(100 * (double)i/num_planes) + ")\n";
		Polygons *p = new Polygons(&contours[i]);
		p->process_polygons();
		slice_polygons.push_back(p);
	}
	cout << "\nFinished making polygons....\n";

}

/*
*
*	Get points of facet that intersect the plane with index plane_index
*
*/
void Slices::get_points(int facet_index, int plane_index) {

	float z = slice_thickness * (float) plane_index;
	facet *curr_facet = &my_mesh->mesh[facet_index];
	
	float a_dist = curr_facet->a[2] - z;
	float b_dist = curr_facet->b[2] - z;
	float c_dist = curr_facet->c[2] - z;

	int count = 0;
	if (!a_dist) count++;
	if (!b_dist) count++;
	if (!c_dist) count++;
 
	if (count == 1) return; 
	if (count == 3) return; // add functionality to deal with case where triangle lies entirely in plane

	vertex<float> *start = new vertex<float>();
	vertex<float> *end = new vertex<float>();

	if (count == 2) {
			
		if (!a_dist && !b_dist) {
			memcpy((void *)start, (void*) curr_facet->a, (size_t) 8);
			memcpy((void *)end, (void*) curr_facet->b, (size_t) 8);
		} else if (!b_dist && !c_dist) {
			memcpy((void *)start, (void*) curr_facet->b, (size_t) 8);
			memcpy((void *)end, (void*) curr_facet->c, (size_t) 8);
		} else if (!a_dist && !c_dist) {
			memcpy((void *)start, (void*) curr_facet->a, (size_t) 8);
			memcpy((void *)end, (void*) curr_facet->c, (size_t) 8);
		}

	} else {
			
		bool first_pt = true, a_below = false, b_below = false, c_below = false;

		if (a_dist < 0) a_below = true;
		if (b_dist < 0) b_below = true;
		if (c_dist < 0) c_below = true;

		if(a_below ^ b_below) {
			get_intersect(curr_facet->a, curr_facet->b, &start->x, z);
			first_pt = false;
		}

		if (b_below ^ c_below) {
			if (first_pt) get_intersect(curr_facet->b, curr_facet->c, &start->x, z);
			else get_intersect(curr_facet->b, curr_facet->c, &end->x, z);
		}

		if (a_below ^ c_below) {
			float s = a_dist / (a_dist - c_dist);
			get_intersect(curr_facet->a, curr_facet->c, &end->x, z);
		}

	}

	slice_points[plane_index].push_back(start);
	slice_points[plane_index].push_back(end);
		
}

void Slices::get_intersect(float *a, float *b, float *out, float z) {
		
	float ret[3];

	float s_a = a[2] - z;
	float s_b = b[2] - z;
	float s = (s_a) / (s_a - s_b);
	
	memcpy((void *)ret, (void *)a, (size_t)12);
	scale_vec(ret, -1);
	add_vec(b, ret, ret);
	
	scale_vec(ret, s);
	add_vec(a, ret, ret);

	memcpy((void *)out, (void *)ret, (size_t)8);

}

void Slices::init_planes() {
	float height = my_mesh->mesh_bounds[2][1] - my_mesh->mesh_bounds[2][0];

	assert(height > 0);
	num_planes = ((int) (height / slice_thickness)) + 2;
	
	for (int i = 0; i < num_planes; i++) {
		vector<vertex<float>*> curr_plane;
		vector<bounds<int>* > curr_bounds;
		slice_points.push_back(curr_plane);
		contour_bounds.push_back(curr_bounds);
	}	

}

void Slices::init_images() {
	for (int i = 0; i < num_planes; i++) 
		slice_images.push_back(cv::Mat(mat_dim, mat_dim, CV_8UC3, cv::Scalar(255, 255, 255)));
}

int Slices::get_num_planes() { return num_planes; }

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

float Slices::get_max(float x, float y) {
	if (x > y) return x;
	else return y;
}

float Slices::get_min(float x, float y) {
	if (x < y) return x;
	else return y;
}

void Slices::prune_contours() {	
	
	for (int i = 0; i < num_planes; i++) {
		
		int num_contours = (int) contours[i].size();
		
		for (int j = 0; j < num_contours; j++) {
			bounds<int> *b = new bounds<int>();
			b->x[0] = numeric_limits<int>::max();
			b->x[1] = numeric_limits<int>::lowest();
			b->y[0] = numeric_limits<int>::max();
			b->y[1] = numeric_limits<int>::lowest();

			int num_points = (int) contours[i][j].size();
			for (int k = 1; k < num_points - 1; k++) {
				int x = contours[i][j][k].x;
				int y = contours[i][j][k].y;
				if (x < b->x[0]) b->x[0] = x;
				if (x > b->x[1]) b->x[1] = x;
				if (y < b->y[0]) b->y[0] = y;
				if (y > b->y[1]) b->y[1] = y;
			}

			contour_bounds[i].push_back(b);	
		}

		for (int j = 0; j < (int) contours[i].size() - 1; j++) {
			
			double currArea = abs(contourArea(contours[i][j], false));

			if (currArea < (mat_dim/MIN_AREA_FRAC)) {
			 	cout << "contour erased (area)...\n";
			 	contours[i].erase(contours[i].begin() + j);
			 	contour_bounds[i].erase(contour_bounds[i].begin() + j);
			 	j--;
			} else {
				for (int k = j + 1; k < (int) contours[i].size(); k++) {
				
					if (abs(contour_bounds[i][j]->x[1] - contour_bounds[i][k]->x[1]) < BOUNDARY_EPSILON &&
						abs(contour_bounds[i][j]->x[0] - contour_bounds[i][k]->x[0]) < BOUNDARY_EPSILON &&
						abs(contour_bounds[i][j]->y[1] - contour_bounds[i][k]->y[1]) < BOUNDARY_EPSILON &&
						abs(contour_bounds[i][j]->y[0] - contour_bounds[i][k]->y[0]) < BOUNDARY_EPSILON
					) {
						if ((currArea - contourArea(contours[i][k], false)) < (mat_dim/EPSILON_FRAC)) {
							cout << "contour erased (overlap	)...\n";
							contours[i].erase(contours[i].begin() + k);
							contour_bounds[i].erase(contour_bounds[i].begin() + k);
							k--;
						}
					}

				}
			}

		}
		

	}

}

Slices::~Slices() {
	for (int i = 0; i < num_planes; i++) {
		int size = (int) slice_points[i].size();
		for (int j = 0; j < size; j++) {
			delete slice_points[i][j];
		}
		int num_contours = (int) contour_bounds[i].size();
		for (int j = 0; j < num_contours; j++) {
			delete contour_bounds[i][j];
		}
 	}
}