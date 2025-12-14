/**************************************************************************/
/*  interactive_grid.h                                                    */
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
#include "custom_cell_data.h"

#include <godot_cpp/classes/a_star2d.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/convex_polygon_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <algorithm>
#include <chrono>
#include <queue>

class InteractiveGrid3D : public godot::Node3D {
	GDCLASS(InteractiveGrid3D, Node3D);

public:
	enum Layout {
		LAYOUT_SQUARE,
		LAYOUT_HEXAGONAL
	};

	enum Movement {
		MOVEMENT_FOUR_DIRECTIONS,
		MOVEMENT_SIX_DIRECTIONS,
		MOVEMENT_EIGH_DIRECTIONS
	};

	struct DebugOptions {
		bool print_logs_enabled = false;
		bool print_execution_time_enabled = false;
	} _debug_options;

private:
	struct Cell {
		int index = -1;
		godot::Transform3D local_xform;
		godot::Transform3D global_xform;
		uint32_t flags = 0;
		uint32_t collision_layer = 0;
		godot::Color color;
		uint32_t custom_flags = 0;
		uint32_t custom_collision_layer = 0;
		godot::Color custom_color;
		bool has_custom_color = false;
		godot::Array neighbors;
	};

	struct Data {
		unsigned int rows{ 9 };
		unsigned int columns{ 9 };
		uint32_t flags = 0;

		godot::Vector3 center_global_position = godot::Vector3(0.0f, 0.0f, 0.0f);

		godot::Ref<godot::AStar2D> astar;

		Layout layout_index = LAYOUT_SQUARE;
		Movement movement = MOVEMENT_FOUR_DIRECTIONS;
		uint32_t obstacles_collision_masks = 1 << 13;
		uint32_t floor_collision_mask = 1 << 14;

		godot::Ref<godot::Mesh> cell_mesh;
		godot::Ref<godot::Shape3D> cell_shape;
		godot::Vector3 cell_shape_offset = godot::Vector3(0.0f, 0.0f, 0.0f);
		godot::MultiMeshInstance3D *multimesh_instance;
		godot::Ref<godot::MultiMesh> multimesh;
		godot::Vector2 cell_size = godot::Vector2(1.0f, 1.0f);
		godot::Vector<Cell *> cells;

		godot::Array custom_cell_data;

		godot::Array selected_cells;
		int hovered_cell_index = -1;

		godot::Color walkable_color{ godot::Color(0.5, 0.65, 1.0, 1) }; // BLUE
		godot::Color unwalkable_color{ godot::Color(0.8039216, 0.36078432, 0.36078432, 1.0) }; // INDIAN_RED
		godot::Color unreachable_color{ godot::Color(1.0, 1.0, 1.0, 1.0) }; // #ffffff00
		godot::Color selected_color{ godot::Color(0.8784314, 1.0, 1.0, 1.0) }; // LIGHT_CYAN
		godot::Color path_color{ godot::Color(0.5647059, 0.93333334, 0.5647059, 1) };
		godot::Color hovered_color{ godot::Color(1.0, 0.84313726, 0, 1.0) };

		godot::Ref<godot::Material> material_override;

	} data;

	static constexpr int GFL_CREATED = 1 << 0;
	static constexpr int GFL_CENTERED = 1 << 1;
	static constexpr int GFL_CELL_UNREACHABLE_HIDDEN = 1 << 2;
	static constexpr int GFL_CELL_DISTANT_HIDDEN = 1 << 3;
	static constexpr int GFL_HOVER_ENABLED = 1 << 4;

	static constexpr int CFL_WALKABLE = 1 << 0;
	static constexpr int CFL_REACHABLE = 1 << 1;
	static constexpr int CFL_IN_VOID = 1 << 2;
	static constexpr int CFL_HOVERED = 1 << 3;
	static constexpr int CFL_SELECTED = 1 << 4;
	static constexpr int CFL_PATH = 1 << 5;
	static constexpr int CFL_VISIBLE = 1 << 6;

	void _create();
	void _delete();

	void _init_multi_mesh();
	void _init_astar();

	void _align_cells_with_floor();
	void _scan_environnement_obstacles();
	void _scan_environnement_custom_data();

	void _layout(godot::Vector3 p_center_position);
	void _layout_cells_as_square_grid(godot::Vector3 p_center_position);
	void _layout_cells_as_hexagonal_grid(godot::Vector3 p_center_position);

	void _configure_astar();
	void _configure_astar_4_dir();
	void _configure_astar_6_dir();
	void _configure_astar_8_dir();
	void _breadth_first_search(unsigned int p_start_cell_index);

	void _apply_material(const godot::Ref<godot::Material> &p_material);

