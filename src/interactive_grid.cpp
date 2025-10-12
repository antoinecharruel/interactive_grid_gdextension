/*+===================================================================
File: interactive_grid.cpp

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

#include "interactive_grid.h"

InteractiveGrid::InteractiveGrid() {} // CONSTRUCTOR.

InteractiveGrid::~InteractiveGrid() {} // DESTRUCTOR.

void InteractiveGrid::_bind_methods() {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Summary: _bind_methods, is a static function that Godot will call to 
	         find out which methods can be called and which properties it
			 exposes.

    Last Modified: October 01, 2025
  -----------------------------------------------------------------F-F*/

	// --- Grid dimensions.

	// Number of rows.
	godot::ClassDB::bind_method(godot::D_METHOD("set_rows"), &InteractiveGrid::set_rows);
	godot::ClassDB::bind_method(godot::D_METHOD("get_rows"), &InteractiveGrid::get_rows);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "_rows"), "set_rows",
			"get_rows");

	// Number of columns.
	godot::ClassDB::bind_method(godot::D_METHOD("set_columns"),
			&InteractiveGrid::set_columns);
	godot::ClassDB::bind_method(godot::D_METHOD("get_columns"),
			&InteractiveGrid::get_columns);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "_columns"),
			"set_columns", "get_columns");

	// Size of a cell (width × height).
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_size"),
			&InteractiveGrid::set_cell_size);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_size"),
			&InteractiveGrid::get_cell_size);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::VECTOR2, "cell_size"),
			"set_cell_size", "get_cell_size");

	// Mesh used for each cell.
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_mesh", "_cell_mesh"),
			&InteractiveGrid::set_cell_mesh);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_mesh"),
			&InteractiveGrid::get_cell_mesh);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "_cell_mesh",
						 godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"),
			"set_cell_mesh", "get_cell_mesh");

	// --- Grid colors.

	// Color indicating that a cell is walkable.
	godot::ClassDB::bind_method(godot::D_METHOD("set_walkable_color"),
			&InteractiveGrid::set_walkable_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_walkable_color"),
			&InteractiveGrid::get_walkable_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "walkable color"),
			"set_walkable_color", "get_walkable_color");

	// Color indicating that a cell is unwalkable.
	godot::ClassDB::bind_method(godot::D_METHOD("set_unwalkable_color"),
			&InteractiveGrid::set_unwalkable_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_unwalkable_color"),
			&InteractiveGrid::get_unwalkable_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "unwalkable color"),
			"set_unwalkable_color", "get_unwalkable_color");

	// Color indicating that a cell is inaccessible (e.g., out of range).
	godot::ClassDB::bind_method(godot::D_METHOD("set_inaccessible_color"),
			&InteractiveGrid::set_inaccessible_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_inaccessible_color"),
			&InteractiveGrid::get_inaccessible_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "inaccessible color"),
			"set_inaccessible_color", "get_inaccessible_color");

	// Color of the currently selected cell.
	godot::ClassDB::bind_method(godot::D_METHOD("set_selected_color"),
			&InteractiveGrid::set_selected_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_selected_color"),
			&InteractiveGrid::get_selected_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "selected color"),
			"set_selected_color", "get_selected_color");

	// Color of the calculated path (e.g., movement trajectory).
	godot::ClassDB::bind_method(godot::D_METHOD("set_path_color"),
			&InteractiveGrid::set_path_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_path_color"),
			&InteractiveGrid::get_path_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "path color"),
			"set_path_color", "get_path_color");

	// Color displayed when hovering over a cell with the mouse.
	godot::ClassDB::bind_method(godot::D_METHOD("set_hovered_color"),
			&InteractiveGrid::set_hovered_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_hovered_color"),
			&InteractiveGrid::get_hovered_color);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "hovered color"),
			"set_hovered_color", "get_hovered_color");

	// Enables passing cell bitflags through the alpha channel.
	godot::ClassDB::bind_method(godot::D_METHOD("enable_alpha_pass", "enabled"),
			&InteractiveGrid::enable_alpha_pass);
	godot::ClassDB::bind_method(godot::D_METHOD("is_alpha_pass_enabled"),
			&InteractiveGrid::is_alpha_pass_enabled);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "enable_alpha_pass"),
			"enable_alpha_pass", "is_alpha_pass_enabled");

	// --- Grid materials.

	godot::ClassDB::bind_method(godot::D_METHOD("get_material_override"),
			&InteractiveGrid::get_material_override);
	godot::ClassDB::bind_method(
			godot::D_METHOD("set_material_override", "material"),
			&InteractiveGrid::set_material_override);
	ADD_PROPERTY(
			godot::PropertyInfo(godot::Variant::OBJECT, "_material_override",
					godot::PROPERTY_HINT_RESOURCE_TYPE, "Material"),
			"set_material_override", "get_material_override");

	// --- Highlight (Surbillance).

	godot::ClassDB::bind_method(godot::D_METHOD("highlight_on_hover", "global_position"),
			&InteractiveGrid::highlight_on_hover);
	godot::ClassDB::bind_method(godot::D_METHOD("highlight_path", "path"),
			&InteractiveGrid::highlight_path);

	// --- Grid position.

	godot::ClassDB::bind_method(godot::D_METHOD("center", "center_position"), &InteractiveGrid::center);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_golbal_position", "cell_index"),
			&InteractiveGrid::get_cell_golbal_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_index_from_global_position", "global_position"),
			&InteractiveGrid::get_cell_index_from_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_center_global_position"),
			&InteractiveGrid::get_grid_center_global_position);

	// --- Grid layout.

	godot::ClassDB::bind_method(godot::D_METHOD("set_layout", "value"),
			&InteractiveGrid::set_layout);
	godot::ClassDB::bind_method(godot::D_METHOD("get_layout"),
			&InteractiveGrid::get_layout);
	ADD_PROPERTY(godot::PropertyInfo(
						 godot::Variant::INT, "layout", godot::PROPERTY_HINT_ENUM,
						 "SQUARE, HEXAGONAL_WORK_IN_PROGRESS"),
			"set_layout", "get_layout");

	// --- Grid visibility.

	godot::ClassDB::bind_method(godot::D_METHOD("set_visible", "visible"),
			&InteractiveGrid::set_visible);
	godot::ClassDB::bind_method(godot::D_METHOD("is_visible"),
			&InteractiveGrid::is_visible);
	godot::ClassDB::bind_method(godot::D_METHOD("compute_inaccessible_cells", "start_cell_index"),
			&InteractiveGrid::compute_inaccessible_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("hide_distant_cells", "start_cell_index", "distance"),
			&InteractiveGrid::hide_distant_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("set_hover_enabled", "enabled"),
			&InteractiveGrid::set_hover_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_hover_enabled"),
			&InteractiveGrid::is_hover_enabled);

	// --- Grid state.

	godot::ClassDB::bind_method(godot::D_METHOD("is_grid_created"),
			&InteractiveGrid::is_created);
	godot::ClassDB::bind_method(godot::D_METHOD("reset_cells_state"),
			&InteractiveGrid::reset_cells_state);

	// --- Cell state.

	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_walkable", "cell_index"),
			&InteractiveGrid::is_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_inaccesible", "cell_index"),
			&InteractiveGrid::is_cell_inaccesible);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_hovered", "cell_index"),
			&InteractiveGrid::is_cell_hovered);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_selected", "cell_index"),
			&InteractiveGrid::is_cell_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_visible", "cell_index"),
			&InteractiveGrid::is_cell_visible);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_walkable", "cell_index", "is_walkable"),
			&InteractiveGrid::set_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_inaccesible", "cell_index", "set_cell_inaccesible"),
			&InteractiveGrid::set_cell_inaccesible);
	// --- Cell color.
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_color", "cell_index", "color"),
			&InteractiveGrid::set_cell_color);

	// --- Masks.

	godot::ClassDB::bind_method(godot::D_METHOD("set_obstacles_collision_masks", "masks"), &InteractiveGrid::set_obstacles_collision_masks);
	godot::ClassDB::bind_method(godot::D_METHOD("get_obstacles_collision_masks"), &InteractiveGrid::get_obstacles_collision_masks);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "obstacles_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_RENDER),
			"set_obstacles_collision_masks", "get_obstacles_collision_masks");

	godot::ClassDB::bind_method(godot::D_METHOD("set_grid_floor_collision_masks", "masks"), &InteractiveGrid::set_grid_floor_collision_masks);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_floor_collision_masks"), &InteractiveGrid::get_grid_floor_collision_masks);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "floor_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_RENDER),
			"set_grid_floor_collision_masks", "get_grid_floor_collision_masks");

	// --- Astar.

	godot::ClassDB::bind_method(godot::D_METHOD("set_movement", "value"),
			&InteractiveGrid::set_movement);
	godot::ClassDB::bind_method(godot::D_METHOD("get_movement"),
			&InteractiveGrid::get_movement);
	ADD_PROPERTY(godot::PropertyInfo(
						 godot::Variant::INT, "movement", godot::PROPERTY_HINT_ENUM,
						 "FOUR-DIRECTIONS,SIX-DIRECTIONS,EIGH-DIRECTIONS"),
			"set_movement", "get_movement");

	// --- User interaction.

	godot::ClassDB::bind_method(godot::D_METHOD("select_cell", "global_position"),
			&InteractiveGrid::select_cell);
	godot::ClassDB::bind_method(godot::D_METHOD("get_selected_cells"),
			&InteractiveGrid::get_selected_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("get_latest_selected"),
			&InteractiveGrid::get_latest_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("get_path", "start_cell_index", "target_cell_index"),
			&InteractiveGrid::get_path);
}

