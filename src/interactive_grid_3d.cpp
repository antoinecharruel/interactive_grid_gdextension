/*+===================================================================
File: interactive_grid.cpp

Summary: InteractiveGrid3D is a Godot 4.5 GDExtension that allows player
         interaction with a 3D grid, including cell selection,
		 pathfinding, and hover highlights.

Last Modified: November 28, 2025

This file is part of the InteractiveGrid3D GDExtension Source Code.
Repository: https://github.com/antoinecharruel/interactive_grid

Version InteractiveGrid3D: 1.6.0
Version: Godot Engine v4.5.stable.steam - https://godotengine.org

Author: Antoine Charruel
===================================================================+*/

#include "interactive_grid_3d.h"

void InteractiveGrid3D::_create() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes the grid if it has not been created yet.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(data.flags & GFL_CREATED)) {
		data.center_global_position = get_global_transform().origin;

		_init_multi_mesh();
		_init_astar();

		data.flags |= GFL_CREATED;

		center(data.center_global_position);

		if (godot::Engine::get_singleton()->is_editor_hint()) {
			set_visible(true);
		} else {
			set_visible(false);
		}
	}
}

void InteractiveGrid3D::_delete() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Frees all grid resources and resets state.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (data.flags & GFL_CREATED) {
		// Delete cells
		for (Cell *c : data.cells) {
			delete c;
		}
		data.cells.clear();

		// Delete multimesh
		if (data.multimesh_instance) {
			data.multimesh_instance->queue_free();
			data.multimesh_instance = nullptr;
		}

		// Reset AStar2D
		data.astar = godot::Ref<godot::AStar2D>();

		data.flags &= ~GFL_CREATED;
	}
}

void InteractiveGrid3D::_init_multi_mesh() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes and configures the MultiMesh used for rendering 
           the grid efficiently. MultiMesh enables high-
		   performance instancing by drawing the same mesh multiple 
		   times using the GPU.
		   
  MultiMesh: "Provides high-performance drawing of a mesh multiple times
		     using GPU instancing."
		https://docs.godotengine.org/fr/4.x/classes/class_multimesh.html#
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// Create the MultiMeshInstance3D
	data.multimesh_instance = memnew(godot::MultiMeshInstance3D);
	this->add_child(data.multimesh_instance);
	data.multimesh.instantiate();

	data.multimesh->set_transform_format(godot::MultiMesh::TRANSFORM_3D);
	data.multimesh->set_use_custom_data(true);

	int cell_count = data.columns * data.rows;
	data.multimesh->set_instance_count(cell_count);

	// Assign the MultiMesh to the instance
	data.multimesh_instance->set_multimesh(data.multimesh);
	data.multimesh->set_mesh(data.cell_mesh);

	godot::Transform3D xform;
	xform.origin = godot::Vector3(0, 0, 0);

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index =
					row * data.columns + column;

			// Position and color all cells
			data.multimesh->set_instance_transform(index, xform);
			data.multimesh->set_instance_custom_data(index, data.walkable_color);

			// Save the metadata
			data.cells.push_back(new Cell);
			data.cells.at(index)->index = index;
			data.cells.at(index)->local_xform = xform;
			data.cells.at(index)->global_xform = xform;
		}
	}

	_apply_material(data.material_override);
	data.flags |= GFL_VISIBLE;

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid MultiMesh has been created.");
	}
}

void InteractiveGrid3D::_init_astar() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Initializes the A* pathfinding instance by creating a new 
           AStar2D object. Must be called before configuring points or
		   calculating paths.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.astar.instantiate();
}

void InteractiveGrid3D::_layout(godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Positions the cells around the center according to the 
           selected layout.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit
	}

	switch (data.layout_index) {
		case LAYOUT::SQUARE:
			_layout_cells_as_square_grid(center_position);
			break;
		case LAYOUT::HEXAGONAL:
			_layout_cells_as_hexagonal_grid(center_position);
			break;
	}
}

void InteractiveGrid3D::_layout_cells_as_square_grid(godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: This method arranges the cells of the grid into a 
           square grid layout, positioning each cell relative to a 
		   center.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.center_global_position = center_position;

	// Calculate the distances between the center and the grid's edges
	const float center_to_grid_edge_x = (data.columns / 2) * data.cell_size.x;
	const float center_to_grid_edge_z = (data.rows / 2) * data.cell_size.y;

	//  Initialize the member `grid_offset_`
	data.top_left_global_position.x = center_position.x - center_to_grid_edge_x;
	data.top_left_global_position.z = center_position.z - center_to_grid_edge_z;

	// Iterate through the cells
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column; // Index in the 2D array stored as 1D

			// Calculate the cell's position
			float cell_pos_x = data.top_left_global_position.x + column * data.cell_size.x;
			float cell_pos_y = center_position.y;
			float cell_pos_z = data.top_left_global_position.z + row * data.cell_size.y;
			godot::Vector3 cell_pos(cell_pos_x, cell_pos_y, cell_pos_z);

			// Apply the position (global, not local)
			godot::Transform3D global_xform = data.multimesh_instance->get_global_transform();
			godot::Transform3D local_xform = global_xform.affine_inverse(); // Inverse du global

			// Convert the global position to local:
			godot::Vector3 local_pos = local_xform.xform(cell_pos);

			// Then, apply the local position
			godot::Transform3D xform;
			xform.origin = local_pos;

			data.multimesh->set_instance_transform(index, xform);

			// Save cell's metadata
			data.cells.at(index)->local_xform = data.multimesh->get_instance_transform(index);
			data.cells.at(index)->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

			set_cell_visible(index, false);
		}
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid cells have been laid out as a square grid.");
	}
}

void InteractiveGrid3D::_layout_cells_as_hexagonal_grid(godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: This method arranges the cells of the grid into a 
           hexagonal grid layout, positioning each cell relative to a 
		   center.

  ref : jmbiv. (2021, October 5). How to make a 3D hexagon grid in Godot
        (Tutorial) [Video]. YouTube. 
		https://www.youtube.com/watch?v=3Lt2TfP8WEw

        16:00 "building columns in our grid"

  		Patel, A. J. (2013). Hexagonal grids. 
  		https://www.redblobgames.com/grids/hexagons/#neighbors

		https://www.gigacalculator.com/calculators/hexagon-calculator.php
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	data.center_global_position = center_position;

	// The short diagonal (s) can be calculated using the formula: s = a · √3
	const float hex_short_diagonal = data.cell_size.x; // s

	// a = s / √3
	const float hex_side_length = hex_short_diagonal / sqrt(3); // a

	// r = a · √3 / 2
	const float hex_inradius = hex_side_length * sqrt(3) / 2;

	// Calculate the distances between the center and the grid's edges.
	float center_to_grid_edge_x = (data.columns / 2) * data.cell_size.x;
	float center_to_grid_edge_z = (data.rows / 2) * data.cell_size.y;

	// Z-AXIS CORRECTION.
	if (!(data.rows % 2)) {
		center_to_grid_edge_z -= hex_side_length;
	}

	// X-AXIS CORRECTION.
	if (!(data.columns % 2)) {
		center_to_grid_edge_x -= data.cell_size.x / 2; // Side to side.
	}

	// Initialize the member `grid_offset_`.
	data.top_left_global_position.x = center_position.x - center_to_grid_edge_x - hex_inradius;
	data.top_left_global_position.z = center_position.z - center_to_grid_edge_z - hex_side_length;

	// Iterate through the cells.
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column; // Index in the 2D array stored as 1D.

			// Compute columns.
			float cell_pos_x{ 0.0f };

			bool is_even_row = (row % 2) == 0;

			if (is_even_row) {
				cell_pos_x = data.top_left_global_position.x + (column * data.cell_size.x);
			} else {
				cell_pos_x = data.top_left_global_position.x + (column * data.cell_size.x) + (data.cell_size.x / 2);
			}

			// Apply final position.
			float cell_pos_y = center_position.y;
			float cell_pos_z = data.top_left_global_position.z + (row * data.cell_size.y) + hex_side_length;
			godot::Vector3 cell_pos(cell_pos_x, cell_pos_y, cell_pos_z);

			// Apply the position (global, not local).
			godot::Transform3D global_xform = data.multimesh_instance->get_global_transform();
			godot::Transform3D local_xform = global_xform.affine_inverse(); // Inverse du global

			// Convert the global position to local:
			godot::Vector3 local_pos = local_xform.xform(cell_pos);

			// Then, apply the local position.
			godot::Transform3D xform;
			xform.origin = local_pos;

			data.multimesh->set_instance_transform(index, xform);

			// Save cell's metadata.
			data.cells.at(index)->local_xform = data.multimesh->get_instance_transform(index);
			data.cells.at(index)->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

			set_cell_visible(index, false);
		}
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid cells have been laid out as a hexagonal grid.");
	}
}

