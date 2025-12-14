/**************************************************************************/
/*  interactive_grid.cpp                                                  */
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

#include "interactive_grid_3d.h"

constexpr const char *default_shader_code = R"(
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

void InteractiveGrid3D::_create() {
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
	if (data.flags & GFL_CREATED) {
		for (Cell *c : data.cells) {
			delete c;
		}
		data.cells.clear();

		if (data.multimesh_instance) {
			data.multimesh_instance->queue_free();
			data.multimesh_instance = nullptr;
		}

		data.astar = godot::Ref<godot::AStar2D>();
		data.flags &= ~GFL_CREATED;
	}
}

void InteractiveGrid3D::_init_multi_mesh() {
	data.multimesh_instance = memnew(godot::MultiMeshInstance3D);
	this->add_child(data.multimesh_instance);
	data.multimesh.instantiate();

	data.multimesh->set_transform_format(godot::MultiMesh::TRANSFORM_3D);
	data.multimesh->set_use_custom_data(true);

	int cell_count = data.columns * data.rows;
	data.multimesh->set_instance_count(cell_count);

	data.multimesh_instance->set_multimesh(data.multimesh);
	data.multimesh->set_mesh(data.cell_mesh);

	godot::Transform3D xform;
	xform.origin = godot::Vector3(0, 0, 0);

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index =
					row * data.columns + column;

			data.multimesh->set_instance_transform(index, xform);
			data.multimesh->set_instance_custom_data(index, data.walkable_color);

			data.cells.push_back(new Cell);
			data.cells.write[index]->index = index;
			data.cells.write[index]->local_xform = xform;
			data.cells.write[index]->global_xform = xform;
		}
	}

	_apply_material(data.material_override);

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid MultiMesh has been created.");
	}
}

void InteractiveGrid3D::_init_astar() {
	data.astar.instantiate();
}

void InteractiveGrid3D::_layout(godot::Vector3 p_center_position) {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return;
	}

	switch (data.layout_index) {
		case Layout::LAYOUT_SQUARE:
			_layout_cells_as_square_grid(p_center_position);
			break;
		case Layout::LAYOUT_HEXAGONAL:
			_layout_cells_as_hexagonal_grid(p_center_position);
			break;
	}
}

void InteractiveGrid3D::_layout_cells_as_square_grid(godot::Vector3 p_center_position) {
	data.center_global_position = p_center_position;

	const float center_to_edge_x = (data.columns / 2) * data.cell_size.x;
	const float center_to_edge_z = (data.rows / 2) * data.cell_size.y;

	godot::Vector3 top_left_global_position;
	top_left_global_position.x = p_center_position.x - center_to_edge_x;
	top_left_global_position.z = p_center_position.z - center_to_edge_z;

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			float cell_pos_x = top_left_global_position.x + column * data.cell_size.x;
			float cell_pos_y = p_center_position.y;
			float cell_pos_z = top_left_global_position.z + row * data.cell_size.y;
			godot::Vector3 cell_pos(cell_pos_x, cell_pos_y, cell_pos_z);

			godot::Transform3D global_xform = data.multimesh_instance->get_global_transform();
			godot::Transform3D local_xform = global_xform.affine_inverse();

			godot::Vector3 local_pos = local_xform.xform(cell_pos);

			godot::Transform3D xform;
			xform.origin = local_pos;

			data.multimesh->set_instance_transform(index, xform);

			data.cells.write[index]->local_xform = data.multimesh->get_instance_transform(index);
			data.cells.write[index]->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

			set_cell_visible(index, false);
		}
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid cells have been laid out as a square grid.");
	}
}

void InteractiveGrid3D::_layout_cells_as_hexagonal_grid(godot::Vector3 p_center_position) {
	data.center_global_position = p_center_position;

	const float hex_short_diagonal = data.cell_size.x; // s = a · √3
	const float hex_side_length = hex_short_diagonal / sqrt(3); // a = s / √3.
	const float hex_side_to_side = data.cell_size.x / 2;
	const float hex_inradius = hex_side_length * sqrt(3) / 2; // r = a · √3 / 2.

	float center_to_edge_x = (data.columns / 2) * data.cell_size.x;
	float center_to_edge_z = (data.rows / 2) * data.cell_size.y;

	if ((data.columns % 2)) {
		center_to_edge_x;
	}

	if (!(data.rows % 2)) {
		center_to_edge_z -= hex_side_length;
	}

	godot::Vector3 top_left_global_position;
	top_left_global_position.x = p_center_position.x - center_to_edge_x;
	top_left_global_position.z = p_center_position.z - center_to_edge_z;

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			float cell_pos_x{ 0.0f };
			if (!(row % 2)) { // Even.
				cell_pos_x = top_left_global_position.x + (column * data.cell_size.x);
			} else {
				cell_pos_x = top_left_global_position.x + (column * data.cell_size.x) + hex_side_to_side;
			}

			float cell_pos_y = p_center_position.y;
			float cell_pos_z = top_left_global_position.z + (row * data.cell_size.y);
			godot::Vector3 cell_pos(cell_pos_x, cell_pos_y, cell_pos_z);

			godot::Transform3D global_xform = data.multimesh_instance->get_global_transform();
			godot::Transform3D local_xform = global_xform.affine_inverse();
			godot::Vector3 local_pos = local_xform.xform(cell_pos);
			godot::Transform3D xform;
			xform.origin = local_pos;

			data.multimesh->set_instance_transform(index, xform);

			data.cells.write[index]->local_xform = data.multimesh->get_instance_transform(index);
			data.cells.write[index]->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

			set_cell_visible(index, false);
		}
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "The grid cells have been laid out as a hexagonal grid.");
	}
}

void InteractiveGrid3D::_configure_astar() {
	if (godot::Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	data.astar->clear();

	// Register all grid points and mark obstacles.
	for (int index = 0; index < data.cells.size(); ++index) {
		int x = index % data.columns;
		int y = index / data.columns;
		data.astar->add_point(index, godot::Vector2(x, y), 1.0);
		data.astar->set_point_disabled(index, !is_cell_walkable(index));
	}

	switch (data.movement) {
		case Movement::MOVEMENT_FOUR_DIRECTIONS:
			_configure_astar_4_dir();
			break;
		case Movement::MOVEMENT_SIX_DIRECTIONS:
			_configure_astar_6_dir();
			break;
		case Movement::MOVEMENT_EIGH_DIRECTIONS:
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
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			// Connect to the right
			if (column + 1 < data.columns) {
				int right = row * data.columns + (column + 1);
				data.astar->connect_points(index, right);
				data.cells[index]->neighbors.push_back(right);
			}

			// Connect to the left
			if (column - 1 >= 0) {
				int left = row * data.columns + (column - 1);
				//_astar->connect_points(index, left);
				data.cells[index]->neighbors.push_back(left);
			}

			// Connect to the down
			if (row + 1 < data.rows) {
				int down = (row + 1) * data.columns + column;
				data.astar->connect_points(index, down);
				data.cells[index]->neighbors.push_back(down);
			}

			// Connect to the up
			if (row - 1 >= 0) {
				int up = (row - 1) * data.columns + column;
				//_astar->connect_points(index, up);
				data.cells[index]->neighbors.push_back(up);
			}
		}
	}
}

