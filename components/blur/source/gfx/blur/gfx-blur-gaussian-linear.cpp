// AUTOGENERATED COPYRIGHT HEADER START
// Copyright (C) 2019-2023 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
// Copyright (C) 2022 lainon <GermanAizek@yandex.ru>
// AUTOGENERATED COPYRIGHT HEADER END

#include "gfx-blur-gaussian-linear.hpp"
#include "common.hpp"
#include "obs/gs/gs-helper.hpp"

#include "warning-disable.hpp"
#include <stdexcept>
#include "warning-enable.hpp"

// FIXME: This breaks when MAX_KERNEL_SIZE is changed, due to the way the Gaussian
//  function first goes up at the point, and then once we pass the critical point
//  will go down again and it is not handled well. This is a pretty basic
//  approximation anyway at the moment.
#define ST_MAX_KERNEL_SIZE 128
#define ST_MAX_BLUR_SIZE (ST_MAX_KERNEL_SIZE - 1)
#define ST_SEARCH_DENSITY double_t(1. / 500.)
#define ST_SEARCH_THRESHOLD double_t(1. / (ST_MAX_KERNEL_SIZE * 5))
#define ST_SEARCH_EXTENSION 1
#define ST_SEARCH_RANGE ST_MAX_KERNEL_SIZE * 2

streamfx::gfx::blur::gaussian_linear_data::gaussian_linear_data() : _gfx_util(::streamfx::gfx::util::get())
{
	{
		auto gctx = streamfx::obs::gs::context();

		{
			auto file = streamfx::data_file_path("effects/blur/gaussian-linear.effect");
			try {
				_effect = streamfx::obs::gs::effect::create(file);
			} catch (const std::exception& ex) {
				DLOG_ERROR("Error loading '%s': %s", file.generic_u8string().c_str(), ex.what());
			}
		}
	}

	// Precalculate Kernels
	for (std::size_t kernel_size = 1; kernel_size <= ST_MAX_BLUR_SIZE; kernel_size++) {
		std::vector<double_t> kernel_math(ST_MAX_KERNEL_SIZE);
		std::vector<float>  kernel_data(ST_MAX_KERNEL_SIZE);
		double_t              actual_width = 1.;

		// Find actual kernel width.
		for (double_t h = ST_SEARCH_DENSITY; h < ST_SEARCH_RANGE; h += ST_SEARCH_DENSITY) {
			if (streamfx::util::math::gaussian<double_t>(double_t(kernel_size + ST_SEARCH_EXTENSION), h) > ST_SEARCH_THRESHOLD) {
				actual_width = h;
				break;
			}
		}

		// Calculate and normalize
		double_t sum = 0;
		for (std::size_t p = 0; p <= kernel_size; p++) {
			kernel_math[p] = streamfx::util::math::gaussian<double_t>(double_t(p), actual_width);
			sum += kernel_math[p] * (p > 0 ? 2 : 1);
		}

		// Normalize to fill the entire 0..1 range over the width.
		double_t inverse_sum = 1.0 / sum;
		for (std::size_t p = 0; p <= kernel_size; p++) {
			kernel_data.at(p) = float(kernel_math[p] * inverse_sum);
		}

		_kernels.push_back(std::move(kernel_data));
	}
}

streamfx::gfx::blur::gaussian_linear_data::~gaussian_linear_data()
{
	_effect.reset();
}

streamfx::obs::gs::effect streamfx::gfx::blur::gaussian_linear_data::get_effect()
{
	return _effect;
}

std::vector<float> const& streamfx::gfx::blur::gaussian_linear_data::get_kernel(std::size_t width)
{
	if (width < 1)
		width = 1;
	if (width > ST_MAX_BLUR_SIZE)
		width = ST_MAX_BLUR_SIZE;
	width -= 1;
	return _kernels[width];
}

std::shared_ptr<streamfx::gfx::util> streamfx::gfx::blur::gaussian_linear_data::get_gfx_util()
{
	return _gfx_util;
}

streamfx::gfx::blur::gaussian_linear_factory::gaussian_linear_factory() {}

streamfx::gfx::blur::gaussian_linear_factory::~gaussian_linear_factory() {}

bool streamfx::gfx::blur::gaussian_linear_factory::is_type_supported(::streamfx::gfx::blur::type v)
{
	switch (v) {
	case ::streamfx::gfx::blur::type::Area:
	case ::streamfx::gfx::blur::type::Directional:
		return true;
	default:
		return false;
	}
}