void InteractiveGrid3D::_align_cells_with_floor() {
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
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (data.flags & GFL_CREATED) {
		auto start = std::chrono::high_resolution_clock::now();

		// Maximum raycast length
		const int ray_length{ 500 };

		// Global transform of the MultiMeshInstance (position/rotation/scale in
		// world space)
		const godot::Transform3D global_transform = data.multimesh_instance->get_global_transform();

		// Affine inverse: allows converting global coordinates into the local
		// space
		const godot::Transform3D global_to_local = global_transform.affine_inverse();

		// Iterate through the cells
		for (int row = 0; row < data.rows; row++) {
			for (int column = 0; column < data.columns; column++) {
				const int index =
						row * data.columns + column; // Index in the 2D array stored as 1D

				/*--------------------------------------------------------------------
         Initialization of the starting coordinates and the ray
        --------------------------------------------------------------------*/

				// Local origin of the cell (in the grid's local space)
				godot::Vector3 local_from = data.cells.at(index)->local_xform.origin;

				// Global position of the cell (in the 3D world).
				godot::Vector3 global_from = data.cells.at(index)->global_xform.origin;

				// Raises the raycast starting point to ensure it begins above the cell
				global_from.y += 100.0f;

				// Raycast end point: 500 units below the starting point
				godot::Vector3 global_to =
						global_from - godot::Vector3(0, ray_length, 0);

				// Retrieves the 3D physics space of the scene (for performing physics queries)
				godot::Ref<godot::World3D> world = get_world_3d();
				godot::PhysicsDirectSpaceState3D *space_state = world->get_direct_space_state();

				// Sets up the parameters for the raycast query
				godot::Ref<godot::PhysicsRayQueryParameters3D> ray_query;
				ray_query.instantiate();
				ray_query->set_collide_with_areas(false); // Ignores Area3D nodes
				ray_query->set_from(global_from); // Starting point of the ray
				ray_query->set_to(global_to); // End point of the ray

				ray_query->set_collision_mask(data.floor_collision_masks);

				// Excludes the MultiMesh to prevent it from blocking its own ray
				godot::TypedArray<godot::RID> exclude_array;
				exclude_array.append(data.multimesh->get_rid());
				ray_query->set_exclude(exclude_array);

				// Executes the raycast and retrieves the result
				godot::Dictionary result = space_state->intersect_ray(ray_query);

				/*--------------------------------------------------------------------
          Checks the validity of the hit mesh (ignores invisible objects)
        --------------------------------------------------------------------*/

				if (!result.is_empty()) {
					// Retrieves the collided object
					godot::Object *collider_obj = Object::cast_to<godot::Object>(result["collider"]);

					// Checks if a valid object was found
					if (collider_obj) {
						// Ignores the collision if the mesh is invisible in the scene tree
						godot::Node3D *collider_node = Object::cast_to<godot::Node3D>(collider_obj);

						// Skips the collision if the mesh is invisible in the scene tree
						if (collider_node && !collider_node->is_visible_in_tree()) {
							continue; // Passe à la cellule suivante
						}
					}

					/*--------------------------------------------------------------------
            Aligns the cell with the detected floor
          --------------------------------------------------------------------*/

					// Global position of the hit point
					godot::Vector3 hit_position_global = result["position"];

					// Surface normal at the hit point (used to correct orientation)
					godot::Vector3 floor_normal = result["normal"];

					// Converts the hit position from global to local coordinates
					godot::Vector3 hit_position_local = global_to_local.xform(hit_position_global);

					// Creates a new transform with the origin positioned on the floor
					godot::Transform3D xform;
					xform.origin = hit_position_local;

					// Aligns the Y axis with the floor normal
					xform.basis.set_column(1, floor_normal.normalized()); // Y = floor normal

					// Recalculates the X and Z axes to obtain an orthogonal basis
					godot::Vector3 basis_z = xform.basis.get_column(2);
					godot::Vector3 basis_x = floor_normal.cross(basis_z).normalized();
					xform.basis.set_column(0, basis_x); // X = cross product of Y and Z

					basis_z = basis_x.cross(floor_normal).normalized();
					xform.basis.set_column(2, basis_z); // Corrected Z axis
					xform.basis = xform.basis.orthonormalized(); // Orthonormalizes to prevent
					// numerical errors.
					data.multimesh->set_instance_transform(index, xform);

					// Updates the instance transform in the MultiMesh
					data.cells.at(index)->local_xform = xform;
					data.cells.at(index)->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

					set_cell_walkable(index, true);
					set_cell_reachable(index, true);
					set_cell_visible(index, true);

					// Optional debug:
					// godot::print_line("New transformation of the cell: ", xform);
				} else if (!godot::Engine::get_singleton()->is_editor_hint()) {
					// In game
					_set_cell_in_void(index, true);
				} else {
					// In editor
					set_cell_walkable(index, true);
					set_cell_reachable(index, true);
					set_cell_visible(index, true);
				}
			}
		}

		auto end = std::chrono::high_resolution_clock::now();

		if (_debug_options.print_execution_time_enabled) {
			std::chrono::duration<double, std::milli> duration = end - start;
			PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
		}

		if (_debug_options.print_logs_enabled) {
			PrintLine(__FILE__, __FUNCTION__, __LINE__, "Grid cells have been aligned with the floor surface.");
		}
	}
}