void InteractiveGrid3D::_configure_astar_6_dir() {
	const int even_directions[6][2] = {
		{ +1, 0 }, // East.
		{ -1, 0 }, // West.
		{ 0, -1 }, // North-East.
		{ -1, -1 }, // North-West.
		{ 0, +1 }, // South-East.
		{ -1, +1 } // South-West.
	};

	const int odd_directions[6][2] = {
		{ +1, 0 }, // East.
		{ -1, 0 }, // West.
		{ +1, -1 }, // North-East.
		{ 0, -1 }, // North-West.
		{ +1, +1 }, // South-East.
		{ 0, +1 } // South-West.
	};

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			const int(*dirs)[2] = (row % 2 == 0) ? even_directions : odd_directions;

			// Iterate over the 6 directions.
			for (int d = 0; d < 6; d++) {
				int nx = column + dirs[d][0];
				int ny = row + dirs[d][1];

				if (nx >= 0 && nx < data.columns && ny >= 0 && ny < data.rows) {
					int neighbor_index = ny * data.columns + nx;

					data.cells[index]->neighbors.push_back(neighbor_index);

					if (!is_cell_walkable(index))
						continue;

					if (is_cell_walkable(neighbor_index)) {
						if (!data.astar->has_point(neighbor_index)) {
							data.astar->add_point(neighbor_index, godot::Vector2(nx, ny));
						}

						data.astar->connect_points(index, neighbor_index);
					}
				}
			}
		}
	}
}

void InteractiveGrid3D::_configure_astar_8_dir() {
	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;

			for (int row_offset = -1; row_offset <= 1; ++row_offset) {
				for (int col_offset = -1; col_offset <= 1; ++col_offset) {
					if (col_offset == 0 && row_offset == 0)
						continue; // Do not connect to itself.

					int nx = column + col_offset;
					int ny = row + row_offset;

					if (nx >= 0 && nx < data.columns && ny >= 0 && ny < data.rows) {
						int neighbor_index = ny * data.columns + nx;
						data.cells[index]->neighbors.push_back(neighbor_index);

						bool neighbor_walkable = is_cell_walkable(neighbor_index);
						if (neighbor_walkable) {
							data.astar->connect_points(index, neighbor_index);
						}
					}
				}
			}
		}
	}
}

