#**************************************************************************#
#*  pawn_player.gd                                                        *#
#**************************************************************************#
#*                         This file is part of:                          *#
#*                     INTERACTIVE GRID GDExtension                       *#
#*         https://github.com/antoinecharruel/interactive_grid            *#
#**************************************************************************#
#* Copyright (c) 2025 Antoine Charruel.                                   *#
#*                                                                        *#
#* Permission is hereby granted, free of charge, to any person obtaining  *#
#* a copy of this software and associated documentation files (the        *#
#* "Software"), to deal in the Software without restriction, including    *#
#* without limitation the rights to use, copy, modify, merge, publish,    *#
#* distribute, sublicense, and/or sell copies of the Software, and to     *#
#* permit persons to whom the Software is furnished to do so, subject to  *#
#* the following conditions:                                              *#
#*                                                                        *#
#* The above copyright notice and this permission notice shall be         *#
#* included in all copies or substantial portions of the Software.        *#
#*                                                                        *#
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *#
#* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *#
#* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. *#
#* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   *#
#* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   *#
#* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      *#
#* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 *#
#**************************************************************************#

extends CharacterBody3D

@onready var interactive_grid_3d: InteractiveGrid3D = $"../InteractiveGrid3D"
@onready var animation_player: AnimationPlayer = $model/AnimationPlayer
@onready var model: Node3D = $model
@onready var start_control: Control = $"../Start_control"

const SPEED:float = 5.0

enum PawnMovementStates{
	IDLE,
	WALKING,
	RUN
}

var _movement_state:int = PawnMovementStates.IDLE
var _cells_traveled:int = 0


func _ready() -> void:
	pass


func _physics_process(delta: float) -> void:	
	# Add the gravity.
	if not is_on_floor():
		velocity += get_gravity() * delta
		move_and_slide()
	
	if self.velocity == Vector3.ZERO:
		if _movement_state != PawnMovementStates.IDLE:
			_movement_state = PawnMovementStates.IDLE
		else:
			animation_player.play("idle", 0.2)


func move_to(global_position: Vector3)-> void:
	var pawn_global_position:Vector3 = self.global_position
	var target_global_position: Vector3 = Vector3(global_position.x, pawn_global_position.y, global_position.z)
	var direction:Vector3 = (target_global_position - pawn_global_position).normalized()
	var distance_to_target: float = pawn_global_position.distance_to(target_global_position)

	self.velocity = direction * SPEED

	if _movement_state != PawnMovementStates.RUN:
		_movement_state = PawnMovementStates.RUN
	else:
		animation_player.play("run", 0.2)

	var dir: Vector3 = (target_global_position - model.global_position)
	dir.y = 0
	dir = dir.normalized()

	var target_rot: float = atan2(-dir.x, -dir.z)
	model.rotation.y = lerp_angle(model.rotation.y, target_rot, 0.2)

	move_and_slide()


func idle():
	if _movement_state != PawnMovementStates.IDLE:
		_movement_state = PawnMovementStates.IDLE
		animation_player.play("idle", 0.2)


func _on_button_button_down() -> void:
	interactive_grid_3d.set_pawn(self)
	interactive_grid_3d.show_grid()
	start_control.visible = false


func _input(event):
	if event.is_action_pressed("show_grid") and start_control.visible:
		interactive_grid_3d.set_pawn(self)
		interactive_grid_3d.show_grid()
		start_control.visible = false