void InteractiveGrid3D::_scan_environnement_obstacles() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Scans the game grid to detect obstacles and updates the 
           corresponding grid cells as walkable or unwalkable. For each 
		   cell in the grid, a physics query is performed using a box 
		   shape representing the cell. The query checks for collisions 
		   with objects on specific layers. Cells with collisions are 
		   marked as invalid (unwalkable), while cells without collisions 
		   are marked as valid (walkable). Debug logs are generated to 
		   provide information about the collision results.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(data.flags & GFL_VISIBLE)) {
		return;
	}

	// Retrieve the physics interface (PhysicsDirectSpaceState3D) of the current
	// world, which allows performing collision queries
	godot::PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();

	if (!space_state) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No PhysicsDirectSpaceState3D available.");
		return;
	}

	// Prepare the shape if it has not been created yet
	if (data.obstacle_shape.is_null()) {
		data.obstacle_shape.instantiate();
		data.obstacle_shape->set_size(godot::Vector3(data.cell_size.x, 1.0, data.cell_size.y));
	}

	auto start = std::chrono::high_resolution_clock::now();

	// Iterate through the cells
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			// Calculates the cell index
			const int index = row * data.columns + column;
			// Retrieves the position of the cell
			const godot::Vector3 cell_pos = data.cells.at(index)->global_xform.origin;

			// Configure a physics query for collision detection
			godot::Ref<godot::PhysicsShapeQueryParameters3D> query;

			// Create a new PhysicsShapeQueryParameters3D instance
			query.instantiate();

			// Assign the shape to be tested (here: the box shape representing a grid cell)
			query->set_shape(data.obstacle_shape);

			// Place the shape in the world at the current grid cell position (no rotation applied)
			query->set_transform(godot::Transform3D(godot::Basis(), cell_pos));

			// Define which collision layers will be considered by this query
			query->set_collision_mask(data.obstacles_collision_masks);

			// Enable collision checks with PhysicsBody3D (e.g., walls, obstacles, characters)
			query->set_collide_with_bodies(true);

			// Disable collision checks with Area3D to avoid detecting sensor-only volumes
			query->set_collide_with_areas(false);

			// Perform the physics query: check which objects intersect the given
			// shape. Returns up to 32 results, each stored as a Dictionary
			godot::TypedArray<godot::Dictionary> results = space_state->intersect_shape(query, 32);

			// If there are any results from the collision query
			if (results.size() > 0) {
				// Debug log: reports the detected collision along with the cell index
				// and its grid coordinates

				// ** Debug logs.
				// PrintLine(__FILE__, __FUNCTION__, __LINE__,
				// 		"[GridScan] Collision detected at cell index " +
				// 				godot::String::num_int64(index) +
				// 				" (row: " + godot::String::num_int64(i) +
				// 				", column: " + godot::String::num_int64(j) + ")");

				// Iterate over each collision result returned by the physics query
				for (int k = 0; k < results.size(); k++) {
					// Retrieve the k-th result as a Dictionary
					godot::Dictionary hit = results[k];

					// Extract the 'collider' object from the result
					godot::Object *collider_obj = hit["collider"];

					// Attempt to cast the collider to a Node, since all objects inherit
					// from Node
					godot::Node *collider =
							godot::Object::cast_to<godot::Node>(collider_obj);

					if (collider) {
						// Log the detected collision, showing the node's name and its
						// class

						// ** Debug logs.
						// PrintLine(__FILE__, __FUNCTION__, __LINE__,
						// 		"[GridScan] Collision -> Node: " + collider->get_name() +
						// 				" (Class: " + collider->get_class() + ")");

						// Mark the grid cell as invalid (obstructed)

						/*
							Prevent cells that are above empty space and touching an obstacle
							from being displayed
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

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Scan complete.");
	}
}

void InteractiveGrid3D::_apply_material(const godot::Ref<godot::Material> &p_material) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Applies the supplied material as an override to the grid’s
           MultiMeshInstance
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (data.multimesh_instance == nullptr) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	if (p_material.is_null()) {
		// No material provided; clearing existing material override and applying default material
		data.multimesh_instance->set_material_override(nullptr);
		apply_default_material();
		return;
	} else {
		data.multimesh_instance->set_material_override(p_material);
	}
}

void InteractiveGrid3D::_set_cells_visible(bool visible) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Toggles the visual visibility of every cell in the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	int cell_count = data.multimesh->get_instance_count();

	// Iterate through the cells
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index =
					row * data.columns + column;

			if (visible == true) {
				data.multimesh->set_instance_custom_data(index, data.walkable_color); // Visible
			} else {
				data.multimesh->set_instance_custom_data(index, godot::Color(0.0, 0.0, 0.0, 0.0)); // Invisible
			}
		}
	}

	_apply_material(data.material_override);
}

void InteractiveGrid3D::_set_cell_in_void(unsigned int cell_index, bool is_in_void) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Marks a cell as being "in void" or not. If a cell is in void,
	       it is hidden and flagged accordingly. Used to prevent cells
	       above empty space.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_in_void) {
		data.cells.at(cell_index)->flags |= CFL_IN_VOID;
		set_cell_visible(cell_index, false);
	} else if (!is_in_void) {
		data.cells.at(cell_index)->flags &= ~CFL_IN_VOID;
	}
}

void InteractiveGrid3D::_set_cell_hovered(unsigned int cell_index, bool is_hovered) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a particular grid cell (cell_index) is hovered.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_hovered) {
		data.cells.at(cell_index)->flags |= CFL_HOVERED;
		set_cell_color(data.hovered_cell_index, data.hovered_color);
	} else if (!is_hovered) {
		data.cells.at(cell_index)->flags &= ~CFL_HOVERED;
	}
}

void InteractiveGrid3D::_set_cell_selected(unsigned int cell_index, bool is_selected) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific grid cell (cell_index) is marked as 
           selected.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_selected) {
		data.cells.at(cell_index)->flags |= CFL_SELECTED;
		set_cell_color(cell_index, data.selected_color);
	} else if (!is_selected) {
		data.cells.at(cell_index)->flags &= ~CFL_SELECTED;
	}
}

void InteractiveGrid3D::_set_cell_on_path(unsigned int cell_index, bool is_on_path) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific grid cell (cell_index) is part of the 
           current path.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_on_path) {
		data.cells.at(cell_index)->flags |= CFL_PATH;
		set_cell_color(cell_index, data.path_color);
	} else if (!is_on_path) {
		data.cells.at(cell_index)->flags &= ~CFL_PATH;
	}
}

void InteractiveGrid3D::_configure_astar() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: // TODO
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return; // ! Exit
	}

	auto start = std::chrono::high_resolution_clock::now();

	data.astar->clear();

	// Register all grid points and mark obstacles
	for (int index = 0; index < data.cells.size(); ++index) {
		int x = index % data.columns;
		int y = index / data.columns;
		data.astar->add_point(index, godot::Vector2(x, y), 1.0);
		data.astar->set_point_disabled(index, !is_cell_walkable(index));
	}

	switch (data.movement) {
		case MOVEMENT::FOUR_DIRECTIONS:
			_configure_astar_4_dir();
			break;
		case MOVEMENT::SIX_DIRECTIONS:
			_configure_astar_6_dir(); // Hexagonal
			break;
		case MOVEMENT::EIGH_DIRECTIONS:
			_configure_astar_8_dir();
			break;
	}

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}
}

void InteractiveGrid3D::_configure_astar_4_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Configures the A* pathfinding graph for four directions movement.
           Each cell is connected to its four immediate neighbors (up, 
		   down, left, right) if they exist.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			// Connect to the right
			if (column + 1 < data.columns) {
				int right = row * data.columns + (column + 1);
				data.astar->connect_points(index, right);
				data.cells.at(index)->neighbors.push_back(right);
			}

			// Connect to the left
			if (column - 1 >= 0) {
				int left = row * data.columns + (column - 1);
				//_astar->connect_points(index, left);
				data.cells.at(index)->neighbors.push_back(left);
			}

			// Connect to the down
			if (row + 1 < data.rows) {
				int down = (row + 1) * data.columns + column;
				data.astar->connect_points(index, down);
				data.cells.at(index)->neighbors.push_back(down);
			}

			// Connect to the up
			if (row - 1 >= 0) {
				int up = (row - 1) * data.columns + column;
				//_astar->connect_points(index, up);
				data.cells.at(index)->neighbors.push_back(up);
			}
		}
	}
}

void InteractiveGrid3D::_configure_astar_6_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Summary: Configures the A* pathfinding graph for six directions 
	           movement (hexagonal grid). Each cell is connected to its 
			   six immediate neighbors if they exist and are walkable.

	  Reference: Patel, A. J. (2013). Hexagonal grids. 
	             https://www.redblobgames.com/grids/hexagons/#neighbors
	  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const int even_directions[6][2] = {
		{ +1, 0 }, // East
		{ -1, 0 }, // West
		{ 0, -1 }, // North-East
		{ -1, -1 }, // North-West
		{ 0, +1 }, // South-East
		{ -1, +1 } // South-West
	};

	const int odd_directions[6][2] = {
		{ +1, 0 }, // East
		{ -1, 0 }, // West
		{ +1, -1 }, // North-East
		{ 0, -1 }, // North-West
		{ +1, +1 }, // South-East
		{ 0, +1 } // South-West
	};

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			if (!is_cell_walkable(index))
				continue;

			const int(*dirs)[2] = (row % 2 == 0) ? even_directions : odd_directions;

			// Iterate over the 6 directions
			for (int d = 0; d < 6; d++) {
				int nx = column + dirs[d][0];
				int ny = row + dirs[d][1];

				if (nx >= 0 && nx < data.columns && ny >= 0 && ny < data.rows) {
					int neighbor_index = ny * data.columns + nx;

					if (is_cell_walkable(neighbor_index)) {
						// Add the neighbor if it doesn't already exist
						if (!data.astar->has_point(neighbor_index)) {
							data.astar->add_point(neighbor_index, godot::Vector2(nx, ny));
						}

						data.astar->connect_points(index, neighbor_index);
						data.cells.at(index)->neighbors.push_back(neighbor_index);
					}
				}
			}
		}
	}
}

void InteractiveGrid3D::_configure_astar_8_dir() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Configures the A* pathfinding graph for eight directions 
  		   movement. Each cell is connected to all eight neighboring 
		   cells if the neighbor is walkable.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// Create 8-direction connections
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			for (int row_offset = -1; row_offset <= 1; ++row_offset) {
				for (int col_offset = -1; col_offset <= 1; ++col_offset) {
					if (col_offset == 0 && row_offset == 0)
						continue; // Do not connect to itself

					int nx = column + col_offset;
					int ny = row + row_offset;

					if (nx >= 0 && nx < data.columns && ny >= 0 && ny < data.rows) {
						int neighbor_index = ny * data.columns + nx;

						// Check if the neighbor is walkable before connecting
						bool neighbor_walkable = is_cell_walkable(neighbor_index);
						if (neighbor_walkable) {
							data.astar->connect_points(index, neighbor_index);
							data.cells.at(index)->neighbors.push_back(neighbor_index);
						}
					}
				}
			}
		}
	}
}

void InteractiveGrid3D::_breadth_first_search(unsigned int start_cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Performs a breadth-first search from a given start cell to 
			 identify which walkable grid cells are reachable. This 
			 traversal ignores non-walkable (blocked) cells and marks
			 only valid reachable tiles.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	struct BSFNode {
		int index{ 0 };
		bool visited{ false };
		bool is_walkable{ false };
		bool is_reachable{ false };
		godot::PackedInt64Array neighbors{};
	};

	unsigned int grid_size = data.rows * data.columns;
	godot::Vector<BSFNode> graph;
	graph.resize(grid_size);

	// Init nodes
	for (int index = 0; index < grid_size; index++) {
		graph.write[index].is_walkable = is_cell_walkable(index);
		graph.write[index].is_reachable = is_cell_reachable(index);
		graph.write[index].is_reachable = is_cell_reachable(index);
		graph.write[index].neighbors = get_neighbors(index);
	}

	std::queue<int> q; // FIFO queue for BFS

	graph.write[start_cell_index].visited = true;
	q.push(start_cell_index);

	// BFS loop
	while (!q.empty()) {
		int current = q.front(); // take the node at the front of the queue
		q.pop(); // remove it from the queue

		if (!graph[current].is_walkable) {
			continue;
		}

		// Explore all neighbors of the current node
		for (int neighbor : graph[current].neighbors) {
			if (!graph[neighbor].is_walkable) {
				continue;
			}

			if (!graph[neighbor].visited) {
				q.push(neighbor);
				graph.write[neighbor].visited = true;
			}
		}
	}

	// Mark unreachable walkable cells
	for (int index = 0; index < grid_size; index++) {
		if (graph[index].is_walkable && !graph[index].visited)
			set_cell_reachable(index, false);
	}
}

void InteractiveGrid3D::_bind_methods() {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Summary: _bind_methods, is a static function that Godot will call to 
				find out which methods can be called and which properties it
				exposes.
	-----------------------------------------------------------------F-F*/

	// Grid dimensions.

	godot::ClassDB::bind_method(godot::D_METHOD("set_rows"), &InteractiveGrid3D::set_rows);
	godot::ClassDB::bind_method(godot::D_METHOD("get_rows"), &InteractiveGrid3D::get_rows);

	godot::ClassDB::bind_method(godot::D_METHOD("set_columns"), &InteractiveGrid3D::set_columns);
	godot::ClassDB::bind_method(godot::D_METHOD("get_columns"), &InteractiveGrid3D::get_columns);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_size"), &InteractiveGrid3D::set_cell_size);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_size"), &InteractiveGrid3D::get_cell_size);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_mesh", "_cell_mesh"), &InteractiveGrid3D::set_cell_mesh);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_mesh"), &InteractiveGrid3D::get_cell_mesh);

	// Grid colors.

	godot::ClassDB::bind_method(godot::D_METHOD("set_walkable_color"), &InteractiveGrid3D::set_walkable_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_walkable_color"), &InteractiveGrid3D::get_walkable_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_unwalkable_color"), &InteractiveGrid3D::set_unwalkable_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_unwalkable_color"), &InteractiveGrid3D::get_unwalkable_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_unreachable_color"), &InteractiveGrid3D::set_unreachable_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_unreachable_color"), &InteractiveGrid3D::get_unreachable_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_selected_color"), &InteractiveGrid3D::set_selected_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_selected_color"), &InteractiveGrid3D::get_selected_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_path_color"), &InteractiveGrid3D::set_path_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_path_color"), &InteractiveGrid3D::get_path_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_hovered_color"), &InteractiveGrid3D::set_hovered_color);
	godot::ClassDB::bind_method(godot::D_METHOD("get_hovered_color"), &InteractiveGrid3D::get_hovered_color);

	// Grid materials.

	godot::ClassDB::bind_method(godot::D_METHOD("get_material_override"), &InteractiveGrid3D::get_material_override);
	godot::ClassDB::bind_method(godot::D_METHOD("set_material_override", "material"), &InteractiveGrid3D::set_material_override);

	// Highlight.

	godot::ClassDB::bind_method(godot::D_METHOD("highlight_on_hover", "global_position"), &InteractiveGrid3D::highlight_on_hover);
	godot::ClassDB::bind_method(godot::D_METHOD("highlight_path", "path"), &InteractiveGrid3D::highlight_path);

	godot::ClassDB::bind_method(godot::D_METHOD("set_hover_enabled", "enabled"), &InteractiveGrid3D::set_hover_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_hover_enabled"), &InteractiveGrid3D::is_hover_enabled);

	// Grid position.

	godot::ClassDB::bind_method(godot::D_METHOD("center", "center_position"), &InteractiveGrid3D::center);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_global_position", "cell_index"), &InteractiveGrid3D::get_cell_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_index_from_global_position", "global_position"), &InteractiveGrid3D::get_cell_index_from_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_center_global_position"), &InteractiveGrid3D::get_grid_center_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_top_left_global_position"), &InteractiveGrid3D::get_top_left_global_position);

	// Grid layout.

	godot::ClassDB::bind_method(godot::D_METHOD("set_layout", "value"), &InteractiveGrid3D::set_layout);
	godot::ClassDB::bind_method(godot::D_METHOD("get_layout"), &InteractiveGrid3D::get_layout);

	// Grid visibility.

	godot::ClassDB::bind_method(godot::D_METHOD("set_visible", "visible"), &InteractiveGrid3D::set_visible);
	godot::ClassDB::bind_method(godot::D_METHOD("is_visible"), &InteractiveGrid3D::is_visible);

	godot::ClassDB::bind_method(godot::D_METHOD("compute_unreachable_cells", "start_cell_index"), &InteractiveGrid3D::compute_unreachable_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("hide_distant_cells", "start_cell_index", "distance"), &InteractiveGrid3D::hide_distant_cells);

	// Grid state.

	godot::ClassDB::bind_method(godot::D_METHOD("is_grid_created"), &InteractiveGrid3D::is_created);
	godot::ClassDB::bind_method(godot::D_METHOD("reset_cells_state"), &InteractiveGrid3D::reset_cells_state);

	// Cell state.

	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_walkable", "cell_index"), &InteractiveGrid3D::is_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_reachable", "cell_index"), &InteractiveGrid3D::is_cell_reachable);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_hovered", "cell_index"), &InteractiveGrid3D::is_cell_hovered);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_selected", "cell_index"), &InteractiveGrid3D::is_cell_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_visible", "cell_index"), &InteractiveGrid3D::is_cell_visible);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_walkable", "cell_index", "is_walkable"), &InteractiveGrid3D::set_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_reachable", "cell_index", "set_cell_reachable"), &InteractiveGrid3D::set_cell_reachable);

	// Cell color.

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_color", "cell_index", "color"), &InteractiveGrid3D::set_cell_color);

	// Masks.

	godot::ClassDB::bind_method(godot::D_METHOD("set_obstacles_collision_masks", "masks"), &InteractiveGrid3D::set_obstacles_collision_masks);
	godot::ClassDB::bind_method(godot::D_METHOD("get_obstacles_collision_masks"), &InteractiveGrid3D::get_obstacles_collision_masks);

	godot::ClassDB::bind_method(godot::D_METHOD("set_grid_floor_collision_masks", "masks"), &InteractiveGrid3D::set_grid_floor_collision_masks);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_floor_collision_masks"), &InteractiveGrid3D::get_grid_floor_collision_masks);

	// Astar.

	godot::ClassDB::bind_method(godot::D_METHOD("set_movement", "value"), &InteractiveGrid3D::set_movement);
	godot::ClassDB::bind_method(godot::D_METHOD("get_movement"), &InteractiveGrid3D::get_movement);

	// User interaction.

	godot::ClassDB::bind_method(godot::D_METHOD("select_cell", "global_position"), &InteractiveGrid3D::select_cell);
	godot::ClassDB::bind_method(godot::D_METHOD("get_selected_cells"), &InteractiveGrid3D::get_selected_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("get_latest_selected"), &InteractiveGrid3D::get_latest_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("get_path", "start_cell_index", "target_cell_index"), &InteractiveGrid3D::get_path);
	godot::ClassDB::bind_method(godot::D_METHOD("get_neighbors", "cell_index"), &InteractiveGrid3D::get_neighbors);

	// Debug.

	godot::ClassDB::bind_method(godot::D_METHOD("set_print_logs_enabled", "enabled"), &InteractiveGrid3D::set_print_logs_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_print_logs_enabled"), &InteractiveGrid3D::is_print_logs_enabled);

	godot::ClassDB::bind_method(godot::D_METHOD("set_print_execution_time_enabled", "enabled"), &InteractiveGrid3D::set_print_execution_time_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_print_execution_time_enabled"), &InteractiveGrid3D::is_print_execution_time_enabled);

	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "_rows"), "set_rows", "get_rows");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "_columns"), "set_columns", "get_columns");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::VECTOR2, "cell_size"), "set_cell_size", "get_cell_size");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "_cell_mesh", godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_cell_mesh", "get_cell_mesh");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "walkable color"), "set_walkable_color", "get_walkable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "unwalkable color"), "set_unwalkable_color", "get_unwalkable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "unreachable color"), "set_unreachable_color", "get_unreachable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "selected color"), "set_selected_color", "get_selected_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "path color"), "set_path_color", "get_path_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "hovered color"), "set_hovered_color", "get_hovered_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "_material_override", godot::PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material_override", "get_material_override");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "layout", godot::PROPERTY_HINT_ENUM, "SQUARE, HEXAGONAL"), "set_layout", "get_layout");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "obstacles_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_RENDER), "set_obstacles_collision_masks", "get_obstacles_collision_masks");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "floor_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_RENDER), "set_grid_floor_collision_masks", "get_grid_floor_collision_masks");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "movement", godot::PROPERTY_HINT_ENUM, "FOUR-DIRECTIONS,SIX-DIRECTIONS,EIGH-DIRECTIONS"), "set_movement", "get_movement");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "print_logs_enabled"), "set_print_logs_enabled", "is_print_logs_enabled");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "print_execution_time"), "set_print_execution_time_enabled", "is_print_execution_time_enabled");
}

