#ifndef FILTER_FINDER_MODEL_H
#define FILTER_FINDER_MODEL_H


#include <memory>
#include <string>
#include "Arrays.h"
#include <filesystem>

template <typename T>
class Model {
public:
    double sigma;
    double lambda;
    size_t filters;
    size_t resolution;
    double learning_rate;
    CubeArray<T> w;
    explicit Model(double sigma_, double lambda_, int grid_size_, int image_res_, double learning_rate_ = 0.1) : sigma(sigma_), lambda(lambda_), filters(grid_size_ * grid_size_), resolution(image_res_), learning_rate(learning_rate_), w(false, grid_size_ * grid_size_, image_res_, image_res_), diff(true, grid_size_ * grid_size_, image_res_, image_res_) {};
    void update(SquareArray<T> const &x);

    void save(const char &subfigure);
    bool load(const char &subfigure);

private:
    double f(int i, SquareArray<T> const &x);
    CubeArray<T> diff;
};


#endif //FILTER_FINDER_MODEL_H
