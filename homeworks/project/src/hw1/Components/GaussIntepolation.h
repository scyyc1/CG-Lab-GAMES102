#pragma once

#include <_deps/imgui/imgui.h>
#include <eigen-3.4.0/Eigen/Eigen>



class Gaussian_Interpolation {
public:
    float sigma = 20.f;
    float b0 = 0.f;

    Gaussian_Interpolation(float s, float b) {
        this->sigma = s;
        this->b0 = b;
    }

    Eigen::VectorXf gaussian_fitting(Eigen::VectorXf& x, Eigen::VectorXf& y) {
        int size = x.rows();

        Eigen::MatrixXf A = Eigen::MatrixXf::Zero(size, size);
        A.col(0) = Eigen::VectorXf::Ones(size);
        // 设b0为超参数，约束b0的值
        // 则Aw=y-b0
        // 构造新y
        Eigen::VectorXf ny = y - Eigen::VectorXf::Ones(size) * b0;
        // 构造A
        for (int i = 0; i < size; i++) {
            Eigen::VectorXf dX = x - Eigen::VectorXf::Ones(size) * x(i);
            Eigen::VectorXf temp = (-dX.array() * dX.array() / (2 * sigma * sigma));
            A.col(i) = temp.array().exp();
        }
        Eigen::VectorXf weight = A.fullPivLu().solve(y);
        return weight;
    }

    Eigen::VectorXf gaussian_interpolation(Eigen::VectorXf weight, Eigen::VectorXf x, float minX, float maxX, int dense) {
        Eigen::VectorXf x_discretized = Eigen::VectorXf::LinSpaced(dense, minX, maxX);          // x离散化
        Eigen::MatrixXf x_matrix = Eigen::MatrixXf::Zero(x_discretized.rows(), weight.rows());  // x的矩阵
        // 构造x的矩阵
        for (int i = 0; i < weight.rows(); i++) {
            Eigen::VectorXf dX = x_discretized - Eigen::VectorXf::Ones(x_discretized.size()) * x(i);
            Eigen::VectorXf temp = (-dX.array() * dX.array() / (2 * sigma * sigma));
            x_matrix.col(i) = temp.array().exp();
        }
        Eigen::VectorXf y = x_matrix * weight + Eigen::VectorXf::Ones(x_discretized.size()) * b0;                // 求y
        return y;
    }
};