InteractiveGrid3D::InteractiveGrid3D() {}

InteractiveGrid3D::~InteractiveGrid3D() {
	_delete();
}

void InteractiveGrid3D::_ready(void) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary:  Called when the node enters the scene tree for the first time.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
}

void InteractiveGrid3D::_physics_process(double p_delta) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Called every frame. 'delta' is the elapsed time since the 
           previous frame.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_create(); // Create the grid at startup

	if (godot::Engine::get_singleton()->is_editor_hint()) {
		if (data.center_global_position != get_global_transform().origin) {
			_delete();
		}
	}
}

void InteractiveGrid3D::set_rows(const unsigned int rows) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the number of rows in the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.rows = rows;
	_delete();
}

int InteractiveGrid3D::get_rows(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the number of rows in the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.rows;
}

void InteractiveGrid3D::set_columns(const unsigned int columns) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the number of columns in the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.columns = columns;
	_delete();
}

int InteractiveGrid3D::get_columns(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the columns of rows in the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.columns;
}

void InteractiveGrid3D::set_cell_size(const godot::Vector2 cell_size) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the size of each cell in the grid. The provided 
           value will be stored and used for grid layout.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.cell_size = cell_size;
	_delete();
}

godot::Vector2 InteractiveGrid3D::get_cell_size(void) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the size of a single cell in the grid.
           This value is used to manage grid dimensions and cell 
		   positioning.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.cell_size;
}

