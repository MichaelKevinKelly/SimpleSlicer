#ifndef RENDERER_H
#define RENDERER_H

#include "slices.hpp"

class Renderer {
	public:
		Renderer(Slices *_my_slices);
		void render(int contour_thickness, const bool show_path);
	private:
		Slices *my_slices;
		int num_planes;
};

#endif