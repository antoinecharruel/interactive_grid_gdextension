# =================================================================================================
# File: interactive_grid.gd
#
# Summary: Script extending InteractiveGrid to handle player interaction with the grid.
#
# Node: InteractiveGrid (InteractiveGrid).
#
# Last modified: October 08, 2025
#
# This file is part of the InteractiveGrid GDExtension Source Code.
# Repository: https://github.com/antoinecharruel/interactive_grid_gdextension
#
# Version InteractiveGrid: 1.1.0
# Version: Godot Engine v4.5.stable.steam - https://godotengine.org
#
# Author: Antoine Charruel
# =================================================================================================

extends InteractiveGrid

@onready var pawn: CollisionShape3D = $"../Pawn"
@onready var ray_cast_from_mouse: RayCast3D = $"../RayCastFromMouse"
@onready var camera_3d: Camera3D = $"../Camera3D"

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
	# Last Modified: October 04, 2025
	
	if pawn != null:
		# Highlight the cell under the mouse.
		if self.get_selected_cells().is_empty():
			self.highlight_on_hover(ray_cast_from_mouse.get_ray_intersection_position())
	# ----------------------------------------------------------------------------------------F-F*/
	
func _input(event):
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Handles mouse input events for the InteractiveGrid.
	#
	# Last Modified: October 06, 2025
	
	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_RIGHT:
	# --------------------------------------------------------------------
	#  RIGHT MOUSE CLICK.
	# --------------------------------------------------------------------
		if event.pressed:
			print("Right button is held down at ", event.position)
			
			if pawn != null:
				# Makes the grid visible.
				self.set_grid_visible(true)
				# Centers the grid.
				# ! Info: every time center is called, the state of the cells is reset.
				self.center(pawn.global_position)
				
				var index_cell_pawn: int = self.get_cell_index_from_global_position(pawn.global_position)
				
				# Manually set cell as unwalkable.
				set_cell_walkable(75, false);
				
				# Check if the cell is walkable
				print("Cell 75 is walkable ? : ", is_cell_walkable(75))
				
 				# Hides distant cells.
				self.hide_distant_cells(index_cell_pawn, 6)	
				self.compute_inaccessible_cells(index_cell_pawn)
				
				# Manually set cell color.
				var couleur_cell = Color(0.3, 0.4, 0.9)
				self.set_cell_color(65, couleur_cell)
		else:
			print("Right button was released")


	if event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
	# --------------------------------------------------------------------
	#  LEFT MOUSE CLICK.
	# --------------------------------------------------------------------
		if event.pressed:
			print("Left button is held down at ", event.position)
			
			if pawn != null:
				# Select a cell.
				if self.get_selected_cells().is_empty():
					self.select_cell(ray_cast_from_mouse.get_ray_intersection_position())
				
				# Retrieve the selected cells.
				var selected_cells: Array = self.get_selected_cells()
				if selected_cells.size() > 0:
					print("Selected cell index: ", selected_cells[0])
					print("Selected cells: ", selected_cells)
					print("Position of the selected cell: ", self.get_cell_golbal_position(selected_cells[0]))

					var index_cell_pawn = self.get_cell_index_from_global_position(self.get_grid_center_position())
					print("Pawn index: ", index_cell_pawn)
					
					# Retrieve the path.
					var path: PackedInt64Array
					path = self.get_path(index_cell_pawn, selected_cells[0]) # only the first one.
					#path = self.get_path(index_cell_pawn, self.get_latest_selected()) # the last one.
					print("Last selected cell:", self.get_latest_selected())
					print("Path:", path)
					
					# Highlight the path.
					self.highlight_path(path)
		else:
			print("Right button was released")
	# ----------------------------------------------------------------------------------------F-F*/	