void InteractiveGrid3D::set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the mesh used for each cell of the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.cell_mesh = p_mesh;
	_delete();
}

godot::Ref<godot::Mesh> InteractiveGrid3D::get_cell_mesh() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary:  Returns the mesh used for each cell of the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.cell_mesh;
}

void InteractiveGrid3D::set_movement(unsigned int value) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the movement type used for pathfinding on the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.movement = value;
}

unsigned int InteractiveGrid3D::get_movement() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the current movement type used for pathfinding on the
           grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.movement;
}

void InteractiveGrid3D::set_walkable_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the walkable color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.walkable_color = p_color;
	_delete();
}

godot::Color InteractiveGrid3D::get_walkable_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the walkable color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.walkable_color;
}

void InteractiveGrid3D::set_unwalkable_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the unwalkable color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.unwalkable_color = p_color;
	_delete();
}

godot::Color InteractiveGrid3D::get_unwalkable_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the unvalid color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.unwalkable_color;
}

void InteractiveGrid3D::set_unreachable_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the unreachable color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.unreachable_color = p_color;
}

godot::Color InteractiveGrid3D::get_unreachable_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the unreachable color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.unreachable_color;
}

void InteractiveGrid3D::set_selected_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the selected color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.selected_color = p_color;
}

godot::Color InteractiveGrid3D::get_selected_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the selected color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.selected_color;
}

