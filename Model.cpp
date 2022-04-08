//
// Created by ingebrigt on 21.01.2022.
//

#include "Model.h"

#include <utility>
#include <fstream>
#include <iterator>
#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/utility/dim.hpp>

#define DELIMITER ' '


namespace compute = boost::compute;
namespace po = boost::program_options;
double learning_rate = 0.1;

template <typename T>
std::vector<std::vector<T>> operator-=(std::vector<std::vector<double>> &x, SquareArray<T> y) {
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < x[i].size(); ++j) {
            x[i][j] -= y[i][j];
        }
    }
    return x;
}


template <typename T>
double Model<T>::f(int i, SquareArray<T> const &x) {
    return std::exp(this->w.calc(x, i)/this->sigma);
}
static double test = 0;
template <typename T>
void Model<T>::update(int j) {
    kernel.set_arg(3,j);

    using compute::uint_;
    queue.enqueue_nd_range_kernel(kernel, compute::dim(0, 0), compute::dim(16, 1), compute::dim(1,1));
}


template <typename T>
void Model<T>::save(const char &subfigure) {
    std::string path = "../saved/figure2";
    path.push_back(subfigure);
    path.append(".fig");
    std::ofstream output_file(path);

    std::cout << "Saving figure" << std::endl;

    for(size_t layer = 0; layer < filters; layer++) {
        for (size_t row = 0; row < resolution; row++) {
            auto temp2 = w[layer];
            auto temp = temp2[row];
            std::ostream_iterator<double> output_iterator(output_file, " ");
            std::copy(temp.begin(), temp.end(), output_iterator);
            output_file << "\n";
        }
        output_file << "\n";
    }

    /*
    for (int x = 0; x < filters; ++x) {
        for (int y = 0; y < resolution; ++y) {
            std::ostream_iterator<double> output_iterator(output_file, " ");
            std::copy(w.cube.begin(), w.cube.begin() + resolution, output_iterator);
            output_file << "\n";
        }
        output_file << "\n";
    }
    */
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

template <typename T>
bool Model<T>::load(const char &subfigure) {
    std::string path = "../saved/figure2";
    path.push_back(subfigure);
    path.append(".fig");
    if(!std::filesystem::exists(path)){
        std::cout << "Figure " << subfigure << " not found." << std::endl;
        return false;
    }
    std::ifstream file(path);

    std::string line;
    this->w.cube.clear();

    std::vector<T> inner = {};

    while (std::getline(file, line)) {
        rtrim(line);
        // TODO The following line may or may not need to be active, depending on system locale \
            If filter plots are empty, try (un)commenting it.
        //std::replace(line.begin(), line.end(), '.', ',');
        size_t last = 0, next;
        while ((next = line.find(DELIMITER, last)) != std::string::npos) {
            inner.emplace_back(std::stod(line.substr(last, next-last)));
            last = next + 1;
        }
        if(!line.substr(last).empty()){
            inner.emplace_back(std::stod(line.substr(last)));
        }
    }

    this->w.cube.swap(inner);
    return true;
}


template<typename T>
compute::program Model<T>::make_sma_program(const compute::context &context) {
    const char source[] = BOOST_COMPUTE_STRINGIZE_SOURCE (
            double f(int i, __global double *mun, __global double *xv) {
                double sum = 0;
                for (int j = 0; j < resolution; ++j) {
                    for (int k = 0; k < resolution; ++k) {
                        sum += pow((xv[(j * resolution) + k] - mun[i * resolution * resolution + (j * resolution) + k]), 2);
                    }
                }
                sum = -sum;
                sum /= sigma;
                sum = exp(sum);
                return sum;
            }

            __kernel void SMA(__global double *mu, __local double *diff, __global double *x_vec, int current_batch) {
                int i1 = get_global_id(0);
                for (int j = 0; j < resolution * resolution; ++j) {
                    diff[i1 * resolution * resolution + j] = (x_vec[current_batch * resolution * resolution + j] - mu[i1 * resolution * resolution + j]) * f(i1,  mu, &x_vec[current_batch * resolution * resolution]);
                }
                for(int i2 = 0; i2 < filters; ++i2) {
                    if (i1 != i2) {
                        for (int k = 0; k < resolution * resolution; ++k) {
                            diff[i1 * resolution * resolution + k] -= 2.0 * lambda * (mu[i2 * resolution * resolution + k] - mu[i1 * resolution * resolution + k]) * f(i1, mu, &mu[i2 * resolution * resolution]);
                        }
                    }
                }
                for (int i = 0; i < resolution * resolution; ++i) {
                    mu[i1 * resolution * resolution + i] += diff[i1 * resolution * resolution + i] * 0.1 / 1.0;
                }
            }
            __kernel void INDICES(__constant double *rands, int rand_counter, __local int *batch_indices, __constant double *data, __global double *out) {
                int i = get_global_id(0);
                    batch_indices[i * 3 + 0] = (rands[rand_counter * batch_size * 3 + i * 3 + 0] * 60000.0);
                    batch_indices[i * 3 + 1] = (rands[rand_counter * batch_size * 3 + i * 3 + 1] * (28 - 4));
                    batch_indices[i * 3 + 2] = (rands[rand_counter * batch_size * 3 + i * 3 + 2] * (28 - 4));

                    int outer_from = batch_indices[i * 3 + 1] - 2;
                    int outer_to = batch_indices[i * 3 + 1] + 3;
                    int inner_from = batch_indices[i * 3 + 2] - 2;
                    int inner_to = batch_indices[i * 3 + 2] + 3;
                    for (int j = outer_from; j < outer_to; ++j) {
                        for (int k = inner_from; k < inner_to; ++k) {
                             out[i * (outer_to - outer_from) * (inner_to - inner_from) + (j - outer_from) * (inner_to - inner_from) + k - inner_from] =
                                     data[batch_indices[i * 3] * 28 * 28 + j * 28 + k ];

                        }
                    }
            }

    );
    // create sma program
    return compute::program::build_with_source(source,context, kernel_options);
}



template class Model<int>;
template class Model<double>;
