#include <iostream>
#include <iomanip>
#include <fstream>
#include <complex>
#include <cmath>
#include <random>
#include <string>
#include <thread>  
#include <atomic>
#include <mutex>

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
	void calculate(int th_count, int iterate_limit);
	void calculate_buddha(int th_count, int random_points, int iterate_limit_min, int iterate_limit_max);
	void save(const char * file_name);
	void save_buddha(const char * file_name);
private:
	void calculate_thread(int start, int block);
	void calculate_buddha_thread(int start, int block);
	void calculate_buddha_random_thread(int start, int block);
	
	int calc_pixel(complex<long double> c);
	void update_pixel(complex<long double> c, int i);
	
	void color(unsigned char * pixels);
	void color_buddha(unsigned char * pixels);

	complex<long double> number_from_index(int i);
	int index_from_number(complex<long double> i);
	int index_from_number_conjugate(complex<long double> i);
	void get_max_buddha_index(int start, int block);

	// member variables
	int                  width, height;
	long double          graph_width, graph_height;
	complex<long double> center;

	int *                iterations;
	atomic<int> *        iterations_buddha;

	int                  pixel_count;
	complex<long double> corner;
	long double          step;
	int                  max_itrations, min_iterations;
	int                  buddha_max_index;
	mutex                buddha_index_mx;
};


Mandelbrot::Mandelbrot(int in_width, int in_height, complex<long double> in_center, long double in_graph_width) :
width(in_width), height(in_height), graph_width(in_graph_width), center(in_center){
	graph_height = graph_width * ((double) height / width);
	pixel_count = width * height;
	iterations = new int[pixel_count];
	iterations_buddha = NULL;
	corner = center - complex<long double>(graph_width / 2.0L , - graph_height / 2.0L);
	step = graph_width / width;
	buddha_max_index = 0;
}

Mandelbrot::~Mandelbrot(){
	delete [] iterations;
	if (iterations_buddha){
		delete [] iterations_buddha;
	}
}


complex<long double> Mandelbrot::number_from_index(int i){
	int x = i % width;
	int y = i / width;
	return corner + complex<long double>(step * x, - step * y);
}

int Mandelbrot::index_from_number(complex<long double> c){
	if (fabs(c.real()) > fabs(corner.real()) || fabs(c.imag()) > fabs(corner.imag()))
		return -1;

	int x = abs(corner.real() - c.real()) / step;
	int y = abs(corner.imag() - c.imag()) / step;

	if (x >= width || y >= height || x < 0 || y < 0)
		return -1;

	return x + y * width;
}

int Mandelbrot::index_from_number_conjugate(complex<long double> c){
	if (fabs(c.real()) > fabs(corner.real()) || fabs(c.imag()) > fabs(corner.imag()))
		return -1;

	int x = abs(corner.real() - conj(c).real()) / step;
	int	y = abs(corner.imag() - conj(c).imag()) / step;

	if (x >= width || y >= height || x < 0 || y < 0)
		return -1;

	return x + y * width;
}

/* ----------------------------------------------
 * Calculate regular Mandelbrot set
 * ----------------------------------------------*/

/* Iterate over pixels */
void Mandelbrot::calculate(int th_count, int iterate_limit){
	max_itrations = iterate_limit;
	vector<thread> threads;
	int block = (int) (pixel_count / th_count);

	for (int i = 0; i < th_count; ++i){
		threads.push_back(thread(&Mandelbrot::calculate_thread, this, i * block, i - 1 == th_count ? pixel_count - i * block : block));
	}

	for (int i = 0; i < th_count; ++i){
		threads[i].join();
	}
}

/* Iterate over range of pixels */
void Mandelbrot::calculate_thread(int start, int block){
	for(long int i = start; i < start + block; ++i){
		int x = i % width;
		int y = i / width;
		iterations[i] = calc_pixel(corner + complex<long double>(step * x, - step * y));
	} 
}


/* Calculate value of a single pixel */
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

/* Color the set */
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

/* Prepare the values and save the file */
void Mandelbrot::save(const char * file_name){
	unsigned char * pixels = new unsigned char[pixel_count * 3];
	color(pixels);
	ofstream outfile (file_name, ofstream::binary);

	string header = "P6 " + to_string(width) + " " + to_string(height) + " " + to_string(255) + "\n"; 
	cout << "saving file..." << endl;

	outfile.write (header.c_str(), header.size());
	outfile.write ((char *) pixels, pixel_count * 3);
	outfile.close();
	delete [] pixels;
}


/* ----------------------------------------------
 * Calculate the Buddhabrot
 * ----------------------------------------------*/