void InteractiveGrid3D::set_path_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the path color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.path_color = p_color;
}

godot::Color InteractiveGrid3D::get_path_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the path color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.path_color;
}

void InteractiveGrid3D::set_hovered_color(const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the hovered color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.hovered_color = p_color;
}

godot::Color InteractiveGrid3D::get_hovered_color() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the hovered color for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.hovered_color;
}

void InteractiveGrid3D::set_material_override(const godot::Ref<godot::Material> &p_material) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the material override for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.material_override = p_material;
	_delete();
}

godot::Ref<godot::Material> InteractiveGrid3D::get_material_override() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the material override for the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.material_override;
}

void InteractiveGrid3D::apply_default_material() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Creates and applies a default ShaderMaterial to the grid's
           MultiMeshInstance.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (data.multimesh_instance == nullptr) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	godot::Ref<godot::Shader> shader;
	shader.instantiate();

	// Code Shader
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

	// Create a ShaderMaterial, instantiate it, and set the shader
	godot::Ref<godot::ShaderMaterial> shader_material;
	shader_material.instantiate();
	shader_material->set_shader(shader);

	// Apply it as the material_override of the MultiMeshInstance
	data.multimesh_instance->set_material_override(shader_material);

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Default ShaderMaterial created and applied.");
	}
}

void InteractiveGrid3D::highlight_on_hover(godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Updates the hover highlight state based on the specified 
           global position. Identifies the corresponding cell, clears
		   any previous hover, and applies the hover color unless the
		   cell is already selected.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (is_visible() == false) {
		return; // !Exit
	}

	if (is_centered() == false) {
		return; // !Exit
	}

	if (is_hover_enabled() == false) {
		return; // !Exit
	}

	// Retrieve the index of the cell that corresponds to the supplied
	// global position
	int closest_index = get_cell_index_from_global_position(global_position);

	// No cell under the mouse: clean the previously hovered cell (if any)
	if (closest_index == -1 || !is_cell_visible(closest_index)) {
		if (data.hovered_cell_index > -1) {
			_set_cell_hovered(data.hovered_cell_index, false);

			bool hovered_cell_is_selected = is_cell_selected(data.hovered_cell_index);

			if (!hovered_cell_is_selected) {
				set_cell_color(data.hovered_cell_index, data.walkable_color);
			}

			data.hovered_cell_index = -1;
		}
		return; // !Exit
	}

	// Already hovering over the same cell: nothing to do
	if (closest_index == data.hovered_cell_index) {
		return; // !Exit
	}

	// Check whether the new cell is already selected
	bool new_is_selected = is_cell_selected(closest_index);

	// Clear the previously hovered cell (if it exists)
	if (data.hovered_cell_index > -1) {
		bool old_is_selected = is_cell_selected(data.hovered_cell_index);

		_set_cell_hovered(data.hovered_cell_index, false);

		if (!old_is_selected) {
			set_cell_color(data.hovered_cell_index, data.walkable_color);
		}

		data.hovered_cell_index = -1;
	}

	// Skip non-walkable cells: only allow hovering on walkable cells
	bool walkable = is_cell_walkable(closest_index);
	if (!walkable) {
		return; // !Exit
	}

	// Skip unreachable cells
	bool unreachable = !is_cell_reachable(closest_index);
	if (unreachable) {
		return; // !Exit
	}

	// If the new cell is not selected, mark it as hovered
	if (!new_is_selected) {
		data.hovered_cell_index = closest_index;
		_set_cell_hovered(data.hovered_cell_index, true);
	}
}

void InteractiveGrid3D::highlight_path(const godot::PackedInt64Array &p_path) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Highlights a given path on the grid by changing the color of 
           each cell along the path to the predefined _path_color.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	for (int step = 0; step < p_path.size(); step++) {
		int cell_index = p_path[step];
		_set_cell_on_path(cell_index, true);
	}
}

godot::Vector3 InteractiveGrid3D::get_cell_global_position(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the global world‑space position of the cell identified
           by index. The method fetches the cell’s Transform3D from the
           internal `_cells` array and extracts its origin component.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	godot::Vector3 cell_global_position = data.cells.at(cell_index)->global_xform.origin;
	return cell_global_position;
}

