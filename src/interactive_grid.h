/*+===================================================================
File: interactive_grid.h

Summary: InteractiveGrid is a Godot 4.5 GDExtension that allows player
         interaction with a 3D grid, including cell selection, 
		 pathfinding, and hover highlights.

Last Modified: October 12, 2025

This file is part of the InteractiveGrid GDExtension Source Code.
Repository: https://github.com/antoinecharruel/interactive_grid

Version InteractiveGrid: 1.0.0
Version: Godot Engine v4.5.stable.steam - https://godotengine.org

Author: Antoine Charruel
===================================================================+*/

#pragma once

#include "common.h"

class InteractiveGrid : public godot::Node3D {
public:
	enum LAYOUT {
		SQUARE = 0,
		HEXAGONAL = 1,
	};

	enum MOVEMENT {
		FOUR_DIRECTIONS = 0,
		SIX_DIRECTIONS = 1,
		EIGH_DIRECTIONS = 2,
	};

	InteractiveGrid(); // CONSTRUCTOR.
	~InteractiveGrid(); // DESTRUCTOR.

	// GODOT.
	static void _bind_methods();
	virtual void _ready() override;
	virtual void _physics_process(double p_delta) override;

	// --- Grid dimensions.

	// Number of rows.
	void set_rows(const unsigned int rows);
	int get_rows() const;

	// Number of columns.
	void set_columns(const unsigned int columns);
	int get_columns() const;

	// Size of a cell (width × height).
	void set_cell_size(const godot::Vector2 cell_size);
	godot::Vector2 get_cell_size() const;

	// Mesh used for each cell.
	void set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh);
	godot::Ref<godot::Mesh> get_cell_mesh() const;

	// --- Astar.

	void set_movement(unsigned int value);
	unsigned int get_movement() const;
	void configure_astar_4_dir();
	void configure_astar_6_dir();
	void configure_astar_8_dir();

	// --- Grid colors.

	// Color indicating that a cell is walkable.
	void set_walkable_color(const godot::Color color);
	godot::Color get_walkable_color() const;

	// Color indicating that a cell is unwalkable.
	void set_unwalkable_color(const godot::Color color);
	godot::Color get_unwalkable_color() const;

	// Color indicating that a cell is inaccessible (e.g., out of range).
	void set_inaccessible_color(const godot::Color color);
	godot::Color get_inaccessible_color() const;

	// Color of the currently selected cell.
	void set_selected_color(const godot::Color color);
	godot::Color get_selected_color() const;

	// Color of the calculated path (e.g., movement trajectory).
	void set_path_color(const godot::Color color);
	godot::Color get_path_color() const;

	// Color displayed when hovering over a cell with the mouse.
	void set_hovered_color(const godot::Color color);
	godot::Color get_hovered_color() const;

	// Enables passing cell bitflags through the alpha channel.
	void enable_alpha_pass(bool enabled);
	bool is_alpha_pass_enabled() const;

	// --- Grid materials.

	void set_material_override(const godot::Ref<godot::Material> &p_material);
	godot::Ref<godot::Material> get_material_override() const;
	void apply_default_material();

	// --- Highlight (Surbillance).

	void highlight_on_hover(const godot::Vector3 global_position);
	void highlight_path(godot::PackedInt64Array path);

	// --- Grid position.

	godot::Vector3 get_cell_golbal_position(const unsigned int cell_index) const;
	int get_cell_index_from_global_position(const godot::Vector3 global_position);
	godot::Vector3 get_grid_center_global_position() const;
	void center(const godot::Vector3 center_position);

	// --- Grid layout.

	void set_layout(unsigned int value);
	unsigned get_layout() const;

	// --- Grid visibility.

	void set_visible(bool visible);
	bool is_visible() const;
	void compute_inaccessible_cells(unsigned int start_cell_index);
	void hide_distant_cells(unsigned int start_cell_index, float distance);
	void set_hover_enabled(bool enabled);
	bool is_hover_enabled() const;

	// --- Grid state.

	bool is_created() const;
	bool is_centered() const;

	// --- Cell state.

	// Public Getters
	bool is_cell_walkable(unsigned int cell_index) const;
	bool is_cell_inaccesible(unsigned int cell_index) const;
	bool is_cell_in_void(unsigned int cell_index) const;
	bool is_cell_hovered(unsigned int cell_index) const;
	bool is_cell_selected(unsigned int cell_index) const;
	bool is_cell_on_path(unsigned int cell_index) const;
	bool is_cell_visible(unsigned int cell_index) const;

	// Public Setters
	void set_cell_walkable(unsigned int cell_index, bool is_walkable);
	void set_cell_visible(unsigned int cell_index, bool is_visible);

	void reset_cells_state();

	// --- Cell color.

	void set_cell_color(unsigned int cell_index, godot::Color color);

	// --- Grid masks.

	void set_obstacles_collision_masks(unsigned int masks);
	int get_obstacles_collision_masks();

	void set_grid_floor_collision_masks(unsigned int masks);
	int get_grid_floor_collision_masks();

	// --- User interaction.

	void select_cell(const godot::Vector3 global_position);
	godot::Array get_selected_cells();
	int get_latest_selected();
	godot::PackedInt64Array get_path(unsigned int start_cell_index,
			unsigned int target_cell_index);