/* calculate the buddha image */
void Mandelbrot::calculate_buddha(int th_count, int random_points, int iterate_limit_min, int iterate_limit_max){

	/* init the arrays */
	iterations_buddha = new atomic<int>[pixel_count];
	for(long int i = 0; i < pixel_count; ++i){
		iterations[i] = 0, iterations_buddha[i] = 0;
	}

	/* setup variables */
	max_itrations = iterate_limit_max;
	min_iterations = iterate_limit_min;
	vector<thread> threads;
	int block = (int) (pixel_count / th_count);

	cout << "prepare map" << endl;
	/* sets up the iterations map */
	calculate(th_count, max_itrations);

	cout << "normal calculate" << endl;
	/* iterate over every single point */
	for (int i = 0; i < th_count; ++i){
		int end_index = i - 1 == th_count ? pixel_count - i * block : block;
		threads.push_back(thread(&Mandelbrot::calculate_buddha_thread, this, i * block, end_index));
	}

	for (int i = 0; i < th_count; ++i){
		threads[i].join();
	}

	threads.clear();

	cout << "random calculate" << endl;
	/* iterate over random points */
	block = (int) (random_points / th_count);
	for (int i = 0; i < th_count; ++i){
		int end_index =  i - 1 == th_count ? random_points - i * block : block;
		threads.push_back(thread(&Mandelbrot::calculate_buddha_random_thread, this, i * block, end_index));
	}

	for (int i = 0; i < th_count; ++i){
		threads[i].join();
	}

	threads.clear();

	cout << "find max" << endl;
	/* calculate buddha index max */
	block = (int) (pixel_count / th_count);
	for (int i = 0; i < th_count; ++i){
		int end_index = i - 1 == th_count ? pixel_count - i * block : block;
		threads.push_back(thread(&Mandelbrot::get_max_buddha_index, this, i * block, end_index));
	}

	for (int i = 0; i < th_count; ++i){
		threads[i].join();
	}
	cout << buddha_max_index << endl;

}

void Mandelbrot::calculate_buddha_thread(int start, int block){
	complex<long double> c;
	int iter_count;

	for (int i = start; i < start + block; i++) {
		iter_count = iterations[i];
		if (iter_count < max_itrations && iter_count > min_iterations){
			c = number_from_index(i);
			update_pixel(c, iter_count);
		}
	}
}


void Mandelbrot::calculate_buddha_random_thread(int start, int block){
	/* setup the random generator */
	default_random_engine generator;
	uniform_real_distribution<double> distribution(0.0,1.0);
	complex<long double> c;
	int iter_count, index;

	for (int i = start; i < start + block; i++) {
		c = complex<long double>(4.0L * distribution(generator) - 2.0L, 4.0L * distribution(generator) - 2.0L);
		index = index_from_number(c);

		if (index == -1)
			continue;

		iter_count = iterations[index];
		if (iter_count < max_itrations && iter_count > min_iterations){
			update_pixel(c, iter_count);
		}
	}
}

void Mandelbrot::update_pixel(complex<long double> c, int i){
	complex<long double> z = c;
	int index;
	for (int j = 0; j < i - 1; j++) {
		
		/* original */
		index = index_from_number(z);

		/* check if is in the image */
		if (index == -1){
			z = z * z + c;
			continue;
		}
		iterations_buddha[index] += 1;
	
		/* mirror */
		index = index_from_number_conjugate(z);

		/* error not possible by now but for sure 
		if (index == -1){
			z = z * z + c;
			continue;
		} */

		iterations_buddha[index] += 1;

		/* new value */
		z = z * z + c;
	}
}


void Mandelbrot::get_max_buddha_index(int start, int block){
	int max = 0;
	for (int i = start; i < start + block; i++){
		if (iterations_buddha[i] > max)
			max = iterations_buddha[i];
	}

	buddha_index_mx.lock();
	if (buddha_max_index < max)
		buddha_max_index = max;
	buddha_index_mx.unlock();

}

void Mandelbrot::color_buddha(unsigned char * pixels){
	long int offset = 0;
	for(int i = 0; i < pixel_count; ++i){
		double value = (double) iterations_buddha[i] / buddha_max_index;
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
	outfile.close();
	delete [] pixels;
}


/* ----------------------------------------------
 * Parsing arguments and main
 * ----------------------------------------------*/


int * parse_args(int count, char ** args){
	int * args_parsed = new int[4];
	if (count < 5) {
       cerr << "Usage: " << args[0] << "width height iterations threads" << endl;
       delete [] args_parsed;
       return NULL;
    }
	args_parsed[0] = atoi(args[1]);
	args_parsed[1] = atoi(args[2]);
	args_parsed[2] = atoi(args[3]);
	args_parsed[3] = atoi(args[4]);
	return args_parsed;
}


int main(int argc, char ** argv){
	//int * args = parse_args(argc, argv);
	//if (args == NULL)
		//return 1;

	Mandelbrot M(1500, 1500, complex<long double>(-0.5L, 0), 4.0L);
	M.calculate_buddha(16, 1000000000, 10, 20);
	M.save_buddha("mandel2.ppm");
	//M.calculate(args[3], args[2]);
	//M.save("mandel.ppm");

	//delete [] args;
	return 0;
}


