/**************************************************************************/
/*  custom_cell_data.h                                                    */
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

#pragma once

#include "common.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

class CustomCellData : public godot::Resource {
private:
	GDCLASS(CustomCellData, Resource);

	godot::String custom_data_name = "";
	uint32_t layer_mask = 1;
	uint32_t collision_layer = 1;
	bool custom_color_enabled = false;
	godot::Color color = godot::Color(1, 1, 1);

protected:
	static void _bind_methods();

public:
	CustomCellData();
	~CustomCellData();

	void set_custom_data_name(godot::String p_name);
	godot::String get_custom_data_name() const;

	void set_layer_mask(const uint32_t p_layer_mask);
	uint32_t get_layer_mask() const;

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_color(const godot::Color &p_color);
	godot::Color get_color() const;

	void set_custom_color_enabled(bool p_enabled);
	bool get_custom_color_enabled() const;

	bool has_layers_in_mask(const uint32_t p_layer_mask) const;

	bool get_collision_layer_value(int p_layer_number) const;
};