void InteractiveGrid::_ready(void) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary:  Called when the node enters the scene tree for the first time.

  Last Modified: September 19, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
}

void InteractiveGrid::_physics_process(double p_delta) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Called every frame. 'delta' is the elapsed time since the 
           previous frame.

  Last Modified: 03 May 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	create(); // Create the grid at startup.
}

void InteractiveGrid::set_rows(const unsigned int rows) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the number of rows in the grid.

  Last Modified: April 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_rows = rows;
}

int InteractiveGrid::get_rows(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the number of rows in the grid.

  Last Modified: April 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _rows;
}

void InteractiveGrid::set_columns(const unsigned int columns) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the number of columns in the grid.

  Last Modified: April 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_columns = columns;
}

int InteractiveGrid::get_columns(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the columns of rows in the grid.

  Last Modified: April 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _columns;
}

void InteractiveGrid::set_cell_size(const godot::Vector2 cell_size) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the size of each cell in the grid. The provided 
           value will be stored and used for grid layout.

  Last Modified: May 03, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_cell_size = cell_size;
}

godot::Vector2 InteractiveGrid::get_cell_size(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the size of a single cell in the grid.
           This value is used to manage grid dimensions and cell 
		   positioning.

  Last Modified: May 03, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _cell_size;
}

void InteractiveGrid::set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the mesh used for each cell of the grid.

  Last Modified: May 03, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_cell_mesh = p_mesh;
}

godot::Ref<godot::Mesh> InteractiveGrid::get_cell_mesh() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary:  Returns the mesh used for each cell of the grid.

  Last Modified: September 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _cell_mesh;
}

void InteractiveGrid::set_walkable_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the walkable color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_walkable_color = color;
}

godot::Color InteractiveGrid::get_walkable_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the walkable color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _walkable_color;
}

void InteractiveGrid::set_unwalkable_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the unwalkable color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_unwalkable_color = color;
}

godot::Color InteractiveGrid::get_unwalkable_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the unvalid color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _unwalkable_color;
}

void InteractiveGrid::set_inaccessible_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the inaccessible color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_inaccessible_color = color;
}

godot::Color InteractiveGrid::get_inaccessible_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the inaccessible color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _inaccessible_color;
}

void InteractiveGrid::set_selected_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the selected color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_selected_color = color;
}

godot::Color InteractiveGrid::get_selected_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the selected color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _selected_color;
}

void InteractiveGrid::set_path_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the path color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_path_color = color;
}

godot::Color InteractiveGrid::get_path_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the path color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _path_color;
}

void InteractiveGrid::set_hovered_color(const godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the hovered color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_hovered_color = color;
}

godot::Color InteractiveGrid::get_hovered_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the hovered color for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _hovered_color;
}

void InteractiveGrid::enable_alpha_pass(bool enabled) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Enables passing cell bitflags through the alpha channel.
           Can be used within shader scripts to modify rendering
		   behavior.

  Last Modified: October 12, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_alpha_pass = enabled;
}

