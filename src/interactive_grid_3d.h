/*+===================================================================
File: interactive_grid.h

Summary: InteractiveGrid is a Godot 4.5 GDExtension that allows player
         interaction with a 3D grid, including cell selection, 
		 pathfinding, and hover highlights.

Last Modified: November 25, 2025

This file is part of the InteractiveGrid GDExtension Source Code.
Repository: https://github.com/antoinecharruel/interactive_grid

Version InteractiveGrid: 1.5.0
Version: Godot Engine v4.5.stable.steam - https://godotengine.org

Author: Antoine Charruel
===================================================================+*/

#pragma once

#include "common.h"

// Godot engine
#include <godot_cpp/classes/a_star2d.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <algorithm>
#include <chrono>
#include <queue>

class InteractiveGrid3D : public godot::Node3D {
private:
	GDCLASS(InteractiveGrid3D, Node3D);

	enum LAYOUT {
		SQUARE = 0,
		HEXAGONAL = 1,
	};

	enum MOVEMENT {
		FOUR_DIRECTIONS = 0,
		SIX_DIRECTIONS = 1,
		EIGH_DIRECTIONS = 2,
	};

	struct DebugOptions {
		bool print_logs_enabled = false;
		bool print_execution_time_enabled = false;
	} _debug_options;

	typedef struct Cell {
		// Cell data
		uint16_t index = -1;
		godot::Transform3D local_xform;
		godot::Transform3D global_xform;
		godot::Color color;
		godot::PackedInt64Array neighbors{};
		uint32_t flags = 0;
	} Cell;

	// Grid flags

	static constexpr int GFL_DEBUG = 1 << 0;
	static constexpr int GFL_CREATED = 1 << 1;
	static constexpr int GFL_CENTERED = 1 << 2;
	static constexpr int GFL_VISIBLE = 1 << 3;
	static constexpr int GFL_CELL_UNREACHABLE_HIDDEN = 1 << 4;
	static constexpr int GFL_CELL_DISTANT_HIDDEN = 1 << 5;
	static constexpr int GFL_HOVER_ENABLED = 1 << 6;

	// Cell flags

	static constexpr int CFL_WALKABLE = 1 << 0;
	static constexpr int CFL_REACHABLE = 1 << 1;
	static constexpr int CFL_IN_VOID = 1 << 2;
	static constexpr int CFL_HOVERED = 1 << 3;
	static constexpr int CFL_SELECTED = 1 << 4;
	static constexpr int CFL_PATH = 1 << 5;
	static constexpr int CFL_VISIBLE = 1 << 6;

	void _create();
	void _destroy();

	// --- Grid initialization

	void _init_multi_mesh();
	void _init_astar();

	// --- Grid position

	void _align_cells_with_floor();
	void _scan_environnement_obstacles();

	// --- Grid layout

	void _layout(godot::Vector3 center_position);
	void _layout_cells_as_square_grid(godot::Vector3 center_position);
	void _layout_cells_as_hexagonal_grid(godot::Vector3 center_position);

	// --- Grid materials

	void _apply_material(const godot::Ref<godot::Material> &p_material);

	// --- Grid visibility

	void _set_cells_visible(bool visible);

	// --- Cell state

	void _set_cell_in_void(unsigned int cell_index, bool is_in_void);
	void _set_cell_hovered(unsigned int cell_index, bool is_hovered);
	void _set_cell_selected(unsigned int cell_index, bool is_selected);
	void _set_cell_on_path(unsigned int cell_index, bool is_on_path);

	/*--------------------------------------------------------------------
    Grid data members
  --------------------------------------------------------------------*/

	struct Data {
		// Grid.

		unsigned int rows{ 9 };
		unsigned int columns{ 9 };
		uint32_t flags = 0;

		godot::Vector3 grid_center_position = godot::Vector3(0.0f, 0.0f, 0.0f);
		godot::Vector3 grid_offset = godot::Vector3(0.0f, 0.0f, 0.0f);
		godot::Ref<godot::AStar2D> astar;

		unsigned int layout_index{ 0 };
		unsigned int movement{ 0 };
		unsigned int obstacles_collision_masks{ 1 << 13 }; // mask 14 = pow(2,13) = 1 << 13 = 8192
		unsigned int floor_collision_masks{ 1 << 14 }; // mask 15 = pow(2,14) = 1 << 14 = 16384

		// Cells.

		godot::Ref<godot::Mesh> cell_mesh;
		godot::MultiMeshInstance3D *multimesh_instance;
		godot::Ref<godot::MultiMesh> multimesh;
		godot::Vector2 cell_size = godot::Vector2(1.0f, 1.0f);
		std::vector<Cell *> cells;
		godot::Array selected_cells;
		int hovered_cell_index{ -1 };

		// Colors.

		godot::Color walkable_color{ godot::Color(0.5, 0.65, 1.0, 1) }; // BLUE
		godot::Color unwalkable_color{ godot::Color(0.8039216, 0.36078432, 0.36078432, 1.0) }; // INDIAN_RED
		godot::Color unreachable_color{ godot::Color(1.0, 1.0, 1.0, 1.0) }; // #ffffff00
		godot::Color selected_color{ godot::Color(0.8784314, 1.0, 1.0, 1.0) }; // LIGHT_CYAN
		godot::Color path_color{ godot::Color(0.5647059, 0.93333334, 0.5647059, 1) };
		godot::Color hovered_color{ godot::Color(1.0, 0.84313726, 0, 1.0) };

