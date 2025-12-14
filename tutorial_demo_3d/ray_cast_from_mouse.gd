# ray_cast_from_mouse.gd

extends RayCast3D

@onready var camera_3d: Camera3D = $"../Pawn/Camera3D"
@export var debug_sphere_raycast: MeshInstance3D

func _ready() -> void:
	debug_sphere_raycast = MeshInstance3D.new()
	debug_sphere_raycast.mesh = SphereMesh.new()
	var mat_target: StandardMaterial3D = StandardMaterial3D.new()
	mat_target.albedo_color = Color.GREEN
	debug_sphere_raycast.material_override = mat_target
	debug_sphere_raycast.scale = Vector3(0.3, 0.3, 0.3)
	add_child(debug_sphere_raycast)
	debug_sphere_raycast.visible = false

func _process(delta: float) -> void:
	debug_sphere_raycast.global_transform.origin = get_ray_intersection_position()

func get_ray_intersection_position() -> Vector3:
	var intersect_ray_position: Vector3 = Vector3.ZERO
	var mouse_pos:Vector2 = get_viewport().get_mouse_position()
	var ray_origin:Vector3 = camera_3d.project_ray_origin(mouse_pos)
	var ray_direction:Vector3 = camera_3d.project_ray_normal(mouse_pos)
	var ray_length:int = 2000

	self.global_position = ray_origin
	self.target_position = ray_direction * ray_length
	self.collide_with_areas = false
	self.collision_mask = 0
	self.set_collision_mask_value(15, true)
	self.set_collision_mask_value(1, false)
	self.force_raycast_update()

	if self.is_colliding():
		var collider:Node3D = self.get_collider()

		intersect_ray_position = self.get_collision_point()
		#print("[get_ray_intersection_position] Collision detected at: ", intersect_ray_position)
		#print("[get_ray_intersection_position] Collision detected with: ", collider.name)

	return intersect_ray_position