int InteractiveGrid3D::get_cell_index_from_global_position(godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the linear index of the grid cell that is closest to
		   the supplied world‑space position.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created.");
		return -1;
	}

	if (!data.multimesh.is_valid()) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid multimesh is not valid.");
		return -1;
	}

	float center_to_edge_x{ 0.0f }, center_to_edge_z{ 0.0f };
	bool is_even_row = (data.rows % 2) == 0;

	switch (data.layout_index) {
		case LAYOUT::SQUARE:

			// Calculate the distances between the center and the grid's edges
			center_to_edge_x = (data.columns / 2) * data.cell_size.x + data.cell_size.x / 2;
			center_to_edge_z = (data.rows / 2) * data.cell_size.y + data.cell_size.y / 2;

			//  Initialize the member `grid_offset_`
			data.top_left_global_position.x = data.center_global_position.x - center_to_edge_x;
			data.top_left_global_position.z = data.center_global_position.z - center_to_edge_z;

			if (is_even_row) {
				if (global_position.x > (data.center_global_position.x + center_to_edge_x - data.cell_size.x) || global_position.x < (data.center_global_position.x - center_to_edge_x)) {
					return -1;
				}
				if (global_position.z > (data.center_global_position.z + center_to_edge_z - data.cell_size.y) || global_position.z < (data.center_global_position.z - center_to_edge_z)) {
					return -1;
				}
			} else {
				if (global_position.x > (data.center_global_position.x + center_to_edge_x) || global_position.x < (data.center_global_position.x - center_to_edge_x)) {
					return -1;
				}
				if (global_position.z > (data.center_global_position.z + center_to_edge_z) || global_position.z < (data.center_global_position.z - center_to_edge_z)) {
					return -1;
				}
			}
			break;
		case LAYOUT::HEXAGONAL:

			// The short diagonal (s) can be calculated using the formula: s = a · √3
			const float hex_short_diagonal = data.cell_size.x; // s

			// a = s / √3
			const float hex_side_length = hex_short_diagonal / sqrt(3); // a

			// The radius of the circumference that contains all vertices of a hexagon (R = a).
			const float hex_circumradius = hex_side_length * 2;

			// Calculate the distances between the center and the grid's edges.
			float center_to_grid_edge_x = (data.columns / 2) * data.cell_size.x;
			float center_to_grid_edge_z = (data.rows / 2) * data.cell_size.y;

			// Z-AXIS CORRECTION.
			if (!(data.rows % 2)) {
				center_to_grid_edge_z -= hex_side_length;
			}

			// X-AXIS CORRECTION.
			if (!(data.columns % 2)) {
				// Side to side.
				center_to_grid_edge_x -= data.cell_size.x / 2;
			}

			if (global_position.x < (data.top_left_global_position.x - data.cell_size.x / 2)) {
				return -1;
			}

			if (global_position.x > (data.top_left_global_position.x + center_to_grid_edge_x * 2 + data.cell_size.x)) {
				return -1;
			}

			if (global_position.z < data.top_left_global_position.z) {
				return -1;
			}

			if (is_even_row) {
				if (global_position.z > (data.top_left_global_position.z + center_to_grid_edge_z * 2 + hex_circumradius + hex_side_length / 2)) {
					godot::print_line("is_even_row true");
					return -1;
				}
			} else {
				if (global_position.z > (data.top_left_global_position.z + center_to_grid_edge_z * 2 + hex_circumradius)) {
					godot::print_line("is_even_row false");
					return -1;
				}
			}
	}

	float closest_distance = std::numeric_limits<float>::max();
	int closest_index = -1;

	// Iterate through the cells
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index =
					row * data.columns + column; // Index in the 2D array stored as 1D

			const godot::Vector3 cell_pos = data.cells.at(index)->global_xform.origin;
			const float distance = global_position.distance_to(cell_pos);

			if (distance < closest_distance) {
				closest_distance = distance;
				closest_index = index;
			}
		}
	}

	return closest_index;
}

godot::Vector3 InteractiveGrid3D::get_grid_center_global_position() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the central position of the interactive grid in world
           coordinates.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.center_global_position;
}

godot::Vector3 InteractiveGrid3D::get_top_left_global_position() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Called to re-center the grid. This also resets the grid state.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	return data.top_left_global_position;
}

void InteractiveGrid3D::center(godot::Vector3 center_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Called to re-center the grid. This also resets the grid state.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit
	}

	auto start = std::chrono::high_resolution_clock::now();

	data.flags &= ~GFL_CENTERED; // Reset

	set_hover_enabled(false); // Prevent hover during grid recentering
	reset_cells_state();
	_layout(center_position);
	_align_cells_with_floor();
	_scan_environnement_obstacles();
	_configure_astar();
	set_hover_enabled(true);

	data.flags |= GFL_CENTERED;

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Grid centered.");
	}
}

void InteractiveGrid3D::set_layout(unsigned int value) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Sets the grid layout.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.layout_index = value;
	_delete();
}

unsigned int InteractiveGrid3D::get_layout() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Returns the current grid layout.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.layout_index;
}

void InteractiveGrid3D::set_visible(bool visible) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the visibility of the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit
	}

	if ((data.flags & GFL_VISIBLE) && !visible) {
		// Visible
		_set_cells_visible(false);
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "false.");
		data.flags &= ~GFL_VISIBLE;
	} else if (!(data.flags & GFL_VISIBLE) && visible) {
		// Not visible
		_set_cells_visible(true);
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "true.");
		data.flags |= GFL_VISIBLE;
	}
}

bool InteractiveGrid3D::is_visible() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether the grid is currently visible in the scene.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.flags & GFL_VISIBLE) != 0;
}

void InteractiveGrid3D::compute_unreachable_cells(unsigned int start_cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Iterates over all grid cells and marks as unreachable those
           cells that cannot be reached from the specified start cell.
		   Updates the visual representation of unreachable cells by
		   applying the _unreachable_color to the grid's multimesh.
			
		   Unreachable cells are not marked as unwalkable to allow
		   gameplay features such as teleportation.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, start_cell_index)) {
		return; // !Exit
	}

	auto start = std::chrono::high_resolution_clock::now();

	if ((data.flags & GFL_VISIBLE) && !(data.flags & GFL_CELL_UNREACHABLE_HIDDEN)) {
		_configure_astar();
		_breadth_first_search(start_cell_index);
		data.flags |= GFL_CELL_UNREACHABLE_HIDDEN;
	}

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}
}

void InteractiveGrid3D::hide_distant_cells(unsigned int start_cell_index, float distance) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Iterates over all grid cells and hides those located farther 
           than the specified distance from the start cell. Marks them
           as non-walkable.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, start_cell_index)) {
		return; // !Exit
	}

	if ((data.flags & GFL_VISIBLE) && !(data.flags & GFL_CELL_DISTANT_HIDDEN)) {
		// Iterate through the cells
		for (int row = 0; row < data.rows; row++) {
			for (int column = 0; column < data.columns; column++) {
				const int index = row * data.columns + column;

				godot::Vector3 start_cell_position = data.cells.at(start_cell_index)->global_xform.origin;
				godot::Vector3 index_cell_position = data.cells.at(index)->global_xform.origin;

				if (start_cell_position.distance_to(index_cell_position) > distance) {
					set_cell_visible(index, false);
					data.cells.at(index)->flags &= ~CFL_WALKABLE;
				}
			}
		}
		data.flags |= GFL_CELL_DISTANT_HIDDEN;
	}
}

void InteractiveGrid3D::set_hover_enabled(bool enabled) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	Summary: Enables or disables hover functionality on the grid.
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit
	}

	if (enabled) {
		data.flags |= GFL_HOVER_ENABLED;
	} else {
		data.flags &= ~GFL_HOVER_ENABLED;
	}
}

bool InteractiveGrid3D::is_hover_enabled() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether hover functionality is currently disabled
	       on the grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.flags & GFL_HOVER_ENABLED) != 0;
}

bool InteractiveGrid3D::is_created() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks if the grid has been created.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.flags & GFL_CREATED) != 0;
}

bool InteractiveGrid3D::is_centered() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether the grid is currently centered. Returns true
	       if the GFL_CENTERED flag is set, false otherwise.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.flags & GFL_CENTERED) != 0;
}

bool InteractiveGrid3D::is_cell_walkable(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as walkable.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_WALKABLE) != 0;
}

bool InteractiveGrid3D::is_cell_reachable(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as unreachable.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_REACHABLE) != 0;
}

bool InteractiveGrid3D::is_cell_in_void(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether a specific cell is marked as "in void".
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_IN_VOID) != 0;
}

bool InteractiveGrid3D::is_cell_hovered(const unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as hovered.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_HOVERED) != 0;
}

