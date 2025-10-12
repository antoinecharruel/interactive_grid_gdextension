# =================================================================================================
# File: interactive_grid.gd
#
# Summary: Script extending InteractiveGrid to handle player interaction with the grid.
#
# Node: InteractiveGrid (InteractiveGrid).
#
# Last modified: October 11, 2025
#
# This file is part of the InteractiveGrid GDExtension Source Code.
# Repository: https://github.com/antoinecharruel/interactive_grid_gdextension
#
# Version InteractiveGrid: 1.0.0
# Version: Godot Engine v4.5.stable.steam - https://godotengine.org
#
# Author: Antoine Charruel
# =================================================================================================

extends InteractiveGrid

@onready var pawn_player: CharacterBody3D = $"../PawnPlayer"
@onready var player_pawn_collision_shape_3d: CollisionShape3D = $"../PawnPlayer/PlayerPawnCollisionShape3D"

@onready var ray_cast_from_mouse: RayCast3D = $"../PawnPlayer/RayCastFromMouse"
@onready var camera_3d: Camera3D = $"../PawnPlayer/Camera3D"

@onready var try_me: Control = $"../TryMe"

var _is_grid_open: bool = false

func _ready() -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called when the node enters the scene tree for the first time.
	#
	# Last Modified: October 04, 2025
	pass
	# ----------------------------------------------------------------------------------------F-F*/

func _process(delta: float) -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called every frame. 'delta' is the elapsed time since the previous frame.
	#
	# Last Modified: October 11, 2025
	
	if pawn_player != null:
		# Highlight the cell under the mouse.
		if self.get_selected_cells().is_empty():
			self.highlight_on_hover(ray_cast_from_mouse.get_ray_intersection_position())
		else:
			var path: PackedInt64Array
			var selected_cells: Array = self.get_selected_cells()
			var index_pawn_cell: int = self.get_cell_index_from_global_position(self.get_grid_center_global_position())
			path = self.get_path(index_pawn_cell, selected_cells[0]) # only the first one.
			pawn_player.move_player_along_path(path)
	# ----------------------------------------------------------------------------------------F-F*/

func open_grid():
	if player_pawn_collision_shape_3d != null:

				# Makes the grid visible.
				self.set_visible(true)
				
				# Centers the grid.
				# ! Info: every time center is called, the state of the cells is reset.
				self.center(round(player_pawn_collision_shape_3d.global_position))
				
				_is_grid_open = true
				try_me.visible = false
	
func _input(event):
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Handles mouse input events for the InteractiveGrid.
	#
	# Last Modified: October 10, 2025
	
	if event.is_action_pressed("open_grid") && _is_grid_open == false:
	# --------------------------------------------------------------------
	#  SPACE BAR: Open the grid
	# --------------------------------------------------------------------
		open_grid()


	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
	# --------------------------------------------------------------------
	#  LEFT MOUSE CLICK.
	# --------------------------------------------------------------------

		if pawn_player == null:
			return
			
		var ray_pos = ray_cast_from_mouse.get_ray_intersection_position()
		if ray_pos == null:
			return
			
		var selected_cells: Array = self.get_selected_cells()
		
		if selected_cells.size() < 1:
			# Retrieve the selected cells.
			self.select_cell(ray_cast_from_mouse.get_ray_intersection_position())
			
			# Select a cell.
			if self.get_selected_cells().is_empty():
				return
			
			var index_pawn_cell: int = self.get_cell_index_from_global_position(self.get_grid_center_global_position())

			# Retrieve the path.
			var path: PackedInt64Array
			path = self.get_path(index_pawn_cell, selected_cells[0]) # only the first one.
			print("Last selected cell:", self.get_latest_selected())
			print("Path:", path)

			# Highlight the path.
			self.highlight_path(path)
	# ----------------------------------------------------------------------------------------F-F*/	

func _on_button_button_down() -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Handles mouse input events for the InteractiveGrid.
	#
	# Last Modified: October 10, 2025
	
	open_grid()
	# ----------------------------------------------------------------------------------------F-F*/	