bool InteractiveGrid::is_alpha_pass_enabled() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns whether alpha pass for cell bitflags is currently 
           enabled. Used to check if cell bitflags are transmitted 
		   through the alpha channel.

  Last Modified: October 12, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _alpha_pass;
}

void InteractiveGrid::set_material_override(
		const godot::Ref<godot::Material> &p_material) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the material override for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_material_override = p_material;
}

godot::Ref<godot::Material> InteractiveGrid::get_material_override() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the material override for the grid.

  Last Modified: April 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _material_override;
}

void InteractiveGrid::apply_default_material() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Creates and applies a default ShaderMaterial to the grid's
           MultiMeshInstance.

  Last Modified: October 01, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (_multimesh_instance == nullptr) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	godot::Ref<godot::Shader> shader;
	shader.instantiate();

	// Code GLSL/Shader
	godot::String shader_code = R"(
		shader_type spatial;
		render_mode unshaded, cull_disabled, depth_draw_opaque;
		varying vec4 instance_c;

		void vertex() {
			instance_c = INSTANCE_CUSTOM;
		}

		void fragment() {
			ALBEDO = instance_c.rgb;
			ALPHA = instance_c.a;
		}
    )";

	shader->set_code(shader_code);

	// Create a ShaderMaterial, instantiate it, and set the shader.
	godot::Ref<godot::ShaderMaterial> shader_material;
	shader_material.instantiate();
	shader_material->set_shader(shader);

	// Apply it as the material_override of the MultiMeshInstance.
	_multimesh_instance->set_material_override(shader_material);

	PrintLine(__FILE__, __FUNCTION__, __LINE__, "Default ShaderMaterial created and applied.");
}

void InteractiveGrid::highlight_on_hover(const godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Updates the hover highlight state based on the specified 
           global position. Identifies the corresponding cell, clears
		   any previous hover, and applies the hover color unless the
		   cell is already selected.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// If the grid isn’t visible, exit early.
	if (is_visible() == false) {
		return; // !Exit
	}

	// Do not hover if the grid is not centered.
	if (is_centered() == false) {
		return; // !Exit
	}

	if (is_hover_enabled() == false) {
		return; // !Exit
	}

	// Retrieve the index of the cell that corresponds to the supplied
	// global position.
	int closest_index = get_cell_index_from_global_position(global_position);

	// No cell under the mouse: clean the previously hovered cell (if any).
	if (closest_index == -1 || !is_cell_visible(closest_index)) {
		if (_hovered_cell_index > -1) {
			set_cell_hovered(_hovered_cell_index, false);

			bool hovered_cell_is_selected = is_cell_selected(_hovered_cell_index);

			if (!hovered_cell_is_selected) {
				set_cell_color(_hovered_cell_index, _walkable_color);
			}

			_hovered_cell_index = -1;
		}
		return;
	}

	// Already hovering over the same cell: nothing to do.
	if (closest_index == _hovered_cell_index) {
		return;
	}

	// Check whether the new cell is already selected.
	bool new_is_selected = is_cell_selected(closest_index);

	// Clear the previously hovered cell (if it exists).
	if (_hovered_cell_index > -1) {
		bool old_is_selected = is_cell_selected(_hovered_cell_index);

		set_cell_hovered(_hovered_cell_index, false);

		if (!old_is_selected) {
			set_cell_color(_hovered_cell_index, _walkable_color);
		}

		_hovered_cell_index = -1;
	}

	// Skip non-walkable cells: only allow hovering on walkable cells.
	bool walkable = is_cell_walkable(closest_index);
	if (!walkable) {
		return;
	}

	// Skip inaccessible cells.
	bool inaccessible = is_cell_inaccesible(closest_index);
	if (inaccessible) {
		return;
	}

	// If the new cell is not selected, mark it as hovered.
	if (!new_is_selected) {
		_hovered_cell_index = closest_index;
		set_cell_hovered(_hovered_cell_index, true);
	}
}

void InteractiveGrid::highlight_path(godot::PackedInt64Array path) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Highlights a given path on the grid by changing the color of 
           each cell along the path to the predefined _path_color.

  Last Modified: October 11, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	for (int step = 0; step < path.size(); step++) {
		int cell_index = path[step];
		set_cell_on_path(cell_index, true);
	}
}

godot::Vector3 InteractiveGrid::get_cell_golbal_position(const unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the global world‑space position of the cell identified
           by index. The method fetches the cell’s Transform3D from the
           internal `_cells` array and extracts its origin component.

  Last Modified: September 26, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	godot::Vector3 cell_global_position = _cells.at(cell_index)->global_xform.origin;

	return cell_global_position;
}

godot::Vector3 InteractiveGrid::get_grid_center_global_position() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the central position of the interactive grid in world
           coordinates.

  Last Modified: September 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _grid_center_position;
}

void InteractiveGrid::set_layout(unsigned int value) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Sets the grid layout.

	Last Modified: October 05, 2025
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_layout = value;
}

unsigned int InteractiveGrid::get_layout() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Returns the current grid layout.

	Last Modified: October 05, 2025
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _layout;
}

void InteractiveGrid::center(const godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Called to re-center the grid. This also resets the grid state.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_flags &= ~GFL_CENTERED; // Reset

	set_hover_enabled(false); // Prevent hover during grid recentering.

	reset_cells_state();
	layout(center_position);
	align_cells_with_floor();
	scan_environnement_obstacles();

	set_hover_enabled(true);
	_flags |= GFL_CENTERED;
}

void InteractiveGrid::set_visible(bool visible_param) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the visibility of the grid.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(_flags & GFL_CREATED)) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit.
	}

	if ((_flags & GFL_VISIBLE) && !visible_param) {
		// Visible.
		set_cells_visible(false);
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "setInteractiveGridVisible : false");
		_flags &= ~GFL_VISIBLE;
	} else if (!(_flags & GFL_VISIBLE) && visible_param) {
		// Not visible.
		set_cells_visible(true);
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "setInteractiveGridVisible : true");
		_flags |= GFL_VISIBLE;
	}
}

bool InteractiveGrid::is_visible() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether the grid is currently visible in the scene.

  Last Modified: May 03, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (_flags & GFL_VISIBLE) != 0;
}

