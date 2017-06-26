#include <iostream>
#include <fstream>
#include <complex>

using namespace std;


struct Pixel {
	Pixel() : r(0), g(0), b(0) {};
	char r, g, b;
};


class Mandelbrot {
public:
	Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width);
	~Mandelbrot();
	void calculate(int threads);
	void save(const char * file_name);
private:
	int width, height;
	long double graph_width, graph_height;
	complex<long double> center;
	int * iterations;
	int pixel_count;
};


Mandelbrot::Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width) :
width(in_width), height(in_height), graph_width(in_graph_width), center(in_center){
	graph_height = graph_width * (in_height / in_width);
	pixel_count = width * height;
	iterations = new int[pixel_count];
}

Mandelbrot::~Mandelbrot(){
	delete [] iterations;
}

void Mandelbrot::calculate(int threads){

}

void Mandelbrot::save(const char * file_name){

}


int main(int argc, char ** argv){
	cout << "testing" << endl;


}