void InteractiveGrid3D::_breadth_first_search(unsigned int p_start_cell_index) {
	struct BSFNode {
		int index{ 0 };
		bool visited = false;
		bool is_walkable = false;
		bool is_reachable = false;
		godot::PackedInt64Array neighbors;
	};

	unsigned int grid_size = data.rows * data.columns;
	godot::Vector<BSFNode> graph;
	graph.resize(grid_size);

	for (int index = 0; index < grid_size; index++) {
		graph.write[index].is_walkable = is_cell_walkable(index);
		graph.write[index].is_reachable = is_cell_reachable(index);
		graph.write[index].is_reachable = is_cell_reachable(index);
		graph.write[index].neighbors = get_neighbors(index);
	}

	std::queue<int> q;

	graph.write[p_start_cell_index].visited = true;
	q.push(p_start_cell_index);

	while (!q.empty()) {
		int current = q.front();
		q.pop();

		if (!graph[current].is_walkable) {
			continue;
		}

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

	for (int index = 0; index < grid_size; index++) {
		if (graph[index].is_walkable && !graph[index].visited)
			set_cell_reachable(index, false);
	}
}

void InteractiveGrid3D::_align_cells_with_floor() {
	if (data.flags & GFL_CREATED) {
		auto start = std::chrono::high_resolution_clock::now();

		const int ray_length = 500;

		const godot::Transform3D global_transform = data.multimesh_instance->get_global_transform();
		const godot::Transform3D global_to_local = global_transform.affine_inverse();

		for (int row = 0; row < data.rows; row++) {
			for (int column = 0; column < data.columns; column++) {
				const int index =
						row * data.columns + column;

				godot::Vector3 local_from = data.cells[index]->local_xform.origin;
				godot::Vector3 global_from = data.cells[index]->global_xform.origin;

				global_from.y += 100.0f;

				godot::Vector3 global_to = global_from - godot::Vector3(0, ray_length, 0);

				godot::Ref<godot::World3D> world = get_world_3d();
				godot::PhysicsDirectSpaceState3D *space_state = world->get_direct_space_state();
				godot::Ref<godot::PhysicsRayQueryParameters3D> ray_query;
				ray_query.instantiate();
				ray_query->set_collide_with_areas(true);
				ray_query->set_from(global_from);
				ray_query->set_to(global_to);
				ray_query->set_collision_mask(data.floor_collision_mask);

				godot::TypedArray<godot::RID> exclude_array;
				exclude_array.append(data.multimesh->get_rid());
				ray_query->set_exclude(exclude_array);

				godot::Dictionary result = space_state->intersect_ray(ray_query);
				if (!result.is_empty()) {
					godot::Object *collider_obj = Object::cast_to<godot::Object>(result["collider"]);

					if (collider_obj) {
						godot::Node3D *collider_node = Object::cast_to<godot::Node3D>(collider_obj);

						if (collider_node && !collider_node->is_visible_in_tree()) {
							continue;
						}
					}

					godot::Vector3 hit_position_global = result["position"];
					godot::Vector3 floor_normal = result["normal"];
					godot::Vector3 hit_position_local = global_to_local.xform(hit_position_global);

					godot::Transform3D xform;
					xform.origin = hit_position_local;
					xform.basis.set_column(1, floor_normal.normalized());
					godot::Vector3 basis_z = xform.basis.get_column(2);
					godot::Vector3 basis_x = floor_normal.cross(basis_z).normalized();
					xform.basis.set_column(0, basis_x);
					basis_z = basis_x.cross(floor_normal).normalized();
					xform.basis.set_column(2, basis_z);
					xform.basis = xform.basis.orthonormalized();
					data.multimesh->set_instance_transform(index, xform);
					data.cells.write[index]->local_xform = xform;
					data.cells.write[index]->global_xform = data.multimesh_instance->get_global_transform() * data.multimesh->get_instance_transform(index);

					set_cell_walkable(index, true);
					set_cell_reachable(index, true);
					set_cell_visible(index, true);

				} else if (!godot::Engine::get_singleton()->is_editor_hint()) {
					_set_cell_in_void(index, true);
				} else {
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
	if (data.cell_mesh.is_null()) {
		return;
	}

	if (!data.cell_shape.is_valid()) {
		return;
	}

	godot::PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();

	if (!space_state) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No PhysicsDirectSpaceState3D available.");
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;
			const godot::Vector3 cell_shape_pos = data.cells[index]->global_xform.origin + data.cell_shape_offset;
			godot::Ref<godot::PhysicsShapeQueryParameters3D> query;
			query.instantiate();
			query->set_shape(data.cell_shape);
			query->set_transform(godot::Transform3D(godot::Basis(), cell_shape_pos));
			query->set_collision_mask(data.obstacles_collision_masks);
			query->set_collide_with_bodies(true);
			query->set_collide_with_areas(true);

			godot::TypedArray<godot::Dictionary> results = space_state->intersect_shape(query, 16);
			if (results.size() > 0) {
				for (int k = 0; k < results.size(); k++) {
					godot::Dictionary hit = results[k];

					godot::Object *collider_obj = hit["collider"];
					godot::Node *collider = godot::Object::cast_to<godot::Node>(collider_obj);

					if (collider) {
						set_cell_walkable(index, false);
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

void InteractiveGrid3D::_scan_environnement_custom_data() {
	if (data.cell_mesh.is_null()) {
		return;
	}

	if (!data.cell_shape.is_valid()) {
		return;
	}

	godot::PhysicsDirectSpaceState3D *space_state = get_world_3d()->get_direct_space_state();

	if (!space_state) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No PhysicsDirectSpaceState3D available.");
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int cell_index = row * data.columns + column;

			if (is_cell_in_void(cell_index)) {
				continue;
			}

			const godot::Vector3 cell_pos = data.cells[cell_index]->global_xform.origin;

			godot::Ref<godot::PhysicsShapeQueryParameters3D> query;
			query.instantiate();
			query->set_shape(data.cell_shape);
			query->set_transform(godot::Transform3D(godot::Basis(), cell_pos));
			query->set_collision_mask(UINT32_MAX);
			query->set_collide_with_bodies(true);
			query->set_collide_with_areas(true);

			godot::TypedArray<godot::Dictionary> results = space_state->intersect_shape(query, 16);
			if (results.size() > 0) {
				for (int k = 0; k < results.size(); k++) {
					godot::Dictionary hit = results[k];
					godot::Object *collider_obj = hit["collider"];
					godot::Node *collider = godot::Object::cast_to<godot::Node>(collider_obj);
					godot::CollisionObject3D *collision_object =
							godot::Object::cast_to<godot::CollisionObject3D>(collider_obj);

					if (collider) {
						godot::Ref<CustomCellData> custom_cell_data;

						for (int index = 0; index < data.custom_cell_data.size(); index++) {
							custom_cell_data = data.custom_cell_data.get(index);

							if (!collision_object) {
								continue;
							}

							if (custom_cell_data.is_null()) {
								continue;
							}

							if (custom_cell_data->get_collision_layer() == 0) {
								continue;
							}

							if (custom_cell_data->get_layer_mask() == 0) {
								continue;
							}

							uint32_t collision_layer = collision_object->get_collision_layer();
							if (!custom_cell_data->has_layers_in_mask(collision_layer)) {
								continue;
							}

							data.cells.write[cell_index]->custom_flags |= custom_cell_data->get_layer_mask();
							data.cells.write[cell_index]->flags |= custom_cell_data->get_layer_mask();

							if (custom_cell_data->get_custom_color_enabled()) {
								data.cells.write[cell_index]->has_custom_color = true;
								data.cells.write[cell_index]->custom_color = custom_cell_data->get_color();

								set_cell_color(cell_index, data.cells[cell_index]->custom_color);
							}
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
	if (data.multimesh_instance == nullptr) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	if (p_material.is_null()) {
		data.multimesh_instance->set_material_override(nullptr);
		apply_default_material();
		return;
	} else {
		data.multimesh_instance->set_material_override(p_material);
	}
}

void InteractiveGrid3D::_set_cell_in_void(int p_cell_index, bool p_is_in_void) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_in_void) {
		data.cells.write[p_cell_index]->flags |= CFL_IN_VOID;
		set_cell_visible(p_cell_index, false);
	} else if (!p_is_in_void) {
		data.cells.write[p_cell_index]->flags &= ~CFL_IN_VOID;
	}
}

void InteractiveGrid3D::_set_cell_hovered(int p_cell_index, bool p_is_hovered) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_hovered) {
		data.cells.write[p_cell_index]->flags |= CFL_HOVERED;
		set_cell_color(data.hovered_cell_index, data.hovered_color);
	} else if (!p_is_hovered) {
		data.cells.write[p_cell_index]->flags &= ~CFL_HOVERED;
	}
}

void InteractiveGrid3D::_set_cell_selected(int p_cell_index, bool p_is_selected) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_selected) {
		data.cells.write[p_cell_index]->flags |= CFL_SELECTED;
		set_cell_color(p_cell_index, data.selected_color);
	} else if (!p_is_selected) {
		data.cells.write[p_cell_index]->flags &= ~CFL_SELECTED;
	}
}

void InteractiveGrid3D::_set_cell_on_path(int p_cell_index, bool p_is_on_path) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_on_path) {
		data.cells.write[p_cell_index]->flags |= CFL_PATH;
		set_cell_color(p_cell_index, data.path_color);
	} else if (!p_is_on_path) {
		data.cells.write[p_cell_index]->flags &= ~CFL_PATH;
	}
}

void InteractiveGrid3D::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("set_rows"), &InteractiveGrid3D::set_rows);
	godot::ClassDB::bind_method(godot::D_METHOD("get_rows"), &InteractiveGrid3D::get_rows);

	godot::ClassDB::bind_method(godot::D_METHOD("set_columns"), &InteractiveGrid3D::set_columns);
	godot::ClassDB::bind_method(godot::D_METHOD("get_columns"), &InteractiveGrid3D::get_columns);

	godot::ClassDB::bind_method(godot::D_METHOD("get_size"), &InteractiveGrid3D::get_size);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_size"), &InteractiveGrid3D::set_cell_size);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_size"), &InteractiveGrid3D::get_cell_size);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_mesh", "cell_mesh"), &InteractiveGrid3D::set_cell_mesh);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_mesh"), &InteractiveGrid3D::get_cell_mesh);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_shape", "cell_shape"), &InteractiveGrid3D::set_cell_shape);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_shape"), &InteractiveGrid3D::get_cell_shape);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_shape_offset", "cell_shape_offset"), &InteractiveGrid3D::set_cell_shape_offset);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_shape_offset"), &InteractiveGrid3D::get_cell_shape_offset);

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

	godot::ClassDB::bind_method(godot::D_METHOD("set_custom_cells_data"), &InteractiveGrid3D::set_custom_cells_data);
	godot::ClassDB::bind_method(godot::D_METHOD("get_custom_cells_data"), &InteractiveGrid3D::get_custom_cells_data);

	godot::ClassDB::bind_method(godot::D_METHOD("add_custom_cell_data", "cell_index", "custom_data_name"), &InteractiveGrid3D::add_custom_cell_data);
	godot::ClassDB::bind_method(godot::D_METHOD("has_custom_cell_data", "cell_index", "custom_data_name"), &InteractiveGrid3D::has_custom_cell_data);
	godot::ClassDB::bind_method(godot::D_METHOD("clear_custom_cell_data", "cell_index", "custom_data_name", "clear_custom_color"), &InteractiveGrid3D::clear_custom_cell_data);
	godot::ClassDB::bind_method(godot::D_METHOD("clear_all_custom_cell_data", "cell_index"), &InteractiveGrid3D::clear_all_custom_cell_data);

	godot::ClassDB::bind_method(godot::D_METHOD("get_material_override"), &InteractiveGrid3D::get_material_override);
	godot::ClassDB::bind_method(godot::D_METHOD("set_material_override", "material"), &InteractiveGrid3D::set_material_override);

	godot::ClassDB::bind_method(godot::D_METHOD("highlight_on_hover", "global_position"), &InteractiveGrid3D::highlight_on_hover);
	godot::ClassDB::bind_method(godot::D_METHOD("highlight_path", "path"), &InteractiveGrid3D::highlight_path);

	godot::ClassDB::bind_method(godot::D_METHOD("set_hover_enabled", "enabled"), &InteractiveGrid3D::set_hover_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_hover_enabled"), &InteractiveGrid3D::is_hover_enabled);

	godot::ClassDB::bind_method(godot::D_METHOD("center", "center_position"), &InteractiveGrid3D::center);
	godot::ClassDB::bind_method(godot::D_METHOD("update_custom_data"), &InteractiveGrid3D::update_custom_data);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_global_position", "cell_index"), &InteractiveGrid3D::get_cell_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_cell_index_from_global_position", "global_position"), &InteractiveGrid3D::get_cell_index_from_global_position);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_center_global_position"), &InteractiveGrid3D::get_grid_center_global_position);

	godot::ClassDB::bind_method(godot::D_METHOD("set_layout", "layout"), &InteractiveGrid3D::set_layout);
	godot::ClassDB::bind_method(godot::D_METHOD("get_layout"), &InteractiveGrid3D::get_layout);

	godot::ClassDB::bind_method(godot::D_METHOD("set_movement", "movement"), &InteractiveGrid3D::set_movement);
	godot::ClassDB::bind_method(godot::D_METHOD("get_movement"), &InteractiveGrid3D::get_movement);

	godot::ClassDB::bind_method(godot::D_METHOD("compute_unreachable_cells", "start_cell_index"), &InteractiveGrid3D::compute_unreachable_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("hide_distant_cells", "start_cell_index", "distance"), &InteractiveGrid3D::hide_distant_cells);

	godot::ClassDB::bind_method(godot::D_METHOD("is_grid_created"), &InteractiveGrid3D::is_created);
	godot::ClassDB::bind_method(godot::D_METHOD("reset_cells_state"), &InteractiveGrid3D::reset_cells_state);

	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_walkable", "cell_index"), &InteractiveGrid3D::is_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_reachable", "cell_index"), &InteractiveGrid3D::is_cell_reachable);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_hovered", "cell_index"), &InteractiveGrid3D::is_cell_hovered);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_selected", "cell_index"), &InteractiveGrid3D::is_cell_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("is_cell_visible", "cell_index"), &InteractiveGrid3D::is_cell_visible);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_walkable", "cell_index", "is_walkable"), &InteractiveGrid3D::set_cell_walkable);
	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_reachable", "cell_index", "set_cell_reachable"), &InteractiveGrid3D::set_cell_reachable);

	godot::ClassDB::bind_method(godot::D_METHOD("set_cell_color", "cell_index", "color"), &InteractiveGrid3D::set_cell_color);

	godot::ClassDB::bind_method(godot::D_METHOD("set_obstacles_collision_masks", "masks"), &InteractiveGrid3D::set_obstacles_collision_masks);
	godot::ClassDB::bind_method(godot::D_METHOD("get_obstacles_collision_masks"), &InteractiveGrid3D::get_obstacles_collision_masks);

	godot::ClassDB::bind_method(godot::D_METHOD("set_grid_floor_collision_masks", "masks"), &InteractiveGrid3D::set_grid_floor_collision_mask);
	godot::ClassDB::bind_method(godot::D_METHOD("get_grid_floor_collision_masks"), &InteractiveGrid3D::get_grid_floor_collision_mask);

	godot::ClassDB::bind_method(godot::D_METHOD("select_cell", "global_position"), &InteractiveGrid3D::select_cell);
	godot::ClassDB::bind_method(godot::D_METHOD("get_selected_cells"), &InteractiveGrid3D::get_selected_cells);
	godot::ClassDB::bind_method(godot::D_METHOD("get_latest_selected"), &InteractiveGrid3D::get_latest_selected);
	godot::ClassDB::bind_method(godot::D_METHOD("get_path", "start_cell_index", "target_cell_index"), &InteractiveGrid3D::get_path);
	godot::ClassDB::bind_method(godot::D_METHOD("get_neighbors", "cell_index"), &InteractiveGrid3D::get_neighbors);

	godot::ClassDB::bind_method(godot::D_METHOD("set_print_logs_enabled", "enabled"), &InteractiveGrid3D::set_print_logs_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_print_logs_enabled"), &InteractiveGrid3D::is_print_logs_enabled);

	godot::ClassDB::bind_method(godot::D_METHOD("set_print_execution_time_enabled", "enabled"), &InteractiveGrid3D::set_print_execution_time_enabled);
	godot::ClassDB::bind_method(godot::D_METHOD("is_print_execution_time_enabled"), &InteractiveGrid3D::is_print_execution_time_enabled);

	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "rows"), "set_rows", "get_rows");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "columns"), "set_columns", "get_columns");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::VECTOR2, "cell_size"), "set_cell_size", "get_cell_size");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "cell_mesh", godot::PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_cell_mesh", "get_cell_mesh");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "cell_shape", godot::PROPERTY_HINT_RESOURCE_TYPE, "Shape3D"), "set_cell_shape", "get_cell_shape");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::VECTOR3, "cell_shape_offset"), "set_cell_shape_offset", "get_cell_shape_offset");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "walkable_color"), "set_walkable_color", "get_walkable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "unwalkable_color"), "set_unwalkable_color", "get_unwalkable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "unreachable_color"), "set_unreachable_color", "get_unreachable_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "selected_color"), "set_selected_color", "get_selected_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "path_color"), "set_path_color", "get_path_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::COLOR, "hovered_color"), "set_hovered_color", "get_hovered_color");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::ARRAY, "custom_cells_data", godot::PROPERTY_HINT_RESOURCE_TYPE, "CustomCellData"), "set_custom_cells_data", "get_custom_cells_data");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "material_override", godot::PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material_override", "get_material_override");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "layout", godot::PROPERTY_HINT_ENUM, "SQUARE, HEXAGONAL"), "set_layout", "get_layout");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "movement", godot::PROPERTY_HINT_ENUM, "FOUR-DIRECTIONS,SIX-DIRECTIONS,EIGH-DIRECTIONS"), "set_movement", "get_movement");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "obstacles_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_obstacles_collision_masks", "get_obstacles_collision_masks");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "floor_collision_masks", godot::PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_grid_floor_collision_masks", "get_grid_floor_collision_masks");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "print_logs_enabled"), "set_print_logs_enabled", "is_print_logs_enabled");
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::BOOL, "print_execution_time_enabled"), "set_print_execution_time_enabled", "is_print_execution_time_enabled");

	BIND_ENUM_CONSTANT(LAYOUT_SQUARE);
	BIND_ENUM_CONSTANT(LAYOUT_HEXAGONAL);

	BIND_ENUM_CONSTANT(MOVEMENT_FOUR_DIRECTIONS);
	BIND_ENUM_CONSTANT(MOVEMENT_SIX_DIRECTIONS);
	BIND_ENUM_CONSTANT(MOVEMENT_EIGH_DIRECTIONS);
}