void InteractiveGrid::compute_inaccessible_cells(unsigned int start_cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Iterates over all grid cells and marks as inaccessible those
           cells that cannot be reached from the specified start cell.
		   Updates the visual representation of inaccessible cells by
		   applying the _inaccessible_color to the grid's multimesh.
			
		   Inaccessible cells are not marked as unwalkable.
           I prefer this option so that this gdextension can adapt
           to any type of game, for example a game that implements
           teleportation.


	       // TODO: Improve performance: 
	       current implementation iterates through
	       all cells and calculates paths for each, which can be slow
           for large grids.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (start_cell_index >= (_rows * _columns)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "Cell index out of bounds.");
		return;
	}

	if ((_flags & GFL_VISIBLE) && !(_flags & GFL_CELL_INACCESSIBLE_HIDDEN)) {
		// Iterate through the cells.
		for (int row = 0; row < _rows; row++) {
			for (int column = 0; column < _columns; column++) {
				const int index = row * _columns + column;
				godot::PackedInt64Array path = get_path(start_cell_index, index);

				if (path.is_empty()) {
					bool walkable = is_cell_walkable(index);

					if (walkable) {
						set_cell_inaccesible(index, true);
					}
				}
			}
		}
		_flags |= GFL_CELL_INACCESSIBLE_HIDDEN;
	}
}

void InteractiveGrid::hide_distant_cells(unsigned int start_cell_index, float distance) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Iterates over all grid cells and hides those located farther 
           than the specified distance from the start cell. Marks them
           as non-walkable.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (start_cell_index >= (_rows * _columns)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "Cell index out of bounds.");
		return;
	}

	if ((_flags & GFL_VISIBLE) && !(_flags & GFL_CELL_DISTANT_HIDDEN)) {
		// Iterate through the cells.
		for (int row = 0; row < _rows; row++) {
			for (int column = 0; column < _columns; column++) {
				const int index = row * _columns + column;

				godot::Vector3 start_cell_position = _cells.at(start_cell_index)->global_xform.origin;
				godot::Vector3 index_cell_position = _cells.at(index)->global_xform.origin;

				if (start_cell_position.distance_to(index_cell_position) > distance) {
					set_cell_visible(index, false);
					_cells.at(index)->flags &= ~CFL_WALKABLE;
				}
			}
		}
		_flags |= GFL_CELL_DISTANT_HIDDEN;
	}
}

void InteractiveGrid::set_hover_enabled(bool disabled) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Enables or disables hover functionality on the grid.

	Last Modified: October 10, 2025
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(_flags & GFL_CREATED)) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit.
	}

	if (disabled) {
		_flags |= GFL_HOVER_ENABLED; // Set the flag
	} else {
		_flags &= ~GFL_HOVER_ENABLED; // Clear the flag
	}
}

bool InteractiveGrid::is_hover_enabled() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether hover functionality is currently disabled
	       on the grid.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (_flags & GFL_HOVER_ENABLED) != 0;
}

bool InteractiveGrid::is_created() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks if the grid has been created.

  Last Modified: May 03, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (_flags & GFL_CREATED) != 0;
}

bool InteractiveGrid::is_centered() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether the grid is currently centered. Returns true
	       if the GFL_CENTERED flag is set, false otherwise.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (_flags & GFL_CENTERED) != 0;
}

bool InteractiveGrid::is_cell_walkable(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as walkable.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_WALKABLE) != 0;
}

bool InteractiveGrid::is_cell_inaccesible(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as inaccesible.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_INACCESSIBLE) != 0;
}

bool InteractiveGrid::is_cell_in_void(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether a specific cell is marked as "in void".

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_IN_VOID) != 0;
}

bool InteractiveGrid::is_cell_hovered(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as hovered.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_HOVERED) != 0;
}

bool InteractiveGrid::is_cell_selected(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as selected.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_SELECTED) != 0;
}

bool InteractiveGrid::is_cell_on_path(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as part of the path.

  Last Modified: October 11, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_PATH) != 0;
}

bool InteractiveGrid::is_cell_visible(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently
           marked as visible.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return (_cells.at(cell_index)->flags & CFL_VISIBLE) != 0;
}

void InteractiveGrid::set_cell_walkable(const unsigned int cell_index, bool is_walkable) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific cell is walkable or not.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_walkable) {
		_cells.at(cell_index)->flags |= CFL_WALKABLE;

		set_cell_color(cell_index, _walkable_color);
	} else if (!is_walkable) {
		_cells.at(cell_index)->flags &= ~CFL_WALKABLE;
		set_cell_color(cell_index, _unwalkable_color);
	}
}

void InteractiveGrid::set_cell_visible(unsigned int cell_index, bool is_visible) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the visibility of a grid cell identified by cell_index.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	godot::Color current_cell_color = _cells.at(cell_index)->color;

	if (is_visible) {
		_cells.at(cell_index)->flags |= CFL_VISIBLE;
		set_cell_color(cell_index, current_cell_color);
	} else if (!is_visible) {
		current_cell_color.a = 0.0;
		_multimesh->set_instance_custom_data(cell_index, current_cell_color);
		_cells.at(cell_index)->flags &= ~CFL_VISIBLE;
	}
}

void InteractiveGrid::InteractiveGrid::reset_cells_state() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Resets the state of all cells in the grid to their default 
           flags.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index = row * _columns + column;
			_cells.at(index)->flags = 0; // Reset.
			//set_cell_visible(index, false);
		}
	}

	_flags &= ~GFL_CELL_INACCESSIBLE_HIDDEN; // Reset.
	_flags &= ~GFL_CELL_DISTANT_HIDDEN; // Reset.

	_hovered_cell_index = -1;
	_selected_cells.clear();
}

void InteractiveGrid::set_cell_color(unsigned int cell_index, godot::Color color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the color of a specific cell in the interactive grid.

  Last Modified: October 11, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (_alpha_pass) {
		uint32_t cell_flags = _cells.at(cell_index)->flags;
		godot::Color new_cell_color{ color.r, color.g, color.b, static_cast<float>(cell_flags) };
		_cells.at(cell_index)->color = new_cell_color;
		_multimesh->set_instance_custom_data(cell_index, new_cell_color);
	} else {
		_cells.at(cell_index)->color = color;
		_multimesh->set_instance_custom_data(cell_index, color);
	}
}

void InteractiveGrid::set_obstacles_collision_masks(unsigned int masks) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the collision masks used by the interactive grid to 
           detect obstacles. These masks define which objects are 
		   considered as obstacles when checking for collisions.

  Last Modified: September 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_obstacles_collision_masks = masks;
}

