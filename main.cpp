#include <string>
#include <assert.h>

#include "mesh.hpp"
#include "slices.hpp"

using namespace std;
using namespace cv;

const float mesh_scale = 2.0f;
const float slice_thickness = 0.5f;
const int contour_thickness = 1;

void render(Slices *my_slices) {
	
	int num_planes = my_slices->get_num_planes();
	namedWindow("Slices", WINDOW_AUTOSIZE);
	vector< vector< vector<Point> > > contours = my_slices->contours;	
	Mat base = Mat(600, 600, CV_8UC3, Scalar(255, 255, 255));

	int i = 0;
	while(i < 10) {
		for (int i = 0; i < num_planes; i++) {
			if (my_slices->planes[i].size() > 0) {
				Mat temp = base.clone();
				drawContours(temp, contours[i], -1, Scalar(0,255,0), contour_thickness);
				imshow("Slices", temp);
				waitKey(15);	
			}
		}
		i++;	
	}

}

int main(int argc, char *argv[]) {
	
	string filename = string(argv[1]);

	Mesh *my_mesh = new Mesh();
	assert(my_mesh->load_STL(filename));
	my_mesh->scale_mesh(mesh_scale);
	
	Slices *my_slices = new Slices();
	my_slices->make_slices(my_mesh, slice_thickness);

	render(my_slices);

	delete my_mesh;
	delete my_slices;
	
}