void InteractiveGrid3D::_ready() {
}

void InteractiveGrid3D::_physics_process(double p_delta) {
	_create();

	if (godot::Engine::get_singleton()->is_editor_hint()) {
		if (data.center_global_position != get_global_transform().origin) {
			_delete();
		}
	}
}

void InteractiveGrid3D::set_rows(int p_rows) {
	data.rows = p_rows;
	_delete();
}

int InteractiveGrid3D::get_rows(void) const {
	return data.rows;
}

void InteractiveGrid3D::set_columns(int p_columns) {
	data.columns = p_columns;
	_delete();
}

int InteractiveGrid3D::get_columns() const {
	return data.columns;
}

void InteractiveGrid3D::set_cell_size(godot::Vector2 p_cell_size) {
	data.cell_size = p_cell_size;
	_delete();
}

godot::Vector2 InteractiveGrid3D::get_cell_size(void) const {
	return data.cell_size;
}

int InteractiveGrid3D::get_size() const {
	return data.rows * data.columns;
	;
}

void InteractiveGrid3D::set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh) {
	if (p_mesh == data.cell_mesh) {
		return;
	}

	data.cell_mesh = p_mesh;
	_delete();
}

godot::Ref<godot::Mesh> InteractiveGrid3D::get_cell_mesh() const {
	return data.cell_mesh;
}

