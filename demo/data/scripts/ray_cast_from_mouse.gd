# =================================================================================================
# File: ray_cast_from_mouse.gd
#
# Summary: 
#
# Node: RayCastFromMouse (RayCast3D).
#
# Last modified: October 04, 2025
#
# This file is part of the InteractiveGrid GDExtension Source Code.
# Repository: https://github.com/antoinecharruel/interactive_grid_gdextension
#
# Version InteractiveGrid: 1.0.0
# Version: Godot Engine v4.5.stable.steam - https://godotengine.org
#
# Author: Antoine Charruel
# =================================================================================================

extends RayCast3D

@onready var ray_cast_from_mouse: RayCast3D = $"."
@export var debug_sphere_ray_cast_: MeshInstance3D
@onready var camera_3d: Camera3D = $"../Camera3D"

func _ready() -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called when the node enters the scene tree for the first time.
	#
	# Last Modified: October 04, 2025
	
	# Create a sphere for raycast debugging.
	debug_sphere_ray_cast_ = MeshInstance3D.new()
	debug_sphere_ray_cast_.mesh = SphereMesh.new()
	var mat_target = StandardMaterial3D.new()
	mat_target.albedo_color = Color.GREEN
	debug_sphere_ray_cast_.material_override = mat_target
	debug_sphere_ray_cast_.scale = Vector3(0.3, 0.3, 0.3)
	
	add_child(debug_sphere_ray_cast_)
	debug_sphere_ray_cast_.visible = false
	# ----------------------------------------------------------------------------------------F-F*/
	
func _process(delta: float) -> void:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Called every frame. 'delta' is the elapsed time since the previous frame.
	#
	# Last Modified: October 04, 2025
	
	# Position the debug sphere at the ray intersection point from the mouse.
	if(ray_cast_from_mouse):
		debug_sphere_ray_cast_.global_transform.origin = get_ray_intersection_position()
	# ----------------------------------------------------------------------------------------F-F*/
	
func get_ray_intersection_position() -> Vector3:
	# /*F+F++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	# Summary: Computes the world position where the mouse ray intersects with collidable objects.
	#          Uses a RayCast3D projected from the camera through the mouse cursor.
	#
	# Last Modified: October 04, 2025
	
	var intersect_ray_position: Vector3 = Vector3.ZERO

	var mouse_pos:Vector2 = get_viewport().get_mouse_position()
	var ray_origin:Vector3 = camera_3d.project_ray_origin(mouse_pos)
	var ray_direction:Vector3 = camera_3d.project_ray_normal(mouse_pos)
	var ray_length:int = 2000
	
	# Position and orient the RayCast.
	ray_cast_from_mouse.global_position = ray_origin
	ray_cast_from_mouse.target_position = ray_direction * ray_length
	ray_cast_from_mouse.collide_with_areas = false
	
	ray_cast_from_mouse.collision_mask = 0 # Reset.
	ray_cast_from_mouse.set_collision_mask_value(15, true) # Ignore this layer.
	
	# Désactiver toutes les autres couches éventuellement conflictuelles
	ray_cast_from_mouse.set_collision_mask_value(1, false)
	
	var debug_sphere_raycast: MeshInstance3D

	ray_cast_from_mouse.force_raycast_update()
	
	# Force an immediate RayCast update.
	if ray_cast_from_mouse.is_colliding():
		var collider:Node3D = ray_cast_from_mouse.get_collider()
		
		intersect_ray_position = ray_cast_from_mouse.get_collision_point()
		#print("[GetRayIntersectionPosition] Collision detected at: ", intersect_ray_position)
		#print("[GetRayIntersectionPosition] Collision detected with: ", collider.name)
		
	return intersect_ray_position
	# ----------------------------------------------------------------------------------------F-F*/