std::shared_ptr<::streamfx::gfx::blur::base> streamfx::gfx::blur::gaussian_linear_factory::create(::streamfx::gfx::blur::type v)
{
	switch (v) {
	case ::streamfx::gfx::blur::type::Area:
		return std::make_shared<::streamfx::gfx::blur::gaussian_linear>();
	case ::streamfx::gfx::blur::type::Directional:
		return std::static_pointer_cast<::streamfx::gfx::blur::gaussian_linear>(std::make_shared<::streamfx::gfx::blur::gaussian_linear_directional>());
	default:
		throw std::runtime_error("Invalid type.");
	}
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_min_size(::streamfx::gfx::blur::type)
{
	return double_t(1.0);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_step_size(::streamfx::gfx::blur::type)
{
	return double_t(1.0);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_max_size(::streamfx::gfx::blur::type)
{
	return double_t(ST_MAX_BLUR_SIZE);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_min_angle(::streamfx::gfx::blur::type v)
{
	switch (v) {
	case ::streamfx::gfx::blur::type::Directional:
	case ::streamfx::gfx::blur::type::Rotational:
		return -180.0;
	default:
		return 0;
	}
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_step_angle(::streamfx::gfx::blur::type)
{
	return double_t(0.01);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_max_angle(::streamfx::gfx::blur::type v)
{
	switch (v) {
	case ::streamfx::gfx::blur::type::Directional:
	case ::streamfx::gfx::blur::type::Rotational:
		return 180.0;
	default:
		return 0;
	}
}

bool streamfx::gfx::blur::gaussian_linear_factory::is_step_scale_supported(::streamfx::gfx::blur::type v)
{
	switch (v) {
	case ::streamfx::gfx::blur::type::Area:
	case ::streamfx::gfx::blur::type::Zoom:
	case ::streamfx::gfx::blur::type::Directional:
		return true;
	default:
		return false;
	}
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_min_step_scale_x(::streamfx::gfx::blur::type)
{
	return double_t(0.01);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_step_step_scale_x(::streamfx::gfx::blur::type)
{
	return double_t(0.01);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_max_step_scale_x(::streamfx::gfx::blur::type)
{
	return double_t(1000.0);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_min_step_scale_y(::streamfx::gfx::blur::type)
{
	return double_t(0.01);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_step_step_scale_y(::streamfx::gfx::blur::type)
{
	return double_t(0.01);
}

double_t streamfx::gfx::blur::gaussian_linear_factory::get_max_step_scale_y(::streamfx::gfx::blur::type)
{
	return double_t(1000.0);
}

std::shared_ptr<::streamfx::gfx::blur::gaussian_linear_data> streamfx::gfx::blur::gaussian_linear_factory::data()
{
	std::unique_lock<std::mutex>                                 ulock(_data_lock);
	std::shared_ptr<::streamfx::gfx::blur::gaussian_linear_data> data = _data.lock();
	if (!data) {
		data  = std::make_shared<::streamfx::gfx::blur::gaussian_linear_data>();
		_data = data;
	}
	return data;
}

::streamfx::gfx::blur::gaussian_linear_factory& streamfx::gfx::blur::gaussian_linear_factory::get()
{
	static ::streamfx::gfx::blur::gaussian_linear_factory instance;
	return instance;
}

streamfx::gfx::blur::gaussian_linear::gaussian_linear() : _data(::streamfx::gfx::blur::gaussian_linear_factory::get().data()), _size(1.), _step_scale({1., 1.})
{
	auto gctx = streamfx::obs::gs::context();

	_rendertarget  = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
	_rendertarget2 = std::make_shared<streamfx::obs::gs::rendertarget>(GS_RGBA, GS_ZS_NONE);
}

streamfx::gfx::blur::gaussian_linear::~gaussian_linear() {}

void streamfx::gfx::blur::gaussian_linear::set_input(std::shared_ptr<::streamfx::obs::gs::texture> texture)
{
	_input_texture = std::move(texture);
}

::streamfx::gfx::blur::type streamfx::gfx::blur::gaussian_linear::get_type()
{
	return ::streamfx::gfx::blur::type::Area;
}

double_t streamfx::gfx::blur::gaussian_linear::get_size()
{
	return _size;
}

void streamfx::gfx::blur::gaussian_linear::set_size(double_t width)
{
	if (width < 1.)
		width = 1.;
	if (width > ST_MAX_BLUR_SIZE)
		width = ST_MAX_BLUR_SIZE;
	_size = width;
}

void streamfx::gfx::blur::gaussian_linear::set_step_scale(double_t x, double_t y)
{
	_step_scale.first  = x;
	_step_scale.second = y;
}

void streamfx::gfx::blur::gaussian_linear::get_step_scale(double_t& x, double_t& y)
{
	x = _step_scale.first;
	y = _step_scale.second;
}

double_t streamfx::gfx::blur::gaussian_linear::get_step_scale_x()
{
	return _step_scale.first;
}

double_t streamfx::gfx::blur::gaussian_linear::get_step_scale_y()
{
	return _step_scale.second;
}

std::shared_ptr<::streamfx::obs::gs::texture> streamfx::gfx::blur::gaussian_linear::render()
{
	auto gctx = streamfx::obs::gs::context();

#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
	auto gdmp = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance, "Gaussian Linear Blur");
#endif

	streamfx::obs::gs::effect effect = _data->get_effect();
	auto                      kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float width  = float(_input_texture->get_width());
	float height = float(_input_texture->get_height());

	// Setup
	gs_set_cull_mode(GS_NEITHER);
	gs_enable_color(true, true, true, true);
	gs_enable_depth_test(false);
	gs_depth_function(GS_ALWAYS);
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_blending(false);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pStepScale").set_float2(float(_step_scale.first), float(_step_scale.second));
	effect.get_parameter("pSize").set_float(float(_size));
	effect.get_parameter("pKernel").set_value(kernel.data(), ST_MAX_KERNEL_SIZE);

	// First Pass
	if (_step_scale.first > std::numeric_limits<double_t>::epsilon()) {
		effect.get_parameter("pImageTexel").set_float2(float(1.f / width), 0.f);

		{
#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
			auto gdm = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance, "Horizontal");
#endif

			auto op = _rendertarget2->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				_data->get_gfx_util()->draw_fullscreen_triangle();
			}
		}

		std::swap(_rendertarget, _rendertarget2);
		effect.get_parameter("pImage").set_texture(_rendertarget->get_texture());
	}

	// Second Pass
	if (_step_scale.second > std::numeric_limits<double_t>::epsilon()) {
		effect.get_parameter("pImageTexel").set_float2(0.f, float(1.f / height));

		{
#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
			auto gdm = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance, "Vertical");
#endif

			auto op = _rendertarget2->render(uint32_t(width), uint32_t(height));
			gs_ortho(0, 1., 0, 1., 0, 1.);
			while (gs_effect_loop(effect.get_object(), "Draw")) {
				_data->get_gfx_util()->draw_fullscreen_triangle();
			}
		}

		std::swap(_rendertarget, _rendertarget2);
	}

	gs_blend_state_pop();

	return this->get();
}

std::shared_ptr<::streamfx::obs::gs::texture> streamfx::gfx::blur::gaussian_linear::get()
{
	return _rendertarget->get_texture();
}

streamfx::gfx::blur::gaussian_linear_directional::gaussian_linear_directional() : _angle(0.) {}

streamfx::gfx::blur::gaussian_linear_directional::~gaussian_linear_directional() {}

::streamfx::gfx::blur::type streamfx::gfx::blur::gaussian_linear_directional::get_type()
{
	return ::streamfx::gfx::blur::type::Directional;
}

double_t streamfx::gfx::blur::gaussian_linear_directional::get_angle()
{
	return D_RAD_TO_DEG(_angle);
}

void streamfx::gfx::blur::gaussian_linear_directional::set_angle(double_t angle)
{
	_angle = D_DEG_TO_RAD(angle);
}

std::shared_ptr<::streamfx::obs::gs::texture> streamfx::gfx::blur::gaussian_linear_directional::render()
{
	auto gctx = streamfx::obs::gs::context();

#if defined(ENABLE_PROFILING) && !defined(D_PLATFORM_MAC) && _DEBUG
	auto gdmp = streamfx::obs::gs::debug_marker(streamfx::obs::gs::debug_color_azure_radiance, "Gaussian Linear Directional Blur");
#endif

	streamfx::obs::gs::effect effect = _data->get_effect();
	auto                      kernel = _data->get_kernel(size_t(_size));

	if (!effect || ((_step_scale.first + _step_scale.second) < std::numeric_limits<double_t>::epsilon())) {
		return _input_texture;
	}

	float width  = float(_input_texture->get_width());
	float height = float(_input_texture->get_height());

	// Setup
	gs_set_cull_mode(GS_NEITHER);
	gs_enable_color(true, true, true, true);
	gs_enable_depth_test(false);
	gs_depth_function(GS_ALWAYS);
	gs_blend_state_push();
	gs_reset_blend_state();
	gs_enable_blending(false);
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	gs_enable_stencil_test(false);
	gs_enable_stencil_write(false);
	gs_stencil_function(GS_STENCIL_BOTH, GS_ALWAYS);
	gs_stencil_op(GS_STENCIL_BOTH, GS_ZERO, GS_ZERO, GS_ZERO);

	effect.get_parameter("pImage").set_texture(_input_texture);
	effect.get_parameter("pImageTexel").set_float2(float(1.f / width * cos(_angle)), float(1.f / height * sin(_angle)));
	effect.get_parameter("pStepScale").set_float2(float(_step_scale.first), float(_step_scale.second));
	effect.get_parameter("pSize").set_float(float(_size));
	effect.get_parameter("pKernel").set_value(kernel.data(), ST_MAX_KERNEL_SIZE);

	// First Pass
	{
		auto op = _rendertarget->render(uint32_t(width), uint32_t(height));
		gs_ortho(0, 1., 0, 1., 0, 1.);
		while (gs_effect_loop(effect.get_object(), "Draw")) {
			_data->get_gfx_util()->draw_fullscreen_triangle();
		}
	}

	gs_blend_state_pop();

	return this->get();
}
