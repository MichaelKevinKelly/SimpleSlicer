# CMake entry point
cmake_minimum_required (VERSION 2.8)
project (test)

find_package(
	OpenCV
)

include_directories(
	${OpenCV_INCLUDE_DIRS}
	/usr/local/cellar/opencv3/3.1.0_3/include/opencv2
	.
)

set(ALL_LIBS
	${OpenCV_LIBS}
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_core.dylib
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_imgcodecs.dylib
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_imgproc.dylib
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_highgui.dylib
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_videoio.dylib
	/usr/local/cellar/opencv3/3.1.0_3/lib/libopencv_video.dylib
)

add_executable(main
	src/main.cpp
	src/mesh.hpp
	src/mesh.cpp
	src/slices.hpp
	src/slices.cpp
	src/renderer.hpp
	src/renderer.cpp
	src/polygons.hpp
	src/polygons.cpp
	src/polygon.hpp
	src/polygon.cpp
	src/polypath.hpp
	src/polypath.cpp
	src/vertex.hpp
	src/bounds.hpp
)

target_link_libraries(main ${ALL_LIBS})
