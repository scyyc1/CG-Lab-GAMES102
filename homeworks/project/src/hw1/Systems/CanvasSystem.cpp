#include "CanvasSystem.h"

#include "../Components/CanvasData.h"

#include <_deps/imgui/imgui.h>

// My implementation
#include "../Components/util.h"
#include "../Components/PolynomialInterpolation.h"
#include "../Components/GaussIntepolation.h"
#include "../Components/Regression.h"
#include <eigen-3.4.0/Eigen/Eigen>

using namespace Ubpa;

bool isClean = false;

void CanvasSystem::OnUpdate(Ubpa::UECS::Schedule& schedule) {
	schedule.RegisterCommand([](Ubpa::UECS::World* w) {
		auto data = w->entityMngr.GetSingleton<CanvasData>();
		if (!data)
			return;

		if (ImGui::Begin("Canvas")) {
			ImGui::Checkbox("Enable grid", &data->opt_enable_grid);
			ImGui::Checkbox("Enable context menu", &data->opt_enable_context_menu);
			ImGui::Text("Mouse Left: drag to add lines, Mouse Right: drag to scroll, click for context menu.");

			ImGui::Checkbox("Clean! ", &isClean);
			ImGui::Checkbox("Enable polynomial interpolation", &data->opt_enable_polynomial_interpolation);
			ImGui::Checkbox("Enable gaussian interpolation", &data->opt_enable_gaussian_interpolation);
			ImGui::SliderFloat("Sigma", &data->hyper_sigma, 1.f, 100.f);
			ImGui::SliderFloat("b0", &data->hyper_b0, -5.f, 5.f);
			ImGui::Checkbox("Enable linear regression", &data->opt_enable_linear_regression);
			ImGui::Checkbox("Enable ridge regression", &data->opt_enable_ridge_regression);
			ImGui::SliderFloat("lamda", &data->hyper_lamda, -1.f, 1.f);

			// Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region + own scrolling.
			// Here we demonstrate that this can be replaced by simple offsetting + custom drawing + PushClipRect/PopClipRect() calls.
			// To use a child window instead we could use, e.g:
			//      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
			//      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));  // Set a background color
			//      ImGui::BeginChild("canvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoMove);
			//      ImGui::PopStyleColor();
			//      ImGui::PopStyleVar();
			//      [...]
			//      ImGui::EndChild();

			// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates! - （0，0）
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available - 到能容纳的最大为止
			if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
			if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
			// p1是canvas的边界大小
			ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

			// Draw border and background color
			ImGuiIO& io = ImGui::GetIO();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();			// 画板buffer
			draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
			draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

			// This will catch our interactions
			ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			const bool is_hovered = ImGui::IsItemHovered(); // Hovered
			const bool is_active = ImGui::IsItemActive();   // Held
			const ImVec2 origin(canvas_p0.x + data->scrolling[0], canvas_p0.y + data->scrolling[1]); // Lock scrolled origin
			const pointf2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

			// ------------------------------------------!!!!!!!!!!!!!-----------------------------------
			if (data->adding_point && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
				ImVec2 mouse_pos_temp;
				mouse_pos_temp.x = mouse_pos_in_canvas[0];
				mouse_pos_temp.y = mouse_pos_in_canvas[1];
				// 记录鼠标点击的点的最大距离
				if (mouse_pos_temp.x < data->minX) data->minX = mouse_pos_temp.x;
				if (mouse_pos_temp.x > data->maxX) data->maxX = mouse_pos_temp.x;
				data->polyPoints.push_back(mouse_pos_temp);
				data->adding_point = false;
			}
			if(!ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				data->adding_point = true;
			// ------------------------------------------!!!!!!!!!!!!!-----------------------------------

			// Add first and second point
			// 初始点加两遍（两点一直线）
			if (is_hovered && !data->adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				data->points.push_back(mouse_pos_in_canvas);
				data->points.push_back(mouse_pos_in_canvas);
				data->adding_line = true;
			}
			// 持续替换第二个点为当前鼠标位置，当松开左键时，不再替换
			if (data->adding_line)
			{
				data->points.back() = mouse_pos_in_canvas;
				if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
					data->adding_line = false;
			}

			// Pan (we use a zero mouse threshold when there's no context menu)
			// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
			const float mouse_threshold_for_pan = data->opt_enable_context_menu ? -1.0f : 0.0f;
			if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, mouse_threshold_for_pan))
			{
				data->scrolling[0] += io.MouseDelta.x;
				data->scrolling[1] += io.MouseDelta.y;
			}

			// Context menu (under default mouse threshold)
			ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (data->opt_enable_context_menu && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && drag_delta.x == 0.0f && drag_delta.y == 0.0f)
				ImGui::OpenPopupContextItem("context");
			if (ImGui::BeginPopup("context"))
			{
				if (data->adding_line)
					data->points.resize(data->points.size() - 2);
				data->adding_line = false;
				if (ImGui::MenuItem("Remove one", NULL, false, data->points.size() > 0)) { data->points.resize(data->points.size() - 2); }
				if (ImGui::MenuItem("Remove all", NULL, false, data->points.size() > 0)) { data->points.clear(); }
				ImGui::EndPopup();
			}

			// Draw grid + all lines in the canvas
			draw_list->PushClipRect(canvas_p0, canvas_p1, true);
			// 画网格
			if (data->opt_enable_grid)
			{
				const float GRID_STEP = 64.0f;
				for (float x = fmodf(data->scrolling[0], GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
				for (float y = fmodf(data->scrolling[1], GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
			}
			// 每次更新都把points里面每两点画一条直线
			for (int n = 0; n < data->points.size(); n += 2)
				draw_list->AddLine(ImVec2(origin.x + data->points[n][0], origin.y + data->points[n][1]), 
					ImVec2(origin.x + data->points[n + 1][0], origin.y + data->points[n + 1][1]), 
					IM_COL32(255, 255, 0, 255), 2.0f);

			// ------------------------------------------!!!!!!!!!!!!!-----------------------------------
			if (isClean) {
				data->points.clear();
				data->polyPoints.clear();
				data->minX = 9999999999999999.f;
				data->maxX = -9999999999999999.f;
				isClean = false;
			}
			// 画点
			for (int n = 0; n < data->polyPoints.size(); n++) {
				draw_list->AddCircle(ImVec2(origin.x + data->polyPoints[n][0], origin.y + data->polyPoints[n][1]),
					3.f, IM_COL32(255, 0, 0, 255));
			}
			//// 离散+plot - 拉格朗日法
			//ImVec2 last_point{0.f, 0.f};
			//last_point.x = data->minX;
			//last_point.y = Lagrange_Interpolation(data->polyPoints, last_point.x);
			//for (float i = data->minX; i < data->maxX; i += 0.1f) {
			//	float y = Lagrange_Interpolation(data->polyPoints, i);
			//	draw_list->AddLine(last_point, ImVec2(i, y), IM_COL32(0, 255, 0, 255));
			//	last_point.x = i;
			//	last_point.y = y;
			//}

			
			Eigen::VectorXf x = pointsToArray(data->polyPoints, 0);
			Eigen::VectorXf y = pointsToArray(data->polyPoints, 1);

			// 多项式插值
			if (x.size() > 0 && data->opt_enable_polynomial_interpolation) {
				Eigen::VectorXf weight = polynomial_fitting(x, y);
				Eigen::VectorXf new_y = polynomial_interpolation(weight, data->minX, data->maxX, 100);
				Eigen::VectorXf new_x = Eigen::VectorXf::LinSpaced(100, data->minX, data->maxX);
				plot(new_x, new_y, draw_list, origin, 0, 255, 0);
			}

			// 高斯基函数
			Gaussian_Interpolation g(data->hyper_sigma, data->hyper_b0);
			if (x.size() > 1 && data->opt_enable_gaussian_interpolation) {
				Eigen::VectorXf weight = g.gaussian_fitting(x, y);
				Eigen::VectorXf new_y = g.gaussian_interpolation(weight, x, data->minX, data->maxX, 100);
				Eigen::VectorXf new_x = Eigen::VectorXf::LinSpaced(100, data->minX, data->maxX);
				plot(new_x, new_y, draw_list, origin, 255, 0, 0);
			}

			// 线性回归
			 Regression r(data->hyper_lamda);
			if (x.size() > 1 && data->opt_enable_linear_regression) {
				Eigen::VectorXf weight = r.Linear_Regression(x, y);
				Eigen::VectorXf new_y = r.Regression_plot(weight, x, data->minX, data->maxX, 100);
				Eigen::VectorXf new_x = Eigen::VectorXf::LinSpaced(100, data->minX, data->maxX);
				plot(new_x, new_y, draw_list, origin, 0, 255, 255);
			}

			// 岭回归
			if (x.size() > 1 && data->opt_enable_ridge_regression) {
				Eigen::VectorXf weight = r.Ridge_Regression(x, y);
				Eigen::VectorXf new_y = r.Regression_plot(weight, x, data->minX, data->maxX, 100);
				Eigen::VectorXf new_x = Eigen::VectorXf::LinSpaced(100, data->minX, data->maxX);
				plot(new_x, new_y, draw_list, origin, 255, 0, 255);
			}
			
			// ------------------------------------------!!!!!!!!!!!!!-----------------------------------
			draw_list->PopClipRect();
		}

		ImGui::End();
	});
}
