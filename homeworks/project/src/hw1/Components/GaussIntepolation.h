#pragma once

#include <_deps/imgui/imgui.h>
#include <eigen-3.4.0/Eigen/Eigen>

float sigma = 20.f;
// ��ú�����ԭ�㣨0.f, 0.f��������b0Ϊ������
float b0 = 0.f;

Eigen::VectorXf gaussian_fitting(Eigen::VectorXf& x, Eigen::VectorXf& y) {
    int size = x.rows();

    Eigen::MatrixXf A = Eigen::MatrixXf::Zero(size, size);
    A.col(0) = Eigen::VectorXf::Ones(size);
    // ��b0Ϊ��������Լ��b0��ֵ
    // ��Aw=y-b0
    // ������y
    Eigen::VectorXf ny = y - Eigen::VectorXf::Ones(size) * b0;
    // ����A
    for (int i = 0; i < size; i++) {
        Eigen::VectorXf dX = x - Eigen::VectorXf::Ones(size) * x(i);
        Eigen::VectorXf temp = (-dX.array() * dX.array() / (2 * sigma * sigma));
        A.col(i) = temp.array().exp();
    }
    Eigen::VectorXf weight = A.fullPivLu().solve(y);
    return weight;
}

Eigen::VectorXf gaussian_interpolation(Eigen::VectorXf weight, Eigen::VectorXf x, float minX, float maxX, int dense) {
    Eigen::VectorXf x_discretized = Eigen::VectorXf::LinSpaced(dense, minX, maxX);          // x��ɢ��
    Eigen::MatrixXf x_matrix = Eigen::MatrixXf::Zero(x_discretized.rows(), weight.rows());  // x�ľ���
    // ����x�ľ���
    for (int i = 0; i < weight.rows(); i++) {
        Eigen::VectorXf dX = x_discretized - Eigen::VectorXf::Ones(x_discretized.size()) * x(i);
        Eigen::VectorXf temp = (-dX.array() * dX.array() / (2 * sigma * sigma));
        x_matrix.col(i) = temp.array().exp();
    }
    Eigen::VectorXf y = x_matrix * weight + Eigen::VectorXf::Ones(x_discretized.size()) * b0;                // ��y
    return y;
}