bool InteractiveGrid3D::is_cell_selected(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as selected.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_SELECTED) != 0;
}

bool InteractiveGrid3D::is_cell_on_path(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently 
           marked as part of the path.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_PATH) != 0;
}

bool InteractiveGrid3D::is_cell_visible(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns true if the cell at the specified index is currently
           marked as visible.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return (data.cells.at(cell_index)->flags & CFL_VISIBLE) != 0;
}

void InteractiveGrid3D::set_cell_walkable(unsigned int cell_index, bool is_walkable) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a specific cell is walkable or not.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_walkable) {
		data.cells.at(cell_index)->flags |= CFL_WALKABLE;
		set_cell_color(cell_index, data.walkable_color);
	} else if (!is_walkable) {
		data.cells.at(cell_index)->flags &= ~CFL_WALKABLE;
		set_cell_color(cell_index, data.unwalkable_color);
	}
}

void InteractiveGrid3D::set_cell_reachable(unsigned int cell_index, bool is_reachable) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets whether a given grid cell is unreachable.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (is_reachable) {
		data.cells.at(cell_index)->flags |= CFL_REACHABLE;
	} else if (!is_reachable) {
		data.cells.at(cell_index)->flags &= ~CFL_REACHABLE;
		set_cell_color(cell_index, data.unreachable_color);
	}
}

void InteractiveGrid3D::set_cell_visible(unsigned int cell_index, bool is_visible) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the visibility of a grid cell identified by cell_index.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	godot::Color current_cell_color = data.cells.at(cell_index)->color;

	if (is_visible) {
		data.cells.at(cell_index)->flags |= CFL_VISIBLE;
		set_cell_color(cell_index, current_cell_color);
	} else if (!is_visible) {
		current_cell_color.a = 0.0;
		data.multimesh->set_instance_custom_data(cell_index, current_cell_color);
		data.cells.at(cell_index)->flags &= ~CFL_VISIBLE;
	}
}

void InteractiveGrid3D::InteractiveGrid3D::reset_cells_state() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Resets the state of all cells in the grid to their default 
           flags.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return; // !Exit
	}

	// Iterate through the cells
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;
			data.cells.at(index)->flags = 0; // Reset
		}
	}

	data.flags &= ~GFL_CELL_UNREACHABLE_HIDDEN; // Reset
	data.flags &= ~GFL_CELL_DISTANT_HIDDEN; // Reset

	data.hovered_cell_index = -1;
	data.selected_cells.clear();
}

void InteractiveGrid3D::set_cell_color(unsigned int cell_index, const godot::Color &p_color) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the color of a specific cell in the interactive grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, cell_index)) {
		return; // !Exit
	}

	if (data.material_override.is_valid()) {
		uint32_t cell_flags = data.cells.at(cell_index)->flags;
		godot::Color new_cell_color{ p_color.r, p_color.g, p_color.b, static_cast<float>(cell_flags) };
		data.cells.at(cell_index)->color = new_cell_color;
		data.multimesh->set_instance_custom_data(cell_index, new_cell_color);
	} else {
		data.cells.at(cell_index)->color = p_color;
		data.multimesh->set_instance_custom_data(cell_index, p_color);
	}
}

void InteractiveGrid3D::set_obstacles_collision_masks(unsigned int masks) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the collision masks used by the interactive grid to 
           detect obstacles. These masks define which objects are 
		   considered as obstacles when checking for collisions.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.obstacles_collision_masks = masks;
}

int InteractiveGrid3D::get_obstacles_collision_masks() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the collision masks currently configured for obstacle 
           detection. These masks specify which objects are treated as 
           obstacles by the interactive grid.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.obstacles_collision_masks;
}

void InteractiveGrid3D::set_grid_floor_collision_masks(unsigned int masks) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Sets the collision masks used by the interactive grid to 
           detect and align with scene floor (meshes).
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	data.floor_collision_masks = masks;
}

int InteractiveGrid3D::get_grid_floor_collision_masks() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the collision masks currently configured for the 
           interactive grid alignment. These masks specify which floor
		   are used as references when aligning grid cells with meshes in 
		   the scene
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.floor_collision_masks;
}

void InteractiveGrid3D::select_cell(godot::Vector3 global_position) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Selects a grid cell based on a world‑space position.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	// If the grid isn’t visible, exit early
	if (is_visible() == false) {
		return; // !Exit
	}

	// Retrieve the index of the cell that corresponds to the supplied
	// global position
	int closest_index = get_cell_index_from_global_position(global_position);

	// If the index is valid
	if (closest_index != -1) {
		// Skip invisible
		bool visible = is_cell_visible(closest_index);
		if (!visible) {
			return;
		}

		// Skip unreachable cells
		bool unreachable = !is_cell_reachable(closest_index);
		if (unreachable) {
			return;
		}

		bool walkable = is_cell_walkable(closest_index);
		if (walkable) {
			_set_cell_selected(closest_index, true);
			data.selected_cells.push_back(closest_index);
		}
	}
}

godot::Array InteractiveGrid3D::get_selected_cells() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns an array of all cells currently marked as selected.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.selected_cells;
}

int InteractiveGrid3D::get_latest_selected() {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the most recently selected cell.

  Last Modified: September 29, 2025
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.selected_cells.back();
}

godot::PackedInt64Array InteractiveGrid3D::get_path(unsigned int start_cell_index, unsigned int target_cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Computes a path between two cells on the grid using A* 
           pathfinding.
		
           Sets up all grid points with their walkable state and 
		   configures the A* algorithm according to the selected movement
		   type (orthogonal or diagonal).
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/

	godot::PackedInt64Array path;

	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return path; // !Exit
	}

	auto start = std::chrono::high_resolution_clock::now();

	path = data.astar->get_id_path(start_cell_index, target_cell_index);

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}

	return path;
}

godot::PackedInt64Array InteractiveGrid3D::get_neighbors(unsigned int cell_index) const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Returns the indices of neighboring cells for the specified
           grid cell.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return data.cells.at(cell_index)->neighbors;
}

void InteractiveGrid3D::set_print_logs_enabled(bool enabled) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
    Summary: Enables or disables debug log printing.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_debug_options.print_logs_enabled = enabled;
}

bool InteractiveGrid3D::is_print_logs_enabled() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
    Summary: Returns whether debug log printing is currently enabled.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _debug_options.print_logs_enabled;
}

void InteractiveGrid3D::set_print_execution_time_enabled(bool enabled) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
    Summary: Enables or disables printing of execution time for debugging.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	_debug_options.print_execution_time_enabled = enabled;
}

bool InteractiveGrid3D::is_print_execution_time_enabled() const {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
    Summary:  Returns whether execution time printing is currently 
	          enabled.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	return _debug_options.print_execution_time_enabled;
}

bool InteractiveGrid3D::is_cell_index_out_of_bounds(const godot::String &file, const godot::String &func, int line, unsigned int cell_index) {
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Summary: Checks whether the specified cell index exceeds the valid 
           grid bounds.
  M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	bool is_out_of_bounds = false;
	unsigned int grid_size = data.rows * data.columns;

	if (cell_index >= (grid_size)) {
		PrintError(file, func, line, "Cell index out of bounds: ", cell_index, " >= ", (grid_size));
		is_out_of_bounds = true;
	}

	return is_out_of_bounds;
}