void InteractiveGrid3D::set_cell_shape(const godot::Ref<godot::Shape3D> &p_shape) {
	if (p_shape == data.cell_shape) {
		return;
	}

	data.cell_shape = p_shape;
	_delete();
}

godot::Ref<godot::Shape3D> InteractiveGrid3D::get_cell_shape() const {
	return data.cell_shape;
}

void InteractiveGrid3D::set_cell_shape_offset(godot::Vector3 p_offset) {
	data.cell_shape_offset = p_offset;
}

godot::Vector3 InteractiveGrid3D::get_cell_shape_offset() {
	return data.cell_shape_offset;
}

void InteractiveGrid3D::InteractiveGrid3D::set_layout(Layout p_layout) {
	data.layout_index = p_layout;
	_delete();
}

InteractiveGrid3D::Layout InteractiveGrid3D::get_layout() const {
	return data.layout_index;
}

void InteractiveGrid3D::set_movement(Movement p_movement) {
	data.movement = p_movement;
}

InteractiveGrid3D::Movement InteractiveGrid3D::get_movement() const {
	return data.movement;
}

void InteractiveGrid3D::set_walkable_color(const godot::Color &p_color) {
	data.walkable_color = p_color;
	_delete();
}

godot::Color InteractiveGrid3D::get_walkable_color() const {
	return data.walkable_color;
}

void InteractiveGrid3D::set_unwalkable_color(const godot::Color &p_color) {
	data.unwalkable_color = p_color;
	_delete();
}

godot::Color InteractiveGrid3D::get_unwalkable_color() const {
	return data.unwalkable_color;
}

void InteractiveGrid3D::set_unreachable_color(const godot::Color &p_color) {
	data.unreachable_color = p_color;
}

godot::Color InteractiveGrid3D::get_unreachable_color() const {
	return data.unreachable_color;
}

void InteractiveGrid3D::set_selected_color(const godot::Color &p_color) {
	data.selected_color = p_color;
}

godot::Color InteractiveGrid3D::get_selected_color() const {
	return data.selected_color;
}

void InteractiveGrid3D::set_path_color(const godot::Color &p_color) {
	data.path_color = p_color;
}

godot::Color InteractiveGrid3D::get_path_color() const {
	return data.path_color;
}

void InteractiveGrid3D::set_hovered_color(const godot::Color &p_color) {
	data.hovered_color = p_color;
}

godot::Color InteractiveGrid3D::get_hovered_color() const {
	return data.hovered_color;
}

void InteractiveGrid3D::set_custom_cells_data(const godot::Array &p_custom_cell_data) {
	data.custom_cell_data = p_custom_cell_data;
}

godot::Array InteractiveGrid3D::get_custom_cells_data() const {
	return data.custom_cell_data;
}

