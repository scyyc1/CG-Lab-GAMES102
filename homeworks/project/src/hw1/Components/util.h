#pragma once

#include <eigen-3.4.0/Eigen/Eigen>
#include <_deps/imgui/imgui.h>

void plot(Eigen::VectorXf x, Eigen::VectorXf y, ImDrawList* draw_list, ImVec2 origin, int R, int G, int B) {
	ImVec2 last_point{ x(0) + origin.x, y(0) + origin.y };
	for (int i = 1; i < x.size(); i++) {
		draw_list->AddLine(last_point, ImVec2(x(i) + origin.x, y(i) + origin.y), IM_COL32(R, G, B, 255));
		last_point.x = x(i) + origin.x;
		last_point.y = y(i) + origin.y;
	}
}

std::vector<ImVec2> arrayToPoints(Eigen::VectorXf& x, Eigen::VectorXf& y) {
    std::vector<ImVec2> points;
    for (int i = 0; i < x.rows(); i++) {
        ImVec2 point;
        point.x = x(i);
        point.y = y(i);
        points.push_back(point);
    }
    return points;
}

Eigen::VectorXf pointsToArray(std::vector<ImVec2> points, int col) {
    int size = points.size();
    Eigen::VectorXf arr(size);
    for (int i = 0; i < size; i++) {
        arr(i) = points[i][col];
    }
    return arr;
}

float Lagrange_Interpolation(std::vector<ImVec2> points, float x) {
    float y = 0;

    for (int i = 0; i < points.size(); i++) {
        float up = 1;
        float down = 1;
        for (int j = 0; j < points.size(); j++) {
            if (i == j) continue;
            up *= (x - points[j].x);
            down *= (points[i].x - points[j].x);
        }
        y += points[i].y * up / down;
    }
    return y;
}