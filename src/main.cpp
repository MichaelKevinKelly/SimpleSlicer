#include <string>
#include <assert.h>

#include "mesh.hpp"
#include "slices.hpp"
#include "renderer.hpp"

using namespace std;
using namespace cv;

const float mesh_scale = 9.0f;
const float slice_thickness = 1.0f;
const int contour_thickness = 1;
const int dim = 900;
const int min_area = 0;
const bool show_path = true;

int main(int argc, char *argv[]) {
	
	string filename = string(argv[1]);

	printf("Loading mesh...\n");
	Mesh m;
	assert(m.load_STL(filename));
	m.scale_mesh(mesh_scale);

	printf("Slicing...\n");
	Slices s;
	s.make_slices(&m, slice_thickness, dim, min_area);

	printf("Rendering...\n");
	Renderer r(&s); 
	r.render(contour_thickness, show_path);
	
}