int InteractiveGrid::get_obstacles_collision_masks() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the collision masks currently configured for obstacle 
           detection. These masks specify which objects are treated as 
           obstacles by the interactive grid.

  Last Modified: September 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _obstacles_collision_masks;
}

void InteractiveGrid::set_grid_floor_collision_masks(unsigned int masks) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the collision masks used by the interactive grid to 
           detect and align with scene floor (meshes).

  Last Modified: October 04, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_floor_collision_masks = masks;
}

int InteractiveGrid::get_grid_floor_collision_masks() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the collision masks currently configured for the 
           interactive grid alignment. These masks specify which floor
		   are used as references when aligning grid cells with meshes in 
		   the scene

  Last Modified: October 04, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _floor_collision_masks;
}

void InteractiveGrid::select_cell(const godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Selects a grid cell based on a world‑space position.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// If the grid isn’t visible, exit early.
	if (is_visible() == false) {
		return; // !Exit
	}

	// Retrieve the index of the cell that corresponds to the supplied
	// global position.
	int closest_index = get_cell_index_from_global_position(global_position);

	// If the index is valid.
	if (closest_index != -1) {
		// Skip invisible
		bool visible = is_cell_visible(closest_index);
		if (!visible) {
			return;
		}

		// Skip inaccessible cells.
		bool inaccessible = is_cell_inaccesible(closest_index);
		if (inaccessible) {
			return;
		}

		bool walkable = is_cell_walkable(closest_index);
		if (walkable) {
			set_cell_selected(closest_index, true);
			_selected_cells.push_back(closest_index);
		}
	}
}

godot::Array InteractiveGrid::get_selected_cells() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns an array of all cells currently marked as selected.

  Last Modified: September 26, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return _selected_cells;
}

int InteractiveGrid::get_latest_selected() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the most recently selected cell.

  Last Modified: September 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _selected_cells.back();
}

godot::PackedInt64Array InteractiveGrid::get_path(unsigned int start_cell_index,
		unsigned int target_cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Computes a path between two cells on the grid using A* 
           pathfinding.
		
           Sets up all grid points with their walkable state and 
		   configures the A* algorithm according to the selected movement
		   type (orthogonal or diagonal).

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	godot::PackedInt64Array path;

	// Register all grid points and mark obstacles.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index = row * _columns + column;

			bool walkable = is_cell_walkable(index);
			_astar->add_point(index, godot::Vector2(column, row), 1.0);
			_astar->set_point_disabled(index, !walkable);
		}
	}

	switch (_movement) {
		case MOVEMENT::FOUR_DIRECTIONS:
			configure_astar_4_dir();
			break;
		case MOVEMENT::SIX_DIRECTIONS:
			configure_astar_8_dir(); // ** tmp
			// configure_astar_6_dir(); // Hexagonal
			break;
		case MOVEMENT::EIGH_DIRECTIONS:
			configure_astar_8_dir();
			break;
	}

	path = _astar->get_id_path(start_cell_index, target_cell_index);

	return path;
}

void InteractiveGrid::set_movement(unsigned int value) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the movement type used for pathfinding on the grid.

  Last Modified: September 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_movement = value;
}

unsigned int InteractiveGrid::get_movement() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the current movement type used for pathfinding on the
           grid.

  Last Modified: September 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _movement;
}

void InteractiveGrid::configure_astar_4_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Configures the A* pathfinding graph for four directions movement.
           Each cell is connected to its four immediate neighbors (up, 
		   down, left, right) if they exist.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// Create the four-direction connections.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index = row * _columns + column;

			// Connect to the right.
			if (column + 1 < _columns) {
				int right = row * _columns + (column + 1);
				_astar->connect_points(index, right);
			}
			// Connect to the left.
			if (row + 1 < _rows) {
				int down = (row + 1) * _columns + column;
				_astar->connect_points(index, down);
			}
		}
	}
}

void InteractiveGrid::configure_astar_6_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Configures the A* pathfinding graph for six directions movement.
           Each cell is connected to its six immediate neighbors if they
		   exist.

  ref: Patel, A. J. (2013). Hexagonal grids. 
  	   https://www.redblobgames.com/grids/hexagons/#neighbors

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	// TODO
}

void InteractiveGrid::configure_astar_8_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Configures the A* pathfinding graph for eight directions 
  		   movement. Each cell is connected to all eight neighboring 
		   cells if the neighbor is walkable.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	// Create 8-direction connections.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index = row * _columns + column;

			for (int row_offset = -1; row_offset <= 1; ++row_offset) {
				for (int col_offset = -1; col_offset <= 1; ++col_offset) {
					if (col_offset == 0 && row_offset == 0)
						continue; // Do not connect to itself.

					int nx = column + col_offset;
					int ny = row + row_offset;

					if (nx >= 0 && nx < _columns && ny >= 0 && ny < _rows) {
						int neighbor_index = ny * _columns + nx;

						// Check if the neighbor is walkable before connecting.
						bool neighbor_walkable = is_cell_walkable(neighbor_index);
						if (neighbor_walkable) {
							_astar->connect_points(index, neighbor_index);
						}
					}
				}
			}
		}
	}
}

void InteractiveGrid::create() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes the grid if it has not been created yet.

  Last Modified: September 19, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(_flags & GFL_CREATED)) {
		init_multi_mesh();
		init_astar();
		layout(godot::Vector3(0, 0, 0));

		_flags |= GFL_CREATED; // Mark as created to avoid duplication.

		set_visible(false);
	}
}

void InteractiveGrid::init_multi_mesh() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes and configures the MultiMesh used for rendering 
           the grid efficiently. MultiMesh enables high-
		   performance instancing by drawing the same mesh multiple 
		   times using the GPU.
		   
  MultiMesh: "Provides high-performance drawing of a mesh multiple times
		     using GPU instancing."
		https://docs.godotengine.org/fr/4.x/classes/class_multimesh.html#

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// Create the MultiMeshInstance3D.
	_multimesh_instance = memnew(godot::MultiMeshInstance3D);
	this->add_child(_multimesh_instance);
	_multimesh.instantiate();

	_multimesh->set_transform_format(godot::MultiMesh::TRANSFORM_3D);

	// Important: enable BEFORE setting instance_count.
	_multimesh->set_use_custom_data(true);

	int cell_count = _columns * _rows;
	_multimesh->set_instance_count(cell_count);

	// Assign the MultiMesh to the instance.
	_multimesh_instance->set_multimesh(_multimesh);
	_multimesh->set_mesh(_cell_mesh);

	godot::Transform3D xform;
	xform.origin = godot::Vector3(0, 0, 0);

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index =
					row * _columns + column; // Index in the 2D array stored as 1D.

			// Position and color all cells.
			_multimesh->set_instance_transform(index, xform);

			_multimesh->set_instance_custom_data(index, _walkable_color);

			// Save the metadata.
			_cells.push_back(new Cell); // init.
			_cells.at(index)->index = index; // init.
			_cells.at(index)->local_xform = xform; // init.
			_cells.at(index)->global_xform = xform; // init.
			_cells.at(index)->flags |= CFL_WALKABLE; // init.
		}
	}

	apply_material(_material_override);
	_flags |= GFL_VISIBLE;

	PrintLine(__FILE__, __FUNCTION__, __LINE__,
			"The grid MultiMesh has been created.");
}

