/**************************************************************************/
/*  custom_cell_data.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                     INTERACTIVE GRID GDExtension                       */
/*         https://github.com/antoinecharruel/interactive_grid            */
/**************************************************************************/
/* Copyright (c) 2025 Antoine Charruel.                                   */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "custom_cell_data.h"

void CustomCellData::set_custom_data_name(godot::String p_name) {
	custom_data_name = p_name;
}

godot::String CustomCellData::get_custom_data_name() const {
	return custom_data_name;
}

void CustomCellData::set_layer_mask(const uint32_t p_layer_mask) {
	layer_mask = p_layer_mask;
}

uint32_t CustomCellData::get_layer_mask() const {
	return layer_mask;
}

void CustomCellData::set_collision_layer(uint32_t p_layer) {
	collision_layer = p_layer;
}

uint32_t CustomCellData::get_collision_layer() const {
	return collision_layer;
}

void CustomCellData::set_color(const godot::Color &p_color) {
	color = p_color;
}

godot::Color CustomCellData::get_color() const {
	return color;
}

void CustomCellData::set_custom_color_enabled(bool p_enabled) {
	custom_color_enabled = p_enabled;
}

bool CustomCellData::get_custom_color_enabled() const {
	return custom_color_enabled;
}

bool CustomCellData::has_layers_in_mask(const uint32_t p_layer_mask) const {
	return (collision_layer & p_layer_mask) == collision_layer;
}

bool CustomCellData::get_collision_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Collision layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return get_collision_layer() & (1 << (p_layer_number - 1));
}

void CustomCellData::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_data_name", "name"), &CustomCellData::set_custom_data_name);
	godot::ClassDB::bind_method(godot::D_METHOD("get_custom_data_name"), &CustomCellData::get_custom_data_name);

	godot::ClassDB::bind_method(godot::D_METHOD("set_layer_mask", "layer_mask"), &CustomCellData::set_layer_mask);
	godot::ClassDB::bind_method(godot::D_METHOD("get_layer_mask"), &CustomCellData::get_layer_mask);

	godot::ClassDB::bind_method(godot::D_METHOD("set_collision_layer", "layer_mask"), &CustomCellData::set_collision_layer);
	godot::ClassDB::bind_method(godot::D_METHOD("get_collision_layer"), &CustomCellData::get_collision_layer);

	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_color_enabled", "use_custom_color"), &CustomCellData::set_custom_color_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("get_custom_color_enabled"), &CustomCellData::get_custom_color_enabled);

	godot::ClassDB::bind_method(godot::D_METHOD("set_color", "color"), &CustomCellData::set_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_color"), &CustomCellData::get_color);

	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "custom_data_name"), "set_custom_data_name", "get_custom_data_name");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "layer_mask", godot::PROPERTY_HINT_LAYERS_3D_RENDER), "set_layer_mask", "get_layer_mask");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "collision_layer", godot::PROPERTY_HINT_LAYERS_3D_RENDER), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "use_custom_color"), "set_custom_color_enabled", "get_custom_color_enabled");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "color"), "set_color", "get_color");
}

CustomCellData::CustomCellData() {}

CustomCellData::~CustomCellData() {}