void InteractiveGrid3D::add_custom_cell_data(int p_cell_index, godot::String p_custom_data_name) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	godot::Ref<CustomCellData> custom_cell_data;

	for (int index = 0; index < data.custom_cell_data.size(); index++) {
		custom_cell_data = data.custom_cell_data.get(index);

		if (custom_cell_data.is_null()) {
			godot::print_error("custom_cell_data is NULL at index: ", p_cell_index);
			continue;
		}

		if (p_custom_data_name != custom_cell_data->get_custom_data_name()) {
			continue;
		}

		data.cells.write[p_cell_index]->custom_flags |= custom_cell_data->get_layer_mask();
		data.cells.write[p_cell_index]->flags |= custom_cell_data->get_layer_mask();

		if (custom_cell_data->get_custom_color_enabled()) {
			data.cells.write[p_cell_index]->has_custom_color = true;
			data.cells.write[p_cell_index]->custom_color = custom_cell_data->get_color();
			set_cell_color(p_cell_index, data.cells[p_cell_index]->custom_color);
		}
	}
}

bool InteractiveGrid3D::has_custom_cell_data(int p_cell_index, godot::String p_custom_data_name) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return false;
	}

	godot::Ref<CustomCellData> custom_cell_data;

	for (int index = 0; index < data.custom_cell_data.size(); index++) {
		custom_cell_data = data.custom_cell_data.get(index);

		if (custom_cell_data.is_null()) {
			godot::print_error("custom_cell_data is NULL at index: ", p_cell_index);
			continue;
		}

		if (p_custom_data_name != custom_cell_data->get_custom_data_name()) {
			continue;
		}

		uint32_t cell_flags = data.cells[p_cell_index]->flags;
		uint32_t custom_cell_data_flags = custom_cell_data->get_layer_mask();

		if ((cell_flags & custom_cell_data_flags) == custom_cell_data_flags) {
			return true;
		}
	}

	return false;
}

void InteractiveGrid3D::clear_custom_cell_data(int p_cell_index, godot::String p_custom_data_name, bool p_clear_custom_color) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	godot::Ref<CustomCellData> custom_cell_data;

	for (int index = 0; index < data.custom_cell_data.size(); index++) {
		custom_cell_data = data.custom_cell_data.get(index);

		if (custom_cell_data.is_null()) {
			godot::print_error("custom_cell_data is NULL at index: ", p_cell_index);
			continue;
		}

		if (p_custom_data_name != custom_cell_data->get_custom_data_name()) {
			continue;
		}

		if (p_clear_custom_color) {
			data.cells.write[p_cell_index]->has_custom_color = false;
			set_cell_color(p_cell_index, data.walkable_color);
		}

		break;
	}
}

void InteractiveGrid3D::clear_all_custom_cell_data(int p_cell_index) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	data.cells.write[p_cell_index]->flags &= ~data.cells[p_cell_index]->custom_flags;
	data.cells.write[p_cell_index]->custom_flags = 0;
	data.cells.write[p_cell_index]->has_custom_color = false;
	set_cell_color(p_cell_index, data.walkable_color);
}

void InteractiveGrid3D::set_material_override(const godot::Ref<godot::Material> &p_material) {
	data.material_override = p_material;
	_delete();
}

godot::Ref<godot::Material> InteractiveGrid3D::get_material_override() const {
	return data.material_override;
}

void InteractiveGrid3D::apply_default_material() {
	if (data.multimesh_instance == nullptr) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "No MultiMeshInstance found.");
		return;
	}

	godot::Ref<godot::Shader> shader;
	shader.instantiate();
	shader->set_code(default_shader_code);

	godot::Ref<godot::ShaderMaterial> shader_material;
	shader_material.instantiate();
	shader_material->set_shader(shader);

	data.multimesh_instance->set_material_override(shader_material);

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Default ShaderMaterial created and applied.");
	}
}

void InteractiveGrid3D::highlight_on_hover(godot::Vector3 p_global_position) {
	if (is_visible() == false) {
		return;
	}

	if (is_centered() == false) {
		return;
	}

	if (is_hover_enabled() == false) {
		return;
	}

	int closest_index = get_cell_index_from_global_position(p_global_position);

	if (closest_index == -1 || !is_cell_visible(closest_index)) {
		if (data.hovered_cell_index > -1) {
			_set_cell_hovered(data.hovered_cell_index, false);

			if (!is_cell_selected(data.hovered_cell_index)) {
				if (data.cells[data.hovered_cell_index]->has_custom_color) {
					set_cell_color(data.hovered_cell_index, data.cells[data.hovered_cell_index]->custom_color);
				} else {
					set_cell_color(data.hovered_cell_index, data.walkable_color);
				}
			}

			data.hovered_cell_index = -1;
		}
		return;
	}

	if (closest_index == data.hovered_cell_index) {
		return;
	}

	if (data.hovered_cell_index > -1) {
		_set_cell_hovered(data.hovered_cell_index, false);

		if (!is_cell_selected(data.hovered_cell_index)) {
			if (data.cells[data.hovered_cell_index]->has_custom_color) {
				set_cell_color(data.hovered_cell_index, data.cells[data.hovered_cell_index]->custom_color);
			} else {
				set_cell_color(data.hovered_cell_index, data.walkable_color);
			}
		}

		data.hovered_cell_index = -1;
	}

	if (!is_cell_walkable(closest_index)) {
		return;
	}

	if (!is_cell_reachable(closest_index)) {
		return;
	}

	if (!is_cell_selected(closest_index)) {
		data.hovered_cell_index = closest_index;
		_set_cell_hovered(data.hovered_cell_index, true);
	}
}

void InteractiveGrid3D::highlight_path(const godot::PackedInt64Array &p_path) {
	for (int step = 0; step < p_path.size(); step++) {
		int cell_index = p_path[step];
		_set_cell_on_path(cell_index, true);
	}
}

godot::Vector3 InteractiveGrid3D::get_cell_global_position(int p_cell_index) const {
	godot::Vector3 cell_global_position = data.cells[p_cell_index]->global_xform.origin;
	return cell_global_position;
}

