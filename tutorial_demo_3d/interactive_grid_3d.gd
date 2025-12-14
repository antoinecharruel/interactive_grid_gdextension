# interactive_grid_3d.gd

extends InteractiveGrid3D

@onready var ray_cast_from_mouse: RayCast3D = $"../RayCastFromMouse"

var _path: PackedInt64Array = []
var _pawn: CharacterBody3D = null

@onready var debug_collision_shape_area_3d: CollisionShape3D = $"../DebugCollisionShapeArea3D/DebugCollisionShapeArea3D"

func _ready() -> void:
	pass


func _process(delta: float) -> void:
	if _pawn == null: 
		return

	if self.get_selected_cells().is_empty():
		self.highlight_on_hover(ray_cast_from_mouse.get_ray_intersection_position())
	else:
		move_along_path(_path)


func show_grid():
	#region InteractiveGrid3D Center
	## Here, the grid is centered around the player.
	## !Note: This operation repositions all cells, aligns them with the environment,
	## rescans obstacles and custom data, and refreshes A* navigation.
	##  - Manual modifications can also be applied here, such as:
	##     - Hiding cells beyond a certain distance
	##     - compute_unreachable_cells
	##     - Adding custom data
	#endregion
	
	if _pawn == null:
		return

	print("show_grid")

	_path = []
	self.set_visible(true)
	self.center(_pawn.global_position)

	var pawn_current_cell_index: int = self.get_cell_index_from_global_position(_pawn.global_position)

	# To prevent the player from getting stuck.
	self.set_cell_walkable(pawn_current_cell_index, true)
	self.set_cell_reachable(pawn_current_cell_index, true)

	self.hide_distant_cells(pawn_current_cell_index, 6)
	self.compute_unreachable_cells(pawn_current_cell_index)
	
	var cell_position: Vector3 = get_cell_global_position(pawn_current_cell_index)
	debug_collision_shape_area_3d.global_position = cell_position + get_cell_shape_offset()
	
	var neighbors: PackedInt64Array = self.get_neighbors(pawn_current_cell_index)
	for neighbor_index in neighbors:
		self.add_custom_cell_data(neighbor_index, "CFL_NEIGHBORS")

	self.add_custom_cell_data(pawn_current_cell_index, "CFL_PLAYER")

	#region update_custom_data()
	## !Note: Don't forget to call update_custom_data().
	## It refreshes custom_cell_flags, colors, and the A* configuration
	## based on the newly updated CellCustomData.
	#endregion
	self.update_custom_data()

func _input(event):
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
		if _pawn == null:
			return

		var ray_pos: Vector3 = ray_cast_from_mouse.get_ray_intersection_position()
		if ray_pos == null:
			return

		var selected_cells: Array = self.get_selected_cells()
		if selected_cells.size() < 1:
			var hit_cell_index: int = self.get_cell_index_from_global_position(ray_pos)
			self.select_cell(hit_cell_index)
			selected_cells = self.get_selected_cells()
			if selected_cells.is_empty():
				return

			var pawn_current_cell_index: int = self.get_cell_index_from_global_position(self.get_grid_center_global_position())
			self.set_cell_walkable(pawn_current_cell_index, true)
			_path = self.get_path(pawn_current_cell_index, selected_cells[0])
			print("Last selected cell:", self.get_latest_selected())
			print("Path:", _path)
			self.highlight_path(_path)


func move_along_path(path: PackedInt64Array)-> void:
	if path.is_empty():
		show_grid()
		return

	var target_cell_index: int = path[0]
	var target_global_position: Vector3 = get_cell_global_position(target_cell_index)
	if not is_on_target_cell(_pawn.global_position, target_global_position, 0.20):
		reaching_cell_target(target_cell_index, path)
	else:
		target_cell_reached()


func reaching_cell_target(target_cell_index: int, path: PackedInt64Array) -> void:
	if _path.size() > 0:
		var target_cell_global_position: Vector3 = self.get_cell_global_position(target_cell_index)
		if _pawn.has_method("move_to"):
			_pawn.move_to(target_cell_global_position)
		else:
			printerr("pawn does not have the 'move_to' method.")


func target_cell_reached():
	if not _path.is_empty():
		_path.remove_at(0)


static func is_on_target_cell(current_global_position: Vector3, target_global_position: Vector3, threshold: float) -> bool:
	return current_global_position.distance_to(target_global_position) <= threshold


func set_pawn(pawn: CharacterBody3D):
	_pawn = pawn