		// Material.

		godot::Ref<godot::Material> material_override;

		// Scan environnement.

		godot::Ref<godot::BoxShape3D> obstacle_shape;

	} data;

protected:
	static void _bind_methods();

public:
	InteractiveGrid3D();
	~InteractiveGrid3D();

	// Godot engine
	virtual void _ready() override;
	virtual void _physics_process(double p_delta) override;

	// --- Grid dimensions

	// Number of rows
	void set_rows(unsigned int rows);
	int get_rows() const;

	// Number of columns
	void set_columns(unsigned int columns);
	int get_columns() const;

	// Size of a cell (width × height)
	void set_cell_size(godot::Vector2 cell_size);
	godot::Vector2 get_cell_size() const;

	// Mesh used for each cell
	void set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh);
	godot::Ref<godot::Mesh> get_cell_mesh() const;

	// --- Layout

	void set_movement(unsigned int value);
	unsigned int get_movement() const;

	// --- Grid colors

	// Color indicating that a cell is walkable
	void set_walkable_color(const godot::Color &p_color);
	godot::Color get_walkable_color() const;

	// Color indicating that a cell is unwalkable
	void set_unwalkable_color(const godot::Color &p_color);
	godot::Color get_unwalkable_color() const;

	// Color indicating that a cell is unreachable
	void set_unreachable_color(const godot::Color &p_color);
	godot::Color get_unreachable_color() const;

	// Color of the currently selected cell
	void set_selected_color(const godot::Color &p_color);
	godot::Color get_selected_color() const;

	// Color of the calculated path
	void set_path_color(const godot::Color &p_color);
	godot::Color get_path_color() const;

	// Color displayed when hovering over a cell with the mouse
	void set_hovered_color(const godot::Color &p_color);
	godot::Color get_hovered_color() const;

	// --- Grid materials

	void set_material_override(const godot::Ref<godot::Material> &p_material);
	godot::Ref<godot::Material> get_material_override() const;
	void apply_default_material();

	// --- Highlight (Surbillance)

	void highlight_on_hover(godot::Vector3 global_position);
	void highlight_path(const godot::PackedInt64Array &p_path);

	// --- Grid position

	godot::Vector3 get_cell_global_position(unsigned int cell_index) const;
	int get_cell_index_from_global_position(godot::Vector3 global_position);
	godot::Vector3 get_grid_center_global_position() const;
	void center(godot::Vector3 center_position);

	// --- Grid layout

	void set_layout(unsigned int value);
	unsigned get_layout() const;

	// --- Grid visibility

	void set_visible(bool visible);
	bool is_visible() const;

	// --- Astar

	void configure_astar();
	void configure_astar_4_dir();
	void configure_astar_6_dir();
	void configure_astar_8_dir();

	// --- Compute

	void compute_unreachable_cells(unsigned int start_cell_index);
	void breadth_first_search(unsigned int start_cell_index);

	void hide_distant_cells(unsigned int start_cell_index, float distance);
	void set_hover_enabled(bool enabled);
	bool is_hover_enabled() const;

	// --- Grid state

	bool is_created() const;
	bool is_centered() const;

	// --- Cell state

	// Public Getters
	bool is_cell_walkable(unsigned int cell_index) const;
	bool is_cell_reachable(unsigned int cell_index) const;
	bool is_cell_in_void(unsigned int cell_index) const;
	bool is_cell_hovered(unsigned int cell_index) const;
	bool is_cell_selected(unsigned int cell_index) const;
	bool is_cell_on_path(unsigned int cell_index) const;
	bool is_cell_visible(unsigned int cell_index) const;

	// Public Setters
	void set_cell_walkable(unsigned int cell_index, const bool is_walkable);
	void set_cell_reachable(unsigned int cell_index, bool is_unreachable);
	void set_cell_visible(unsigned int cell_index, const bool is_visible);

	void reset_cells_state();

	// --- Cell color

	void set_cell_color(unsigned int cell_index, const godot::Color &p_color);

	// --- Grid masks

	void set_obstacles_collision_masks(unsigned int masks);
	int get_obstacles_collision_masks();

	void set_grid_floor_collision_masks(unsigned int masks);
	int get_grid_floor_collision_masks();

	// --- User interaction

	void select_cell(godot::Vector3 global_position);
	godot::Array get_selected_cells();
	int get_latest_selected();
	godot::PackedInt64Array get_path(unsigned int start_cell_index, unsigned int target_cell_index) const;
	godot::PackedInt64Array get_neighbors(unsigned int cell_index) const;

	// --- Debug

	void set_print_logs_enabled(bool enabled);
	bool is_print_logs_enabled() const;

	void set_print_execution_time_enabled(bool enabled);
	bool is_print_execution_time_enabled() const;

	bool is_cell_index_out_of_bounds(const godot::String &file, const godot::String &func, int line, unsigned int cell_index);
};