void InteractiveGrid::init_astar() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes the A* pathfinding instance by creating a new 
           AStar2D object. Must be called before configuring points or
		   calculating paths.

  Last Modified: September 30, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_astar.instantiate(); // Create the AStar2D instance
}

void InteractiveGrid::layout(const godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: ...

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	switch (_layout) {
		case LAYOUT::SQUARE:
			layout_cells_as_square_grid(center_position);
			break;
		case LAYOUT::HEXAGONAL:
			layout_cells_as_square_grid(center_position); // ** tmp
			//layout_cells_as_hexagonal_grid(center_position);
			break;
	}
}

void InteractiveGrid::layout_cells_as_square_grid(const godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: This method arranges the cells of the grid into a 
           square grid layout, positioning each cell relative to a pawn.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	_grid_center_position = center_position;

	// Calculate the distances between the center and the grid's edges.
	const float pawn_to_grid_edge_x = (_columns / 2) * _cell_size.x;
	const float pawn_to_grid_edge_z = (_rows / 2) * _cell_size.y;

	//  Initialize the member `grid_offset_`.
	_grid_offset.x = center_position.x - pawn_to_grid_edge_x;
	_grid_offset.z = center_position.z - pawn_to_grid_edge_z;

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index =
					row * _columns + column; // Index in the 2D array stored as 1D.

			// Calculate the cell's position.
			float cell_pos_x = _grid_offset.x + column * _cell_size.x;
			float cell_pos_y = center_position.y;
			float cell_pos_z = _grid_offset.z + row * _cell_size.y;
			godot::Vector3 cell_pos(cell_pos_x, cell_pos_y, cell_pos_z);

			// Apply the position (global, not local).
			godot::Transform3D global_xform =
					_multimesh_instance->get_global_transform();
			godot::Transform3D local_xform =
					global_xform.affine_inverse(); // Inverse du global.

			// Convert the global position to local:
			godot::Vector3 local_pos = local_xform.xform(cell_pos);

			// Then, apply the local position.
			godot::Transform3D xform;
			xform.origin = local_pos;

			_multimesh->set_instance_transform(index, xform);

			// Save cell's metadata.
			_cells.at(index)->local_xform =
					_multimesh->get_instance_transform(index);
			_cells.at(index)->global_xform =
					_multimesh_instance->get_global_transform() *
					_multimesh->get_instance_transform(index);

			set_cell_visible(index, false);
		}
	}

	PrintLine(__FILE__, __FUNCTION__, __LINE__,
			"The grid cells have been laid out as a square grid.");
}

void InteractiveGrid::layout_cells_as_hexagonal_grid(const godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: This method arranges the cells of the grid into a 
           hexagonal grid layout, positioning each cell relative to a pawn.

  ref : jmbiv. (2021, October 5). How to make a 3D hexagon grid in Godot
        (Tutorial) [Video]. YouTube. 
		https://www.youtube.com/watch?v=3Lt2TfP8WEw

        16:00 "building collumns in our grid"

  		Patel, A. J. (2013). Hexagonal grids. 
  		https://www.redblobgames.com/grids/hexagons/#neighbors

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	// TODO
}