int InteractiveGrid3D::get_cell_index_from_global_position(godot::Vector3 p_global_position) const {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created.");
		return -1;
	}

	if (!data.multimesh.is_valid()) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid multimesh is not valid.");
		return -1;
	}

	float center_to_edge_x{ 0.0f }, center_to_edge_z{ 0.0f };
	godot::Vector3 top_left_global_position;

	switch (data.layout_index) {
		case Layout::LAYOUT_SQUARE:

			center_to_edge_x = (data.columns / 2) * data.cell_size.x + data.cell_size.x / 2;
			center_to_edge_z = (data.rows / 2) * data.cell_size.y + data.cell_size.y / 2;

			top_left_global_position.x = data.center_global_position.x - center_to_edge_x;
			top_left_global_position.z = data.center_global_position.z - center_to_edge_z;

			if (!(data.rows % 2)) { // Even.
				if (p_global_position.x > (data.center_global_position.x + center_to_edge_x - data.cell_size.x) || p_global_position.x < (data.center_global_position.x - center_to_edge_x)) {
					return -1;
				}
				if (p_global_position.z > (data.center_global_position.z + center_to_edge_z - data.cell_size.y) || p_global_position.z < (data.center_global_position.z - center_to_edge_z)) {
					return -1;
				}
			} else {
				if (p_global_position.x > (data.center_global_position.x + center_to_edge_x) || p_global_position.x < (data.center_global_position.x - center_to_edge_x)) {
					return -1;
				}
				if (p_global_position.z > (data.center_global_position.z + center_to_edge_z) || p_global_position.z < (data.center_global_position.z - center_to_edge_z)) {
					return -1;
				}
			}
			break;
		case Layout::LAYOUT_HEXAGONAL:

			const float hex_short_diagonal = data.cell_size.x; // s = a · √3
			const float hex_side_length = hex_short_diagonal / sqrt(3); // a = s / √3.
			const float hex_side_to_side = data.cell_size.x / 2;
			const float hex_inradius = hex_side_length * sqrt(3) / 2; // r = a · √3 / 2.

			float center_to_edge_x = (data.columns / 2) * data.cell_size.x;
			float center_to_edge_z = (data.rows / 2) * data.cell_size.y;

			if (data.rows % 2) { // Odd.
				center_to_edge_z += hex_side_length;
			}

			top_left_global_position.x = data.center_global_position.x - center_to_edge_x;
			top_left_global_position.z = data.center_global_position.z - center_to_edge_z;

			if (p_global_position.x < (top_left_global_position.x - hex_side_to_side)) {
				return -1;
			}

			if (p_global_position.x > (top_left_global_position.x + center_to_edge_x * 2 + data.cell_size.x)) {
				return -1;
			}

			if (p_global_position.z < top_left_global_position.z) {
				return -1;
			}

			if (p_global_position.z > (top_left_global_position.z + center_to_edge_z * 2)) {
				return -1;
			}
	}

	float closest_distance = std::numeric_limits<float>::max();
	int closest_index = -1;

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index =
					row * data.columns + column;

			const godot::Vector3 cell_pos = data.cells[index]->global_xform.origin;
			const float distance = p_global_position.distance_to(cell_pos);

			if (distance < closest_distance) {
				closest_distance = distance;
				closest_index = index;
			}
		}
	}

	return closest_index;
}

godot::Vector3 InteractiveGrid3D::get_grid_center_global_position() const {
	return data.center_global_position;
}

void InteractiveGrid3D::center(godot::Vector3 p_center_position) {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	data.flags &= ~GFL_CENTERED;

	set_hover_enabled(false);
	reset_cells_state();
	_layout(p_center_position);
	_align_cells_with_floor();
	_scan_environnement_obstacles();
	_scan_environnement_custom_data();
	_configure_astar();

	for (int cell_index = 0; cell_index < get_size(); cell_index++) {
		if (data.material_override.is_valid()) {
			uint32_t cell_flags = data.cells[cell_index]->flags;
			godot::Color current_cell_color = data.cells[cell_index]->color;
			godot::Color new_cell_color{ current_cell_color.r, current_cell_color.g, current_cell_color.b, static_cast<float>(cell_flags) };
			data.cells.write[cell_index]->color = new_cell_color;
			data.multimesh->set_instance_custom_data(cell_index, new_cell_color);
		}
	}

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

void InteractiveGrid3D::update_custom_data() {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	set_hover_enabled(false);
	_scan_environnement_custom_data();
	_configure_astar();

	for (int cell_index = 0; cell_index < get_size(); cell_index++) {
		if (data.material_override.is_valid()) {
			uint32_t cell_flags = data.cells[cell_index]->flags;
			godot::Color current_cell_color = data.cells[cell_index]->color;
			godot::Color new_cell_color{ current_cell_color.r, current_cell_color.g, current_cell_color.b, static_cast<float>(cell_flags) };
			data.cells.write[cell_index]->color = new_cell_color;
			data.multimesh->set_instance_custom_data(cell_index, new_cell_color);
		}
	}

	set_hover_enabled(true);

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}

	if (_debug_options.print_logs_enabled) {
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Grid centered.");
	}
}

void InteractiveGrid3D::compute_unreachable_cells(int p_start_cell_index) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_start_cell_index)) {
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	if ((is_visible()) && !(data.flags & GFL_CELL_UNREACHABLE_HIDDEN)) {
		_configure_astar();
		_breadth_first_search(p_start_cell_index);
		data.flags |= GFL_CELL_UNREACHABLE_HIDDEN;
	}

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}
}

void InteractiveGrid3D::hide_distant_cells(int p_start_cell_index, float p_distance) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_start_cell_index)) {
		return;
	}

	if ((is_visible()) && !(data.flags & GFL_CELL_DISTANT_HIDDEN)) {
		for (int row = 0; row < data.rows; row++) {
			for (int column = 0; column < data.columns; column++) {
				const int index = row * data.columns + column;

				godot::Vector3 start_cell_position = data.cells[p_start_cell_index]->global_xform.origin;
				godot::Vector3 index_cell_position = data.cells[index]->global_xform.origin;

				if (start_cell_position.distance_to(index_cell_position) > p_distance) {
					set_cell_visible(index, false);
					data.cells.write[index]->flags &= ~CFL_WALKABLE;
				}
			}
		}
		data.flags |= GFL_CELL_DISTANT_HIDDEN;
	}
}

void InteractiveGrid3D::set_hover_enabled(bool p_enabled) {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return;
	}

	if (p_enabled) {
		data.flags |= GFL_HOVER_ENABLED;
	} else {
		data.flags &= ~GFL_HOVER_ENABLED;
	}
}

bool InteractiveGrid3D::is_hover_enabled() const {
	return (data.flags & GFL_HOVER_ENABLED) != 0;
}

bool InteractiveGrid3D::is_created() const {
	return (data.flags & GFL_CREATED) != 0;
}

bool InteractiveGrid3D::is_centered() const {
	return (data.flags & GFL_CENTERED) != 0;
}

bool InteractiveGrid3D::is_cell_walkable(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_WALKABLE) != 0;
}

bool InteractiveGrid3D::is_cell_reachable(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_REACHABLE) != 0;
}

bool InteractiveGrid3D::is_cell_in_void(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_IN_VOID) != 0;
}