	void _set_cell_in_void(int p_cell_index, bool p_is_in_void);
	void _set_cell_hovered(int p_cell_index, bool p_is_hovered);
	void _set_cell_selected(int p_cell_index, bool p_is_selected);
	void _set_cell_on_path(int p_cell_index, bool p_is_on_path);

protected:
	static void _bind_methods();

public:
	virtual void _ready() override;
	virtual void _physics_process(double p_delta) override;

	void set_rows(int p_rows);
	int get_rows() const;

	void set_columns(int p_columns);
	int get_columns() const;

	void set_cell_size(godot::Vector2 p_cell_size);
	godot::Vector2 get_cell_size() const;

	int get_size() const;

	void set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh);
	godot::Ref<godot::Mesh> get_cell_mesh() const;

	void set_cell_shape(const godot::Ref<godot::Shape3D> &p_shape);
	godot::Ref<godot::Shape3D> get_cell_shape() const;

	void set_cell_shape_offset(godot::Vector3 p_offset);
	godot::Vector3 get_cell_shape_offset();

	void set_layout(Layout p_layout);
	Layout get_layout() const;

	void set_movement(Movement p_movement);
	Movement get_movement() const;

	void set_walkable_color(const godot::Color &p_color);
	godot::Color get_walkable_color() const;

	void set_unwalkable_color(const godot::Color &p_color);
	godot::Color get_unwalkable_color() const;

	void set_unreachable_color(const godot::Color &p_color);
	godot::Color get_unreachable_color() const;

	void set_selected_color(const godot::Color &p_color);
	godot::Color get_selected_color() const;

	void set_path_color(const godot::Color &p_color);
	godot::Color get_path_color() const;

	void set_hovered_color(const godot::Color &p_color);
	godot::Color get_hovered_color() const;

	void set_custom_cells_data(const godot::Array &p_custom_cell_data);
	godot::Array get_custom_cells_data() const;

	void add_custom_cell_data(int p_cell_index, godot::String p_custom_data_name);
	bool has_custom_cell_data(int p_cell_index, godot::String p_custom_data_name);
	void clear_custom_cell_data(int p_cell_index, godot::String p_custom_data_name, bool p_clear_custom_color);
	void clear_all_custom_cell_data(int p_cell_index);

	void set_material_override(const godot::Ref<godot::Material> &p_material);
	godot::Ref<godot::Material> get_material_override() const;
	void apply_default_material();

	void highlight_on_hover(godot::Vector3 p_global_position);
	void highlight_path(const godot::PackedInt64Array &p_path);

	godot::Vector3 get_cell_global_position(int p_cell_index) const;
	int get_cell_index_from_global_position(godot::Vector3 p_global_position) const;
	godot::Vector3 get_grid_center_global_position() const;
	void center(godot::Vector3 p_center_position);
	void update_custom_data();

	void compute_unreachable_cells(int p_start_cell_index);

	void hide_distant_cells(int p_start_cell_index, float p_distance);
	void set_hover_enabled(bool p_enabled);
	bool is_hover_enabled() const;

	bool is_created() const;
	bool is_centered() const;

	bool is_cell_walkable(int p_cell_index) const;
	bool is_cell_reachable(int p_cell_index) const;
	bool is_cell_in_void(int p_cell_index) const;
	bool is_cell_hovered(int p_cell_index) const;
	bool is_cell_selected(int p_cell_index) const;
	bool is_cell_on_path(int p_cell_index) const;
	bool is_cell_visible(int p_cell_index) const;

	void set_cell_walkable(int p_cell_index, const bool p_is_walkable);
	void set_cell_reachable(int p_cell_index, bool p_is_unreachable);
	void set_cell_visible(int p_cell_index, const bool p_is_visible);

	void reset_cells_state();

	void set_cell_color(int cell_index, const godot::Color &p_color);

	void set_obstacles_collision_masks(int p_mask);
	int get_obstacles_collision_masks();

	void set_grid_floor_collision_mask(int p_mask);
	int get_grid_floor_collision_mask();

	void select_cell(int p_cell_index);
	godot::Array get_selected_cells();
	int get_latest_selected() const;
	godot::PackedInt64Array get_path(int p_start_cell_index, int p_target_cell_index) const;
	godot::Array get_neighbors(int p_cell_index) const;

	void set_print_logs_enabled(bool p_enabled);
	bool is_print_logs_enabled() const;

	void set_print_execution_time_enabled(bool p_enabled);
	bool is_print_execution_time_enabled() const;

	bool is_cell_index_out_of_bounds(const godot::String &p_file, const godot::String &p_func, int p_line, int p_cell_index);

	InteractiveGrid3D();
	~InteractiveGrid3D();
};

VARIANT_ENUM_CAST(InteractiveGrid3D::Layout);
VARIANT_ENUM_CAST(InteractiveGrid3D::Movement);