#include <iostream>
#include <iomanip>
#include <fstream>
#include <complex>
#include <cmath>
#include <random>
#include <string>

using namespace std;
#define BAR_SIZE 15


void update_process(double value, double max){
	double percentil = value / max;
	cout << "\rprocessing ";
	int i;
	for (i = 0; i < BAR_SIZE * percentil; ++i)
		cout << "-";

	for (; i < BAR_SIZE; ++i)
		cout << " ";

	cout << " " << setprecision (2) << fixed << percentil * 100 << flush;

}


class Mandelbrot {
public:
	Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width);
	~Mandelbrot();
	void calculate(int threads, int iterate_limit);
	void calculate_buddha(int threads, int count_points, int iterate_limit_min, int iterate_limit_max);
	void save(const char * file_name);
	void save_buddha(const char * file_name);
private:
	int calc_pixel(complex<long double> c);
	void update_pixel(complex<long double> c, int i);
	void color(unsigned char * pixels);
	void color_buddha(unsigned char * pixels);

	int width, height;
	long double graph_width, graph_height;
	complex<long double> center;
	int * iterations;
	int pixel_count;
	complex<long double> corner;
	long double step;
	int max_itrations;
	int buddha_max_index;
};


Mandelbrot::Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width) :
width(in_width), height(in_height), graph_width(in_graph_width), center(in_center){
	graph_height = graph_width * ((double) height / width);
	pixel_count = width * height;
	iterations = new int[pixel_count];
	corner = center - complex<long double>(graph_width / 2.0L , - graph_height / 2.0L);
	step = graph_width / width;
	buddha_max_index = 0;
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


void Mandelbrot::calculate_buddha(int threads, int count_points, int iterate_limit_min, int iterate_limit_max){
	default_random_engine generator;
	uniform_real_distribution<double> distribution(0.0,1.0);


	for(long int i = 0; i < pixel_count; ++i){
		iterations[i] = 0;
	}

	max_itrations = iterate_limit_max;
	complex<long double> c;
	int iter_count;

	cout << "grid points" << endl;
	for (int i = 0; i < pixel_count; i++) {
		int x = i % width;
		int y = i / width;
		c = corner + complex<long double>(step * x, - step * y);
		iter_count = calc_pixel(c);
		if (iter_count < iterate_limit_max && iter_count > iterate_limit_min) 
			update_pixel(c, iter_count);
		update_process(i, pixel_count);
	}

	cout << "\nrandom points" << endl;
	for (int i = 0; i < count_points; i++) {
		c = complex<long double>(4.0L * distribution(generator) - 2.0, 4.0L * distribution(generator) - 2.0);
		iter_count = calc_pixel(c);
		if (iter_count < iterate_limit_max && iter_count > iterate_limit_min) 
			update_pixel(c, iter_count);
		update_process(i, count_points);
	}
	cout << endl;

}


void Mandelbrot::update_pixel(complex<long double> c, int i){
	complex<long double> z = c;
	int x, y;
	for (int j = 0; j < i - 1; j++) {
		x = abs(corner.real() - z.real()) / step;
		y = abs(corner.imag() - z.imag()) / step;
		iterations[x + y * width] += 1;
		
		if (buddha_max_index < iterations[x + y * width]){
			buddha_max_index = iterations[x + y * width];
		}

		x = abs(corner.real() - conj(z).real()) / step;
		y = abs(corner.imag() - conj(z).imag()) / step;
		iterations[x + y * width] += 1;
		z = z * z + c;
	}
}

void Mandelbrot::color_buddha(unsigned char * pixels){
	long int offset = 0;
	for(int i = 0; i < pixel_count; ++i){
		double value = (double) iterations[i] / buddha_max_index;
		pixels[offset++] = (unsigned char) (255 * value);
		pixels[offset++] = (unsigned char) (255 * value);
		pixels[offset++] = (unsigned char) (255 * value);
	}
}

void Mandelbrot::save_buddha(const char * file_name){
	unsigned char * pixels = new unsigned char[pixel_count * 3];
	color_buddha(pixels);
	ofstream outfile (file_name, ofstream::binary);

	string header = "P6 " + to_string(width) + " " + to_string(height) + " " + to_string(255) + "\n"; 
	cout << "saving file..." << endl;

	outfile.write (header.c_str(), header.size());
	outfile.write ((char *) pixels, pixel_count * 3);
}

int main(int argc, char ** argv){

	Mandelbrot M(1500, 1500, complex<long double>(-0.5L, 0), 4.0L);
	M.calculate_buddha(1, 10000000, 10, 10000);
	M.save_buddha("mandel2.ppm");
	//M.calculate(1, 40);
	//M.save("mandel.ppm");
}

