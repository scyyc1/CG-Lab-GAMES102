#pragma once

#include <UGM/UGM.h>
#include <_deps/imgui/imgui.h>

struct CanvasData {
	// --------------The origin------------------------
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };

	// ---------------My implementation----------------
	std::vector<ImVec2> polyPoints;
	bool adding_point{ true };
	int poly_point_count{ 0 };
	float hyper_sigma{ 50.f };
	float hyper_b0{ 0.f };
	float hyper_lamda{ 0.02f };
	float minX{ 9999999999999999.f };
	float maxX{ -9999999999999999.f };
	bool opt_enable_polynomial_interpolation{ true };
	bool opt_enable_gaussian_interpolation{ true };
	bool opt_enable_linear_regression{ true };
	bool opt_enable_ridge_regression{ true };
};

#include "details/CanvasData_AutoRefl.inl"