private:
	GDCLASS(InteractiveGrid, Node3D);

	typedef struct Cell {
		// Cell data.
		uint16_t index = -1;
		godot::Transform3D local_xform;
		godot::Transform3D global_xform;
		godot::Color color;
		uint32_t flags = 0;
	} Cell;

	// Grid flags.

	static constexpr int GFL_DEBUG = 1 << 0;
	static constexpr int GFL_CREATED = 1 << 1;
	static constexpr int GFL_CENTERED = 1 << 2;
	static constexpr int GFL_VISIBLE = 1 << 3;
	static constexpr int GFL_CELL_INACCESSIBLE_HIDDEN = 1 << 4;
	static constexpr int GFL_CELL_DISTANT_HIDDEN = 1 << 5;
	static constexpr int GFL_HOVER_ENABLED = 1 << 6;

	// Cell flags.

	static constexpr int CFL_WALKABLE = 1 << 0;
	static constexpr int CFL_INACCESSIBLE = 1 << 1;
	static constexpr int CFL_IN_VOID = 1 << 2;
	static constexpr int CFL_HOVERED = 1 << 3;
	static constexpr int CFL_SELECTED = 1 << 4;
	static constexpr int CFL_PATH = 1 << 5;
	static constexpr int CFL_VISIBLE = 1 << 6;

	void create();

	// --- Grid initialization.

	void init_multi_mesh();
	void init_astar();

	// --- Grid position.

	void align_cells_with_floor();
	void scan_environnement_obstacles();

	// --- Grid layout.

	void layout(const godot::Vector3 center_position);
	void layout_cells_as_square_grid(const godot::Vector3 center_position);
	void layout_cells_as_hexagonal_grid(const godot::Vector3 center_position);

	// --- Grid materials.

	void apply_material(const godot::Ref<godot::Material> &p_material);

	// --- Grid visibility.

	void set_cells_visible(bool visible_param);

	// --- Cell state.

	// Private Setters
	void set_cell_inaccesible(unsigned int cell_index, bool is_inaccesible);
	void set_cell_in_void(unsigned int cell_index, bool is_in_void);
	void set_cell_hovered(unsigned int cell_index, bool is_hovered);
	void set_cell_selected(unsigned int cell_index, bool is_selected);
	void set_cell_on_path(unsigned int cell_index, bool is_on_path);

	/*--------------------------------------------------------------------
    Grid data members.
  --------------------------------------------------------------------*/

	// --- Grid.

	unsigned int _rows{ 9 }; // ROWS.
	unsigned int _columns{ 9 }; // COLUMNS.
	uint32_t _flags = 0; // FLAGS.

	godot::Vector3 _grid_center_position = godot::Vector3(0.0f, 0.0f, 0.0f);
	godot::Vector3 _grid_offset = godot::Vector3(0.0f, 0.0f, 0.0f);
	godot::Ref<godot::AStar2D> _astar;

	unsigned int _layout{ 0 }; // SQUARE = 0, HEXAGONAL = 1.
	unsigned int _movement{ 0 }; // ORTHOGONAL = 0, DIAGONAL = 1.
	unsigned int _obstacles_collision_masks{ 1 << 13 }; // mask 14 = pow(2,13) = 1 << 13 = 8192
	unsigned int _floor_collision_masks{ 1 << 14 }; // mask 15 = pow(2,14) = 1 << 14 = 16384

	// --- Cells.

	godot::Ref<godot::Mesh> _cell_mesh;
	godot::MultiMeshInstance3D *_multimesh_instance;
	godot::Ref<godot::MultiMesh> _multimesh;
	godot::Vector2 _cell_size = godot::Vector2(1.0f, 1.0f);
	std::vector<Cell *> _cells;
	godot::Array _selected_cells;
	int _hovered_cell_index{ -1 };

	bool is_cell_index_out_of_bounds(const godot::String &file, const godot::String &func, const int &line, const unsigned int &cell_index);

	// --- colors.

	godot::Color _walkable_color{ godot::Color(0.9411765, 1, 0.9411765, (80.0 / 255.0)) }; // HONEYDEW
	godot::Color _unwalkable_color{
		godot::Color(0.8039216, 0.36078432, 0.36078432, (241.0 / 255.0))
	}; // INDIAN_RED
	godot::Color _inaccessible_color{
		godot::Color(1, 1, 1, 255)
	}; // #ffffff00
	godot::Color _selected_color{
		godot::Color(0.8784314, 1, 1, 255)
	}; // LIGHT_CYAN
	godot::Color _path_color{ godot::Color(0.5647059, 0.93333334, 0.5647059, 255) };
	godot::Color _hovered_color{
		godot::Color(1, 0.84313726, 0, 255)
	};

	// --- material.

	godot::Ref<godot::Material> _material_override;

	// --- Scan environnement.

	godot::Ref<godot::BoxShape3D> _obstacle_shape;

	bool _alpha_pass{ false };
};