void InteractiveGrid::align_cells_with_floor() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Aligns each grid cell with the underlying floor
        using a vertical downward raycast.
        The ray starts above the cell and checks for a collision with
        an object on the same layer. If a collision is detected,
        the cell is repositioned and reoriented to match the
        hit surface (floor normal). Cells are not aligned with
        invisible objects.

  Ref : BornCG. (2024, August 4). Godot 4 3D Platformer Lesson #13: 
  		Align Player with Ground! [Video]. YouTube.
		https://www.youtube.com/watch?v=Y5OiChOukfg

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (_flags & GFL_CREATED) {
		// Maximum raycast length.
		const int ray_length{ 500 };

		// Global transform of the MultiMeshInstance (position/rotation/scale in
		// world space).
		const godot::Transform3D global_transform =
				_multimesh_instance->get_global_transform();

		// Affine inverse: allows converting global coordinates into the local
		// space.
		const godot::Transform3D global_to_local =
				global_transform.affine_inverse();

		// Iterate through the cells.
		for (int row = 0; row < _rows; row++) {
			for (int column = 0; column < _columns; column++) {
				const int index =
						row * _columns + column; // Index in the 2D array stored as 1D.

				/*--------------------------------------------------------------------
         Initialization of the starting coordinates and the ray.
        --------------------------------------------------------------------*/

				// Local origin of the cell (in the grid's local space).
				godot::Vector3 local_from = _cells.at(index)->local_xform.origin;

				// Global position of the cell (in the 3D world).
				godot::Vector3 global_from = _cells.at(index)->global_xform.origin;

				// Raises the raycast starting point to ensure it begins above the cell.
				global_from.y += 100.0f;

				// Raycast end point: 500 units below the starting point.
				godot::Vector3 global_to =
						global_from - godot::Vector3(0, ray_length, 0);

				// Retrieves the 3D physics space of the scene (for performing physics
				// queries).
				godot::Ref<godot::World3D> world = get_world_3d();
				godot::PhysicsDirectSpaceState3D *space_state =
						world->get_direct_space_state();

				// Sets up the parameters for the raycast query.
				godot::Ref<godot::PhysicsRayQueryParameters3D> ray_query;
				ray_query.instantiate();
				ray_query->set_collide_with_areas(false); // Ignores Area3D nodes.
				ray_query->set_from(global_from); // Starting point of the ray.
				ray_query->set_to(global_to); // End point of the ray.

				ray_query->set_collision_mask(_floor_collision_masks);

				// Excludes the MultiMesh to prevent it from blocking its own ray.
				godot::TypedArray<godot::RID> exclude_array;
				exclude_array.append(_multimesh->get_rid());
				ray_query->set_exclude(exclude_array);

				// Executes the raycast and retrieves the result.
				godot::Dictionary result = space_state->intersect_ray(ray_query);

				/*--------------------------------------------------------------------
          Checks the validity of the hit mesh (ignores invisible objects).
        --------------------------------------------------------------------*/

				if (!result.is_empty()) {
					// Retrieves the collided object.
					godot::Object *collider_obj =
							Object::cast_to<godot::Object>(result["collider"]);

					// Checks if a valid object was found.
					if (collider_obj) {
						// Ignores the collision if the mesh is invisible in the scene tree.
						godot::Node3D *collider_node =
								Object::cast_to<godot::Node3D>(collider_obj);

						// Skips the collision if the mesh is invisible in the scene tree.
						if (collider_node && !collider_node->is_visible_in_tree()) {
							continue; // Passe à la cellule suivante.
						}
					}

					/*--------------------------------------------------------------------
            Aligns the cell with the detected floor.
          --------------------------------------------------------------------*/

					// Global position of the hit point.
					godot::Vector3 hit_position_global = result["position"];

					// Surface normal at the hit point (used to correct orientation).
					godot::Vector3 floor_normal = result["normal"];

					// Converts the hit position from global to local coordinates.
					godot::Vector3 hit_position_local =
							global_to_local.xform(hit_position_global);

					// Creates a new transform with the origin positioned on the floor.
					godot::Transform3D xform;
					xform.origin = hit_position_local;

					// Aligns the Y axis with the floor normal.
					xform.basis.set_column(
							1, floor_normal.normalized()); // Y = floor normal.

					// Recalculates the X and Z axes to obtain an orthogonal basis.
					godot::Vector3 basis_z = xform.basis.get_column(2);
					godot::Vector3 basis_x = floor_normal.cross(basis_z).normalized();
					xform.basis.set_column(0, basis_x); // X = cross product of Y and Z.

					basis_z = basis_x.cross(floor_normal).normalized();
					xform.basis.set_column(2, basis_z); // Corrected Z axis.
					xform.basis =
							xform.basis.orthonormalized(); // Orthonormalizes to prevent
					// numerical errors.
					_multimesh->set_instance_transform(index, xform);

					// Updates the instance transform in the MultiMesh.
					_cells.at(index)->local_xform = xform;
					_cells.at(index)->global_xform =
							_multimesh_instance->get_global_transform() *
							_multimesh->get_instance_transform(index);

					set_cell_walkable(index, true);
					set_cell_visible(index, true);

					// Optional debug:
					// godot::print_line("New transformation of the cell: ", xform);
				} else {
					set_cell_in_void(index, true);
				}
			}
		}

		PrintLine(__FILE__, __FUNCTION__, __LINE__,
				"Grid cells have been aligned with the floor surface.");
	}
}

void InteractiveGrid::scan_environnement_obstacles() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Scans the game grid to detect obstacles and updates the 
           corresponding grid cells as walkable or unwalkable. For each 
		   cell in the grid, a physics query is performed using a box 
		   shape representing the cell. The query checks for collisions 
		   with objects on specific layers.Cells with collisions are 
		   marked as invalid (unwalkable), while cells without collisions 
		   are marked as valid (walkable). Debug logs are generated to 
		   provide information about the collision results.


  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(_flags & GFL_VISIBLE)) {
		return;
	}

	// Retrieve the physics interface (PhysicsDirectSpaceState3D) of the current
	// world, which allows performing collision queries.
	godot::PhysicsDirectSpaceState3D *space_state =
			get_world_3d()->get_direct_space_state();

	if (!space_state) {
		PrintError(__FILE__, __FUNCTION__, __LINE__,
				"No PhysicsDirectSpaceState3D available.");
		return;
	}

	// Prepare the shape if it has not been created yet.
	if (_obstacle_shape.is_null()) {
		_obstacle_shape.instantiate();
		_obstacle_shape->set_size(godot::Vector3(_cell_size.x, 1.0, _cell_size.y));
	}

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			// Calculates the cell index.
			const int index = row * _columns + column;
			// Retrieves the position of the cell.
			const godot::Vector3 cell_pos = _cells.at(index)->global_xform.origin;

			// Configure a physics query for collision detection.
			godot::Ref<godot::PhysicsShapeQueryParameters3D> query;

			// Create a new PhysicsShapeQueryParameters3D instance.
			query.instantiate();

			// Assign the shape to be tested (here: the box shape representing a grid
			// cell).
			query->set_shape(_obstacle_shape);

			// Place the shape in the world at the current grid cell position (no
			// rotation applied).
			query->set_transform(godot::Transform3D(godot::Basis(), cell_pos));

			// Define which collision layers will be considered by this query.

			query->set_collision_mask(_obstacles_collision_masks);

			// Enable collision checks with PhysicsBody3D (e.g., walls, obstacles,
			// characters).
			query->set_collide_with_bodies(true);

			// Disable collision checks with Area3D to avoid detecting sensor-only
			// volumes.
			query->set_collide_with_areas(false);

			// Perform the physics query: check which objects intersect the given
			// shape. Returns up to 32 results, each stored as a Dictionary (with keys
			// like "collider", "rid", "shape", etc.).
			godot::TypedArray<godot::Dictionary> results =
					space_state->intersect_shape(query, 32);

			// If there are any results from the collision query.
			if (results.size() > 0) {
				// Debug log: reports the detected collision along with the cell index
				// and its grid coordinates.

				// ** Debug logs.
				// PrintLine(__FILE__, __FUNCTION__, __LINE__,
				// 		"[GridScan] Collision detected at cell index " +
				// 				godot::String::num_int64(index) +
				// 				" (row: " + godot::String::num_int64(i) +
				// 				", column: " + godot::String::num_int64(j) + ")");

				// Iterate over each collision result returned by the physics query.
				for (int k = 0; k < results.size(); k++) {
					// Retrieve the k-th result as a Dictionary.
					godot::Dictionary hit = results[k];

					// Extract the 'collider' object from the result.
					godot::Object *collider_obj = hit["collider"];

					// Attempt to cast the collider to a Node, since all objects inherit
					// from Node.
					godot::Node *collider =
							godot::Object::cast_to<godot::Node>(collider_obj);

					if (collider) {
						// Log the detected collision, showing the node's name and its
						// class.

						// ** Debug logs.
						// PrintLine(__FILE__, __FUNCTION__, __LINE__,
						// 		"[GridScan] Collision -> Node: " + collider->get_name() +
						// 				" (Class: " + collider->get_class() + ")");

						// Mark the grid cell as invalid (obstructed).

						/*
							Prevent cells that are above empty space and touching an obstacle
							from being displayed.
						*/
						bool is_in_void = is_cell_in_void(index);

						if (!is_in_void) {
							set_cell_walkable(index, false);
						}
					}
				}
			}
		}
	}

	PrintLine(__FILE__, __FUNCTION__, __LINE__,
			"[GridScan] ScanEnvironnementObstacles() completed.");
}

