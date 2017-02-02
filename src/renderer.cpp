#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "polygon.hpp"
#include "renderer.hpp"

using namespace std;
using namespace cv;

Renderer::Renderer(Slices *_my_slices) {
	my_slices = _my_slices;
	num_planes = my_slices->get_num_planes();
}

void Renderer::render(int contour_thickness, const bool show_path) {

	int mat_dim = my_slices->mat_dim;
	Mat base = Mat(mat_dim, mat_dim, CV_8UC3, Scalar(255, 255, 255));
	namedWindow("Slices", WINDOW_AUTOSIZE);
	
	int n = 0;
	while (n < 15) {

		for (int i = 0; i < num_planes; i++) {
			
			Mat temp = base.clone();
			
			Polygons *p_s = my_slices->slice_polygons[i];
			int num_polys = p_s->get_num_polys();

			assert(num_polys || i == 0 || i == num_planes - 1);
			if (!num_polys) continue;

			string level_data = "Level: " + to_string(i);
			string poly_data = "Num polys: " + to_string(num_polys);
			putText(temp, level_data, Point(60,30), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0,255,0));
			putText(temp, poly_data, Point(60,60), FONT_HERSHEY_SIMPLEX, 1.0f, Scalar(0,255,0));

			if (num_polys < 2) {
				
				Polygon *p = p_s->get_polygon(0);
				int num_points = p->get_size();

				cv::Scalar color = Scalar(255,0,0);
				if (p->is_open()) color = Scalar(0,0,255);

				// Loop to draw polygon
				for (int k = 0; k < num_points - 1; k++) {
					vertex<int> *curr = p->vertices[k];
					vertex<int> *nxt = p->vertices[k+1];
					cv::Point start = Point(curr->x, curr->y);
					cv::Point end = Point(nxt->x, nxt->y);
					line(temp, start, end, color, contour_thickness);
				}

				continue;	
			}


			Point start_p = Point(p_s->test_point.x, p_s->test_point.y);

			int first_poly_ind = p_s->path->order[0];
			Polygon *first_p = p_s->get_polygon(first_poly_ind);
			Point first_vert = Point(first_p->vertices[first_p->start_index]->x, first_p->vertices[first_p->start_index]->y);
			
			if (show_path) {
				circle(temp, start_p, 6, Scalar(0,55,0), 3, 8);
				arrowedLine(temp, start_p, first_vert, Scalar(0,255,0), 1);
			}

			for (int j = 0; j < num_polys; j++) {

				int prev_poly_ind, curr_poly_ind, nxt_poly_ind;
				if (j > 0) prev_poly_ind = p_s->path->order[j - 1];
				curr_poly_ind = p_s->path->order[j];
				if (j < num_polys - 1) nxt_poly_ind = p_s->path->order[j + 1];

				Polygon *p = p_s->get_polygon(curr_poly_ind);
				int num_points = p->get_size();

				cv::Scalar color = Scalar(255,0,0);
				if (p->is_open()) color = Scalar(0,0,255);

				// Loop to draw polygon
				for (int k = 0; k < num_points - 1; k++) {
					vertex<int> *curr = p->vertices[k];
					vertex<int> *nxt = p->vertices[k+1];
					cv::Point start = Point(curr->x, curr->y);
					cv::Point end = Point(nxt->x, nxt->y);
					line(temp, start, end, color, contour_thickness);
				}

				if (show_path) {
					
					int start_ind = p->start_index;
					int end_ind = p->end_index;

					vertex<int> *start = p->vertices[start_ind];
					vertex<int> *end = p->vertices[end_ind];

					circle(temp, Point(start->x, start->y), 4, Scalar(0,255,0), 2, 8);
					circle(temp, Point(end->x + 1, end->y + 1), 4, Scalar(0,0,255), 2, 8);

					// Point a0 = Point(p->bounding_rect[0].x - 1, p->bounding_rect[0].y - 1);
					// Point a2 = Point(p->bounding_rect[2].x + 1, p->bounding_rect[2].y + 1);
					// rectangle(temp, a0, a2, Scalar(0,50,0), 1, 8);

					if (j > 0) {
						Polygon *prev_poly = p_s->get_polygon(prev_poly_ind);
						vertex<int> *prev_vertex = prev_poly->vertices[prev_poly->end_index];
						Point avg_pt = Point(start->x + prev_vertex->x, start->y + prev_vertex->y);
						arrowedLine(temp, Point(prev_vertex->x, prev_vertex->y), Point(start->x, start->y), Scalar(0,255,0), 1);
					}
				}
				
				imshow("Slices", temp);
				if (show_path) waitKey(50);

			}

			waitKey(20);
				
		}
		
		n++;

	}

}



