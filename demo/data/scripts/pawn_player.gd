# =================================================================================================
# File: pawn_player.gd
#
# Summary: 
#
# Node: PlayerPawn (CharacterBody3D).
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

extends CharacterBody3D

@onready var interactive_grid: InteractiveGrid = $"../InteractiveGrid"
@onready var animation_player: AnimationPlayer = $model/AnimationPlayer
@onready var player_pawn_collision_shape_3d: CollisionShape3D = $PlayerPawnCollisionShape3D
@onready var model: Node3D = $model
@onready var try_me: Control = $"../TryMe"

const _SPEED:float = 5.0
const _JUMP_VELOCITY:float = 4.5
const _DISTANCE_THRESHOLD:float = 0.1

var _nb_cell_traveled:int = 0

# Enumerations containing the diferente states of the player.
enum _pawn_movements_states{
	IDLE,
	WALKING,
	RUN
}

# Default state for the character is idle.
var _pawn_curent_movements_states:int = _pawn_movements_states.IDLE

func _ready() -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called when the node enters the scene tree for the first time.
	#
	# Last Modified: October 04, 2025
	pass
	# ----------------------------------------------------------------------------------------F-F*/

func _physics_process(delta: float) -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called every frame. 'delta' is the elapsed time since the previous frame.
	#
	# Last Modified: October 10, 2025
	
	# Add the gravity.
	if not is_on_floor():
		self.velocity += get_gravity() * delta
	
	if self.velocity == Vector3.ZERO:
		if _pawn_curent_movements_states != _pawn_movements_states.IDLE:
			# Reset default state for the character (idle).
			_pawn_curent_movements_states = _pawn_movements_states.IDLE
		else:
			animation_player.play("idle", 0.2)
	# ----------------------------------------------------------------------------------------F-F*/
	
func move_player_to(x:float, z:float)-> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Moves the player toward the specified (x, z) destination. Updates the player's
	#          velocity and movement state, and handles animations accordingly.
	#
	# Last Modified: October 10, 2025
	
	# Retrieves the global position of the PawnPlayer.
	var pawn_global_position:Vector3 = self.player_pawn_collision_shape_3d.global_position
	
	# Calculate the direction towards the destination.
	var target_global_position: Vector3 = Vector3(x, pawn_global_position.y, z) # Keep the current heightle.
	var direction:Vector3 = (target_global_position - pawn_global_position).normalized()
	
	# Calculate the distance to travel.
	var distance_to_target: float = pawn_global_position.distance_to(target_global_position)

	# If the player is very close to the target, stop the movement.
	if distance_to_target <= 0:
		if _pawn_curent_movements_states != _pawn_movements_states.IDLE:
			_pawn_curent_movements_states = _pawn_movements_states.IDLE
	else:
		self.velocity = direction * _SPEED # Update the velocity.

		# Run.
		if _pawn_curent_movements_states != _pawn_movements_states.RUN:
			_pawn_curent_movements_states = _pawn_movements_states.RUN
		else:
			animation_player.play("run", 0.2)

		move_and_slide()
		model.look_at(direction + position)
	# ----------------------------------------------------------------------------------------F-F*/
	
func move_player_along_path(path: PackedInt64Array)-> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Moves the player along the specified path. If the player has not yet reached
	#          the target cell, progresses toward the next cell; otherwise, handles
	#          the actions for when the target is reached.
	#
	# Last Modified: October 10, 2025
	
	# Move the Player along the path.
	if not is_on_target_cell():
		reaching_cell_target(path) # Reach the target.
	else:
		target_reached() # The target has been reached.
	# ----------------------------------------------------------------------------------------F-F*/
	
func reaching_cell_target(path: PackedInt64Array)-> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Moves the player along the given path toward the target cell.
	#          Updates the number of cells traveled as the player progresses.
	#
	# Last Modified: October 10, 2025
	
	# Get how many cells have been traveled.
	var cells_traveled: int = get_how_many_cells_traveled()
	
	if path.size() > 0 and cells_traveled < path.size():
	
		var next_cell_index: int = path[cells_traveled+1]
		var next_cell_global_position: Vector3 = interactive_grid.get_cell_golbal_position(next_cell_index)
		
		# Move the player toward the target cell.
		move_player_to(next_cell_global_position.x, next_cell_global_position.z)

		if self.global_position.distance_to(next_cell_global_position) <= _DISTANCE_THRESHOLD:
			set_how_many_cells_traveled(cells_traveled + 1) # Increment the count.
	# ----------------------------------------------------------------------------------------F-F*/
	
func target_reached()-> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Handles the actions to perform when the player reaches the target.
	#          Stops the player, resets traveled cells, recenters the grid, and updates
	#          the grid.
	#
	# Last Modified: October 10, 2025
	
	self.velocity = Vector3.ZERO
	set_how_many_cells_traveled(0)
	
	if self.velocity == Vector3.ZERO:
		
		# Recenter and reset the grid.
		interactive_grid.center(round(self.player_pawn_collision_shape_3d.global_position))
		
		var index_cell_pawn: int = interactive_grid.get_cell_index_from_global_position(self.player_pawn_collision_shape_3d.global_position)
		
		# To prevent the player from getting stuck.
		interactive_grid.set_cell_walkable(index_cell_pawn, true)
		
		interactive_grid.hide_distant_cells(index_cell_pawn, 9)
		interactive_grid.compute_inaccessible_cells(index_cell_pawn)
	# ----------------------------------------------------------------------------------------F-F*/
	
func is_on_target_cell()-> bool:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Checks if the player is currently on the target cell. 
	#
	# Last Modified: October 10, 2025
	
	var is_on_target: bool = false
	var target_cell: Vector3
	var selected_cells: Array = interactive_grid.get_selected_cells()
	
	if selected_cells.size() > 0:
		target_cell = interactive_grid.get_cell_golbal_position(selected_cells[0])
	
	if self.global_position.distance_to(target_cell) <= _DISTANCE_THRESHOLD:
		is_on_target = true
	
	return is_on_target
	# ----------------------------------------------------------------------------------------F-F*/
	
func get_how_many_cells_traveled()-> int:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Returns the number of cells the player has traveled.
	#
	# Last Modified: October 10, 2025
	return _nb_cell_traveled
	# ----------------------------------------------------------------------------------------F-F*/
	
func set_how_many_cells_traveled(count:int)-> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Updates the number of cells the player has traveled.
	#
	# Last Modified: October 10, 2025
	_nb_cell_traveled = count
	# ----------------------------------------------------------------------------------------F-F*/
