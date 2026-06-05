#pragma once

#include "common.h"
#include "custom_cell_data.h"

#include <godot_cpp/classes/a_star2d.hpp>
#include <godot_cpp/classes/canvas_item_material.hpp>
#include <godot_cpp/classes/collision_object2d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/mesh_instance2d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance2d.hpp>
#include <godot_cpp/classes/physics_direct_space_state2d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters2d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters2d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/shape2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/world2d.hpp>
#include <godot_cpp/core/class_db.hpp>

class InteractiveGrid2D : public godot::Node2D {
	GDCLASS(InteractiveGrid2D, godot::Node2D);

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
		bool logs_enabled = false;
		bool execution_time_enabled = false;
	} _debug_options;

private:
	struct Cell {
		int index = -1;
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

		godot::Transform2D last_xform;

		godot::Ref<godot::AStar2D> astar;

		Layout layout_index = LAYOUT_SQUARE;
		Movement movement = MOVEMENT_FOUR_DIRECTIONS;
		bool obstacles_collision_enabled = false;
		bool floor_collision_enabled = false;
		uint32_t obstacles_collision_mask = 1;
		uint32_t floor_collision_mask = 1;

		godot::Ref<godot::Mesh> cell_mesh;
		godot::Ref<godot::Texture2D> cell_texture;
		godot::Ref<godot::Shape2D> cell_shape;
		godot::Vector2 cell_shape_offset = godot::Vector2(0.0f, 0.0f);
		godot::Vector3 cell_rotation = godot::Vector3(0.0f, 0.0f, 0.0f);
		godot::MultiMeshInstance2D *multimesh_instance;
		godot::Ref<godot::MultiMesh> multimesh;
		godot::Vector2 cell_size = godot::Vector2(64.0f, 64.0f);
		godot::Vector<Cell *> cells;

		godot::Array custom_cell_data;

		godot::Array selected_cells;
		int hovered_cell_index = -1;

		godot::Color accessible_color{ godot::Color(1.0f, 1.0f, 1.0f, 1.0f) };
		godot::Color unaccessible_color{ godot::Color(1.0f, 1.0f, 1.0f, 1.0f) };
		godot::Color unreachable_color{ godot::Color(1.0f, 1.0f, 1.0f, 1.0f) };
		godot::Color selected_color{ godot::Color(1.0f, 1.0f, 1.0f, 1.0f) };
		godot::Color path_color{ godot::Color(1.0f, 1.0f, 1.0, 1.0f) };
		godot::Color hovered_color{ godot::Color(1.0f, 1.0f, 1.0f, 1.0f) };

		godot::Ref<godot::Material> material_override;

	} data;

	static constexpr int GFL_CREATED = 1 << 0;
	static constexpr int GFL_CENTERED = 1 << 1;
	static constexpr int GFL_CELL_UNREACHABLE_HIDDEN = 1 << 2;
	static constexpr int GFL_CELL_DISTANT_HIDDEN = 1 << 3;
	static constexpr int GFL_HOVER_ENABLED = 1 << 4;

	static constexpr int CFL_ACCESSIBLE = 1 << 0;
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

	void _layout(godot::Vector2 p_center_position);
	void _layout_cells_as_square_grid(godot::Vector2 p_center_position);
	void _layout_cells_as_hexagonal_grid(godot::Vector2 p_center_position);

	void _align_cells_with_floor();
	void _scan_environnement_obstacles();
	void _scan_environnement_custom_data();

	void _configure_astar();
	void _configure_astar_4_dir();
	void _configure_astar_6_dir();
	void _configure_astar_8_dir();
	void _breadth_first_search(int p_start_cell_index);

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

	int get_size() const;

	void set_cell_size(godot::Vector2 p_cell_size);
	godot::Vector2 get_cell_size() const;

	void set_cell_mesh(const godot::Ref<godot::Mesh> &p_mesh);
	godot::Ref<godot::Mesh> get_cell_mesh() const;

	void set_cell_texture(const godot::Ref<godot::Texture2D> &p_texture);
	godot::Ref<godot::Texture2D> get_cell_texture() const;

	void set_cell_shape(const godot::Ref<godot::Shape2D> &p_shape);
	godot::Ref<godot::Shape2D> get_cell_shape() const;

	void set_cell_shape_offset(godot::Vector2 p_offset);
	godot::Vector2 get_cell_shape_offset();

	void set_cell_rotation(godot::Vector3 p_rotation);
	godot::Vector3 get_cell_rotation() const;

	void set_layout(Layout p_layout);
	Layout get_layout() const;

	void set_movement(Movement p_movement);
	Movement get_movement() const;

	void set_accessible_color(const godot::Color &p_color);
	godot::Color get_accessible_color() const;

	void set_unaccessible_color(const godot::Color &p_color);
	godot::Color get_unaccessible_color() const;

	void set_unreachable_color(const godot::Color &p_color);
	godot::Color get_unreachable_color() const;

	void set_selected_color(const godot::Color &p_color);
	godot::Color get_selected_color() const;

	void set_path_color(const godot::Color &p_color);
	godot::Color get_path_color() const;

	void set_hovered_color(const godot::Color &p_color);
	godot::Color get_hovered_color() const;

	void set_cell_color(int cell_index, const godot::Color &p_color);
	godot::Color get_cell_color(int cell_index);

	void set_custom_cells_data(const godot::Array &p_custom_cell_data);
	godot::Array get_custom_cells_data() const;

	void update_custom_data();

	void add_custom_cell_data(int p_cell_index, godot::String p_custom_data_name);
	bool has_custom_cell_data(int p_cell_index, godot::String p_custom_data_name);
	void clear_custom_cell_data(int p_cell_index, godot::String p_custom_data_name, bool p_clear_custom_color);
	void clear_all_custom_cell_data(int p_cell_index);

	void set_material_override(const godot::Ref<godot::Material> &p_material);
	godot::Ref<godot::Material> get_material_override() const;
	void apply_default_material();

	void highlight_on_hover(godot::Vector2 p_global_position);
	void highlight_path(const godot::PackedInt64Array &p_path);

	godot::MultiMeshInstance2D *get_multimesh_instance();
	godot::Ref<godot::MultiMesh> get_multimesh() const;
	godot::Vector2 get_cell_position(int p_cell_index) const;
	godot::Vector2 get_cell_global_position(int p_cell_index) const;
	int get_cell_index_from_global_position(godot::Vector2 p_global_position) const;
	godot::Transform2D get_cell_transform(int p_cell_index) const;
	godot::Transform2D get_cell_global_transform(int p_cell_index) const;

	void center(godot::Vector2 p_center_position);

	void compute_unreachable_cells(int p_start_cell_index);

	void hide_distant_cells(int p_start_cell_index, float p_distance);
	void set_hover_enabled(bool p_enabled);
	bool is_hover_enabled() const;

	bool is_created() const;
	bool is_centered() const;

	bool is_cell_accessible(int p_cell_index) const;
	bool is_cell_reachable(int p_cell_index) const;
	bool is_cell_in_void(int p_cell_index) const;
	bool is_cell_hovered(int p_cell_index) const;
	bool is_cell_selected(int p_cell_index) const;
	bool is_cell_on_path(int p_cell_index) const;
	bool is_cell_visible(int p_cell_index) const;

	void set_cell_accessible(int p_cell_index, const bool p_is_accessible);
	void set_cell_reachable(int p_cell_index, bool p_is_unreachable);
	void set_cell_visible(int p_cell_index, const bool p_is_visible);

	void reset_cells_state();

	void set_obstacles_collision_enabled(bool p_enabled);
	bool get_obstacles_collision_enabled() const;

	void set_floor_collision_enabled(bool p_enabled);
	bool get_floor_collision_enabled() const;

	void set_obstacles_collision_mask(int p_mask);
	int get_obstacles_collision_mask() const;

	void set_floor_collision_mask(int p_mask);
	int get_floor_collision_mask() const;

	void select_cell(int p_cell_index);
	godot::Array get_selected_cells();
	int get_latest_selected() const;
	godot::PackedInt64Array get_path(int p_start_cell_index, int p_target_cell_index) const;
	godot::Array get_neighbors(int p_cell_index) const;

	void set_logs_enabled(bool p_enabled);
	bool is_logs_enabled() const;

	void set_execution_time_enabled(bool p_enabled);
	bool is_execution_time_enabled() const;

	InteractiveGrid2D();
	~InteractiveGrid2D();
};

VARIANT_ENUM_CAST(InteractiveGrid2D::Layout);
VARIANT_ENUM_CAST(InteractiveGrid2D::Movement);