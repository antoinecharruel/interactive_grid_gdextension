# pawn.gd

extends CharacterBody3D

@onready var model: MeshInstance3D = $Model
@onready var interactive_grid_3d: InteractiveGrid3D = $"../InteractiveGrid3D"

const SPEED:float = 5.0

func _physics_process(delta: float) -> void:
   # Add the gravity.
	if not is_on_floor():
		velocity += get_gravity() * delta
		move_and_slide()

func move_to(global_position: Vector3)-> void:
	var pawn_global_position:Vector3 = self.global_position
	var target_global_position: Vector3 = Vector3(global_position.x, pawn_global_position.y, global_position.z)
	var direction:Vector3 = (target_global_position - pawn_global_position).normalized()
	var distance_to_target: float = pawn_global_position.distance_to(target_global_position)

	self.velocity = direction * SPEED

	var dir: Vector3 = (target_global_position - model.global_position)
	dir.y = 0
	dir = dir.normalized()

	var target_rot: float = atan2(-dir.x, -dir.z)
	model.rotation.y = lerp_angle(model.rotation.y, target_rot, 0.2)

	move_and_slide()


func _input(event):
	if event.is_action_pressed("show_grid"):
		interactive_grid_3d.set_pawn(self)
		interactive_grid_3d.show_grid()
