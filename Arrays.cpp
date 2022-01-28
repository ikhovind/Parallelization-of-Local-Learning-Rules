//
// Created by ingebrigt on 27.01.2022.
//

#include "Arrays.h"
#include <iostream>

CubeArray::CubeArray(bool zero, int outer, int middle, int inner) {
    for (int i = 0; i < outer; ++i) {
        w.emplace_back(std::vector<std::vector<double>>());
        for (int j = 0; j < middle; ++j) {
            w[i].emplace_back(std::vector<double>());
            for (int k = 0; k < inner; ++k) {
                if (zero) w[i][j].emplace_back(0);
                    // todo use c++ 11 instead
                else w[i][j].emplace_back(((double) rand() / RAND_MAX) * (1 - 0));
            }
        }
    }
}


double CubeArray::calc(SquareArray x, size_t outer) {
    double sum = 0;
    for (size_t inner = 0; inner < 5; inner++) {
        for (size_t index = 0; index < 5; index++) {
            double value = x[inner][index] - this->w[outer][inner][index];
            sum += std::pow(value, 2);
        }
    }
    return -sum;
}

SquareArray CubeArray::operator[](size_t i) const {
    return SquareArray(w[i]);
}

CubeArray operator*(double y,CubeArray x) {
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < x[i].size(); ++j) {
            for (int k = 0; k < x[i][j].size(); ++k) {
                x[i][j][k] *= y;
            }
        }
    }
    return x;
}


size_t CubeArray::size() {
    return w.size();
}

CubeArray CubeArray::operator/(double y) {
    auto x = *this;
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < x[i].size(); ++j) {
            for (int k = 0; k < x[i][j].size(); ++k) {
                x[i][j][k] /= y;
            }
        }
    }
    return x;
}

CubeArray CubeArray::operator+=(CubeArray y) {
    for (int i = 0; i < this->size(); ++i) {
        for (int j = 0; j < (*this)[i].size(); ++j) {
            for (int k = 0; k < (*this)[i][j].size(); ++k) {
                (*this)[i][j][k] += y[i][j][k];
            }
        }
    }
    return *this;
}

SquareArray::SquareArray(std::vector<std::vector<double>> x) {
    arr = x;
}

size_t SquareArray::size() {
    return arr.size();
}

std::vector<double> SquareArray::operator[](size_t i) {
    return arr[i];
}

SquareArray operator*(double x, SquareArray y) {
    for (int i = 0; i < y.size(); i++) {
        for (auto &val : y[i]) {
            val *= x;
        }
    }
    return y;
}

SquareArray SquareArray::operator-(std::vector<std::vector<double>> y) {
    for (int i = 0; i < this->size(); ++i) {
        for (int j = 0; j < this[i].size(); ++j) {
            (*this)[i][j] = (*this)[i][j] - y[i][j];
        }
    }
    return *this;
}

SquareArray operator+=(std::vector<std::vector<double>> x, SquareArray y) {
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < y.size(); ++j) {
            x[i][j] += y[i][j];
        }
    }
    return SquareArray(x);
}

SquareArray SquareArray::operator-(SquareArray y) {
    auto x = *this;
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < x[i].size(); ++j) {
            x[i][j] -= y[i][j];
        }
    }
    return x;
}

SquareArray SquareArray::operator*(double y) {
    auto x = *this;
    for (int i = 0; i < x.size(); ++i) {
        for (int j = 0; j < x[i].size(); ++j) {
            x[i][j] *= y;
        }
    }
    return x;
}

void SquareArray::flat(std::vector<float> &out) {
    for (size_t i = 0; i < this->size(); i++){
        for (size_t j = 0; j < this->size(); j++){
            out.at(this->size() * i + j) = this->arr[i][j];
        }
    }
    // return out;
}
