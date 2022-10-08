#pragma once

#include <_deps/imgui/imgui.h>
#include <eigen-3.4.0/Eigen/Eigen>

Eigen::VectorXf polynomial_fitting(Eigen::VectorXf& x, Eigen::VectorXf& y) {
    int size = x.rows();

    Eigen::MatrixXf A = Eigen::MatrixXf::Zero(size, size);
    Eigen::VectorXf x_power = Eigen::VectorXf::Ones(size);
    // 构造A
    for (int i = 0; i < size; i++) {
        A.col(i) = x_power;
        x_power = x_power.array() * x.array();
    }
    Eigen::VectorXf weight = A.partialPivLu().solve(y);
    return weight;
}

Eigen::VectorXf polynomial_interpolation(Eigen::VectorXf weight, float minX, float maxX, int dense) {
    Eigen::VectorXf x_discretized = Eigen::VectorXf::LinSpaced(dense, minX, maxX);          // x离散化
    Eigen::VectorXf x_power = Eigen::VectorXf::Ones(x_discretized.rows());                  // x的幂次
    Eigen::MatrixXf x_matrix = Eigen::MatrixXf::Zero(x_discretized.rows(), weight.rows());  // x的矩阵
    // 构造x的矩阵
    for (int i = 0; i < weight.rows(); i++) {
        x_matrix.col(i) += x_power;
        x_power = x_discretized.array() * x_power.array();
    }
    Eigen::VectorXf y = x_matrix * weight;                                                  // 求y
    return y;
}