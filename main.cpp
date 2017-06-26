#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
#include <string>

using namespace std;

struct Pixel {
	Pixel() : r(0), g(0), b(0) {};
	char r, g, b;
};


class Mandelbrot {
public:
	Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width);
	~Mandelbrot();
	void calculate(int threads, int iterate_limit);
	void calculate_buddha(int threads, int iterate_limit);
	void save(const char * file_name);
private:
	int calc_pixel(complex<long double> c);
	void color(unsigned char * pixels);

	int width, height;
	long double graph_width, graph_height;
	complex<long double> center;
	int * iterations;
	int pixel_count;
	complex<long double> corner;
	long double step;
	int max_itrations;
};


Mandelbrot::Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width) :
width(in_width), height(in_height), graph_width(in_graph_width), center(in_center){
	graph_height = graph_width * ((double) height / width);
	pixel_count = width * height;
	iterations = new int[pixel_count];
	corner = center - complex<long double>(graph_width / 2.0L , - graph_height / 2.0L);
	step = graph_width / width;
}

Mandelbrot::~Mandelbrot(){
	delete [] iterations;
}

void Mandelbrot::calculate(int threads, int iterate_limit){
	max_itrations = iterate_limit;
	for(long int i = 0; i < pixel_count; ++i){
		int x = i % width;
		int y = i / width;
		iterations[i] = calc_pixel(corner + complex<long double>(step * x, - step * y));
	} 
}

int Mandelbrot::calc_pixel(complex<long double> c){
	complex<long double> z(0.0L, 0.0L);
	int i = 0;
	for (i = 0; i < max_itrations; ++i){
		z = z * z + c;
		if (pow(z.real(), 2) + pow(z.imag(), 2) > 4){
			break;
		}
	}
    return i;
}

void Mandelbrot::color(unsigned char * pixels){
	long int offset = 0;
	for(int i = 0; i < pixel_count; ++i){
		double value = (double) iterations[i] / max_itrations;
		if (value == 1)
			value = 0;
		pixels[offset++] = (unsigned char) (255 * pow(value, 10));
		pixels[offset++] = (unsigned char) (255 * pow(value, 3));
		pixels[offset++] = (unsigned char) (255 * value);
	}
}


void Mandelbrot::save(const char * file_name){
	unsigned char * pixels = new unsigned char[pixel_count * 3];
	color(pixels);
	ofstream outfile (file_name, ofstream::binary);

	string header = "P6 " + to_string(width) + " " + to_string(height) + " " + to_string(255) + "\n"; 
	cout << "saving file..." << endl;

	outfile.write (header.c_str(), header.size());
	outfile.write ((char *) pixels, pixel_count * 3);
}


void Mandelbrot::calculate_buddha(int threads, int iterate_limit){
	for(long int i = 0; i < pixel_count; ++i){
		iterations[i] = 0;
	}

	max_itrations = iterate_limit;
	for(long int i = 0; i < pixel_count; ++i){
		int x = i % width;
		int y = i / width;
		iterations[i] = calc_pixel(corner + complex<long double>(step * x, - step * y));
	} 
}


int main(int argc, char ** argv){
	Mandelbrot M(7016, 4960, complex<long double>(-0.5L, 0), 4.0L);
	M.calculate(1, 40);
	M.save("mandel.ppm");
}