bool InteractiveGrid3D::is_cell_hovered(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_HOVERED) != 0;
}

bool InteractiveGrid3D::is_cell_selected(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_SELECTED) != 0;
}

bool InteractiveGrid3D::is_cell_on_path(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_PATH) != 0;
}

bool InteractiveGrid3D::is_cell_visible(int p_cell_index) const {
	return (data.cells[p_cell_index]->flags & CFL_VISIBLE) != 0;
}

void InteractiveGrid3D::set_cell_walkable(int p_cell_index, bool p_is_walkable) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_walkable) {
		data.cells.write[p_cell_index]->flags |= CFL_WALKABLE;
		set_cell_color(p_cell_index, data.walkable_color);
	} else if (!p_is_walkable) {
		data.cells.write[p_cell_index]->flags &= ~CFL_WALKABLE;
		set_cell_color(p_cell_index, data.unwalkable_color);
	}
}

void InteractiveGrid3D::set_cell_reachable(int p_cell_index, bool p_is_reachable) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (p_is_reachable) {
		data.cells.write[p_cell_index]->flags |= CFL_REACHABLE;
	} else if (!p_is_reachable) {
		data.cells.write[p_cell_index]->flags &= ~CFL_REACHABLE;
		set_cell_color(p_cell_index, data.unreachable_color);
	}
}

void InteractiveGrid3D::set_cell_visible(int p_cell_index, bool p_is_visible) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	godot::Color current_cell_color = data.cells[p_cell_index]->color;

	if (p_is_visible) {
		data.cells.write[p_cell_index]->flags |= CFL_VISIBLE;
		set_cell_color(p_cell_index, current_cell_color);
	} else if (!p_is_visible) {
		current_cell_color.a = 0.0;
		data.multimesh->set_instance_custom_data(p_cell_index, current_cell_color);
		data.cells.write[p_cell_index]->flags &= ~CFL_VISIBLE;
	}
}

void InteractiveGrid3D::InteractiveGrid3D::reset_cells_state() {
	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return;
	}

	for (int row = 0; row < data.rows; row++) {
		for (int column = 0; column < data.columns; column++) {
			const int index = row * data.columns + column;
			clear_all_custom_cell_data(index);
			data.cells.write[index]->flags = 0;
		}
	}

	data.flags &= ~GFL_CELL_UNREACHABLE_HIDDEN;
	data.flags &= ~GFL_CELL_DISTANT_HIDDEN;

	data.hovered_cell_index = -1;
	data.selected_cells.clear();
}

void InteractiveGrid3D::set_cell_color(int p_cell_index, const godot::Color &p_color) {
	if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	if (data.material_override.is_valid()) {
		uint32_t cell_flags = data.cells[p_cell_index]->flags;
		godot::Color new_cell_color{ p_color.r, p_color.g, p_color.b, static_cast<float>(cell_flags) };
		data.cells.write[p_cell_index]->color = new_cell_color;
		data.multimesh->set_instance_custom_data(p_cell_index, new_cell_color);
	} else {
		data.cells.write[p_cell_index]->color = p_color;
		data.multimesh->set_instance_custom_data(p_cell_index, p_color);
	}
}

void InteractiveGrid3D::set_obstacles_collision_masks(int p_mask) {
	data.obstacles_collision_masks = p_mask;
}

int InteractiveGrid3D::get_obstacles_collision_masks() {
	return data.obstacles_collision_masks;
}

void InteractiveGrid3D::set_grid_floor_collision_mask(int p_mask) {
	data.floor_collision_mask = p_mask;
}

int InteractiveGrid3D::get_grid_floor_collision_mask() {
	return data.floor_collision_mask;
}

void InteractiveGrid3D::select_cell(int p_cell_index) {
	if (is_visible() == false) {
		return;
	}

	if (p_cell_index == -1) {
		return;
	} else if (is_cell_index_out_of_bounds(__FILE__, __FUNCTION__, __LINE__, p_cell_index)) {
		return;
	}

	bool visible = is_cell_visible(p_cell_index);
	if (!visible) {
		return;
	}

	bool unreachable = !is_cell_reachable(p_cell_index);
	if (unreachable) {
		return;
	}

	bool walkable = is_cell_walkable(p_cell_index);
	if (walkable) {
		_set_cell_selected(p_cell_index, true);
		data.selected_cells.push_back(p_cell_index);
	}
}

godot::Array InteractiveGrid3D::get_selected_cells() {
	return data.selected_cells;
}

int InteractiveGrid3D::get_latest_selected() const {
	return data.selected_cells.back();
}

godot::PackedInt64Array InteractiveGrid3D::get_path(int p_start_cell_index, int p_target_cell_index) const {
	godot::PackedInt64Array path;

	if (!(data.flags & GFL_CREATED)) {
		PrintError(__FILE__, __FUNCTION__, __LINE__, "The grid has not been created");
		return path;
	}

	auto start = std::chrono::high_resolution_clock::now();

	path = data.astar->get_id_path(p_start_cell_index, p_target_cell_index);

	auto end = std::chrono::high_resolution_clock::now();

	if (_debug_options.print_execution_time_enabled) {
		std::chrono::duration<double, std::milli> duration = end - start;
		PrintLine(__FILE__, __FUNCTION__, __LINE__, "Execution time (ms): ", duration.count());
	}

	return path;
}

godot::Array InteractiveGrid3D::get_neighbors(int p_cell_index) const {
	return data.cells[p_cell_index]->neighbors;
}

void InteractiveGrid3D::set_print_logs_enabled(bool p_enabled) {
	_debug_options.print_logs_enabled = p_enabled;
}

bool InteractiveGrid3D::is_print_logs_enabled() const {
	return _debug_options.print_logs_enabled;
}

void InteractiveGrid3D::set_print_execution_time_enabled(bool p_enabled) {
	_debug_options.print_execution_time_enabled = p_enabled;
}

bool InteractiveGrid3D::is_print_execution_time_enabled() const {
	return _debug_options.print_execution_time_enabled;
}

bool InteractiveGrid3D::is_cell_index_out_of_bounds(const godot::String &p_file, const godot::String &p_func, int p_line, int p_cell_index) {
	bool is_out_of_bounds = false;
	unsigned int grid_size = data.rows * data.columns;

	if (p_cell_index >= (grid_size)) {
		PrintError(p_file, p_func, p_line, "Cell index out of bounds: ", p_cell_index, " >= ", (grid_size));
		is_out_of_bounds = true;
	}

	return is_out_of_bounds;
}

InteractiveGrid3D::InteractiveGrid3D() {}

InteractiveGrid3D::~InteractiveGrid3D() {
	_delete();
}