void InteractiveGrid::apply_material(const godot::Ref<godot::Material> &p_material) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Applies the supplied material as an override to the grid’s
           MultiMeshInstance.

  Last Modified: October 05, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (_multimesh_instance == nullptr) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	if (p_material.is_null()) {
		// No material provided; clearing existing material override and applying default material.
		_multimesh_instance->set_material_override(nullptr);
		apply_default_material();
		return;
	} else {
		_multimesh_instance->set_material_override(p_material);
	}
}

void InteractiveGrid::set_cells_visible(bool visible_param) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Toggles the visual visibility of every cell in the grid.

  Last Modified: October 09, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	int cell_count = _multimesh->get_instance_count();

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index =
					row * _columns + column; // Index in the 2D array stored as 1D.

			if (visible_param == true) {
				_multimesh->set_instance_custom_data(index, _walkable_color); // Visible.
			} else {
				_multimesh->set_instance_custom_data(
						index, godot::Color(0.0, 0.0, 0.0, 0.0)); // Invisible.
			}
		}
	}

	apply_material(_material_override);
}

void InteractiveGrid::set_cell_inaccesible(unsigned int cell_index, bool is_inaccesible) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a given grid cell (cell_index) is inaccessible.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_inaccesible) {
		_cells.at(cell_index)->flags |= CFL_INACCESSIBLE;
		set_cell_color(cell_index, _inaccessible_color);
	} else if (!is_inaccesible) {
		_cells.at(cell_index)->flags &= ~CFL_INACCESSIBLE;
	}
}

void InteractiveGrid::set_cell_in_void(unsigned int cell_index, bool is_in_void) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Marks a cell as being "in void" or not. If a cell is in void,
	       it is hidden and flagged accordingly. Used to prevent cells
	       above empty space or near obstacles from being displayed.

  Last Modified: October 10, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_in_void) {
		_cells.at(cell_index)->flags |= CFL_IN_VOID;
		set_cell_visible(cell_index, false);
	} else if (!is_in_void) {
		_cells.at(cell_index)->flags &= ~CFL_IN_VOID;
	}
}

void InteractiveGrid::set_cell_hovered(unsigned int cell_index, bool is_hovered) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a particular grid cell (cell_index) is hovered.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_hovered) {
		_cells.at(cell_index)->flags |= CFL_HOVERED;
		set_cell_color(_hovered_cell_index, _hovered_color);
	} else if (!is_hovered) {
		_cells.at(cell_index)->flags &= ~CFL_HOVERED;
	}
}

void InteractiveGrid::set_cell_selected(unsigned int cell_index, bool is_selected) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific grid cell (cell_index) is marked as 
           selected.

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_selected) {
		_cells.at(cell_index)->flags |= CFL_SELECTED;
		set_cell_color(cell_index, _selected_color);
	} else if (!is_selected) {
		_cells.at(cell_index)->flags &= ~CFL_SELECTED;
	}
}

void InteractiveGrid::set_cell_on_path(unsigned int cell_index, bool is_on_path) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific grid cell (cell_index) is part of the 
           current path.

  Last Modified: October 11, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // ! Exit.
	}

	if (is_on_path) {
		_cells.at(cell_index)->flags |= CFL_PATH;
		set_cell_color(cell_index, _path_color);
	} else if (!is_on_path) {
		_cells.at(cell_index)->flags &= ~CFL_PATH;
	}
}

int InteractiveGrid::get_cell_index_from_global_position(
		const godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the linear index of the grid cell that is closest to
		   the supplied world‑space position.

  Last Modified: September 26, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(_flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__,
				"The grid has not been created.");
		return -1;
	}

	if (!_multimesh.is_valid()) {
		PrintError(__FILE__, __FUNCTION__, __LINE__,
				"The grid multimesh is not valid.");
		return -1;
	}

	// Calculate the distances between the center and the grid's edges.
	const float center_edge_x =
			((_columns / 2) * _cell_size.x) + (_cell_size.x / 2);
	const float center_edge_z = ((_rows / 2) * _cell_size.y) + (_cell_size.y / 2);

	//  Initialize the member `grid_offset_`.
	_grid_offset.x = _grid_center_position.x - center_edge_x;
	_grid_offset.z = _grid_center_position.z - center_edge_z;

	if (global_position.x > (_grid_center_position.x + center_edge_x) ||
			global_position.x < (_grid_center_position.x - center_edge_x)) {
		return -1;
	}

	if (global_position.z > (_grid_center_position.z + center_edge_z) ||
			global_position.z < ((_grid_center_position.z - center_edge_z))) {
		return -1;
	}

	float closest_distance = std::numeric_limits<float>::max();
	int closest_index = -1;

	// Iterate through the cells.
	for (int row = 0; row < _rows; row++) {
		for (int column = 0; column < _columns; column++) {
			const int index =
					row * _columns + column; // Index in the 2D array stored as 1D.

			const godot::Vector3 cell_pos = _cells.at(index)->global_xform.origin;
			const float distance = global_position.distance_to(cell_pos);

			if (distance < closest_distance) {
				closest_distance = distance;
				closest_index = index;
			}
		}
	}

	return closest_index;
}

bool InteractiveGrid::is_cell_index_out_of_bounds(const godot::String &file,
		const godot::String &func, const int &line, const unsigned int &cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: 

  Last Modified: October 07, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	bool is_out_of_bounds = false;

	if (cell_index >= (_rows * _columns)) {
		PrintError(file, func, line, "Cell index out of bounds.");
		is_out_of_bounds = true;
	}

	return is_out_of_bounds;
}