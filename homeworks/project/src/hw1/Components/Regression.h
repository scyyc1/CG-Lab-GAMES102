#pragma once

#include <eigen-3.4.0/Eigen/Eigen>



class Regression {
public:
    float lamda = 0.05f;

    Regression(float l) {
        this->lamda = l;
    }

    Eigen::VectorXf Linear_Regression(Eigen::VectorXf& x, Eigen::VectorXf& y) {
        int n = x.rows();
        // 使得m<n(分类讨论，最大用最高5次幂的曲线拟合)
        int m = (n > 3) ? ((n > 5) ? 5 : n - 1) : n;

        // 条件：m < n，Ax=b不可能有解
        Eigen::MatrixXf A = Eigen::MatrixXf::Zero(n, m);
        Eigen::VectorXf x_power = Eigen::VectorXf::Ones(n);
        // 构造a
        for (int i = 0; i < m; i++) {
            A.col(i) = x_power;
            x_power = x_power.array() * x.array();
        }
        //Eigen::VectorXf weight = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(y);
        Eigen::VectorXf weight = (A.transpose() * A).inverse() * (A.transpose() * y);
        return weight;
    }

    Eigen::VectorXf Ridge_Regression(Eigen::VectorXf& x, Eigen::VectorXf& y) {
        int n = x.rows();
        int m = (n > 3) ? ((n > 5) ? 5 : n - 1) : n;

        Eigen::MatrixXf A = Eigen::MatrixXf::Zero(n, m);
        Eigen::VectorXf x_power = Eigen::VectorXf::Ones(n);
        for (int i = 0; i < m; i++) {
            A.col(i) = x_power;
            x_power = x_power.array() * x.array();
        }
        Eigen::MatrixXf Regularizer = lamda * Eigen::MatrixXf::Identity(m, m);
        Eigen::VectorXf weight = ((A.transpose() * A) - Regularizer).inverse() * (A.transpose() * y);
        return weight;
    }

    Eigen::VectorXf Regression_plot(Eigen::VectorXf weight, Eigen::VectorXf x, float minX, float maxX, int dense) {
        Eigen::VectorXf x_discretized = Eigen::VectorXf::LinSpaced(dense, minX, maxX);          // x离散化
        Eigen::VectorXf x_power = Eigen::VectorXf::Ones(x_discretized.rows());                  // x的幂次
        Eigen::MatrixXf x_matrix = Eigen::MatrixXf::Zero(x_discretized.rows(), weight.rows());  // x的矩阵

        for (int i = 0; i < weight.rows(); i++) {
            x_matrix.col(i) += x_power;
            x_power = x_discretized.array() * x_power.array();
        }
        Eigen::VectorXf y = x_matrix * weight;
        return y;
    }
};

