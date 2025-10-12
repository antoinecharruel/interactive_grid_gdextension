[![Interactive Grid GDExtension Godot Asset Library page](https://img.shields.io/static/v1?logo=godotengine&label=Interactive%20Grid%20GDExtension&color=478CBF&message=1.0.0)](https://godotengine.org/asset-library/asset/4372)
[![Patreon](https://img.shields.io/badge/Patreon-Vivensoft-F96854?logo=patreon&logoColor=white)](https://www.patreon.com/c/vivensoft/)  
[![Ko-fi](https://img.shields.io/badge/Ko--fi-AntoineCharruel-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/antoinecharruel)
[![AntoineCharruel on Itch.io](https://img.shields.io/badge/Itch.io-AntoineCharruel-FF5E5B?logo=itch.io&logoColor=white)](https://antoine-charruel.itch.io/)
[![Instagram](https://img.shields.io/badge/Instagram-VSFT%20GameDev-E4405F?logo=instagram&logoColor=white)](https://www.instagram.com/vsftgamedev/)

# Interactive Grid GDExtension

![Grid Showcase](/docs/thumbnail/thumbnail.png)

InteractiveGrid is a Godot 4.5 GDExtension that allows player interaction with a 3D grid, including cell selection, pathfinding, and hover highlights.

## Features

- Highlight cells when hovering the mouse over them.
- Select individual cells.
- Detect obstacles (collision mask configurable from the editor).
- Align cells with the floor (collision mask configurable from the editor).
- Hide distant cells to focus on the relevant area.
- Calculate paths from a global position to selected cells using [AStar2D](https://docs.godotengine.org/en/stable/classes/class_astar2d.html).
- Choose movement type: 4 directions, 8 directions, directly from the editor.
- Customize the grid from the editor: grid size, cell size, mesh, colors, and shaders.
- High performance using [MultiMeshInstance3D](https://docs.godotengine.org/en/4.4/classes/class_multimeshinstance3d.html) for efficient rendering of multiple cells.

⚠️ Work in progress : Everything is subject to change.

## Compatibility:

- Fully compatible with Godot 4.5.
- Cross-platform support (Linux, Windows, macOS, Android, iOS, Web/HTML5).

## Try the demo on Itch.io! 🎮

![](/docs/preview/demo.gif)
<div style="text-align:center;">
  <a href="https://ahappypapabear.itch.io/interactive-grid-gdextension" target="_blank" style="text-decoration:none;background-color:#28a745;color:white;padding:5px 12px;border-radius:5px;">
    🎮 Play on Itch.io
  </a>
</div>



## Minimal Demo Example in GDScript

📄 [Download the full Interactive Grid GDExtension Minimal demo PDF](https://raw.githubusercontent.com/antoinecharruel/interactive_grid_gdextension/main/docs/pandoc/demo.pdf)


```python
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
# Version InteractiveGrid: 1.0.0
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
				self.set_visible(true)
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
				var color_cell = Color(0.3, 0.4, 0.9)
				self.set_cell_color(65, color_cell)
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
					
					get_cell_golbal_position(selected_cells[0]))

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
```

## Use Cell Flags via Alpha Channel to Modify Cell Shader at Runtime

```python
// dynamic.gdshader

shader_type spatial;
render_mode unshaded, cull_disabled, depth_draw_opaque;

varying vec4 instance_c;

/*
// Using alpha as a flag due to GPU/shader limitations:

Interactive_grid.h
static constexpr int CFL_WALKABLE = 1 << 0;
static constexpr int CFL_INACCESSIBLE = 1 << 1;
static constexpr int CFL_IN_VOID = 1 << 2;
static constexpr int CFL_HOVERED = 1 << 3;
static constexpr int CFL_SELECTED = 1 << 4;
static constexpr int CFL_PATH = 1 << 5;
static constexpr int CFL_VISIBLE = 1 << 6;
*/

const int CFL_WALKABLE = 1 << 0;
const int CFL_INACCESSIBLE = 1 << 1;
const int CFL_IN_VOID = 1 << 2;
const int CFL_HOVERED = 1 << 3;
const int CFL_SELECTED  = 1 << 4;
const int CFL_PATH = 1 << 5;
const int CFL_VISIBLE = 1 << 6;

void vertex() {
    instance_c = INSTANCE_CUSTOM;

	int cell_flag = int(instance_c.a);

	if ((cell_flag & CFL_WALKABLE) != 0) { // walkable.
		instance_c.a = 0.5;
	}

    // Red pulse for invalid.
    if ((cell_flag & CFL_WALKABLE) == 0) { // Unwalkable.
		float float_speed = 4.0;
		float color_min = 0.3;
		float color_max = 0.8;
		float color_center = (color_max + color_min) * 0.5;
		float color_range  = (color_max - color_min) * 0.5;
		instance_c.r = sin(TIME * float_speed) * color_range + color_center;
		instance_c.g = 0.0;
		instance_c.b = 0.0;
		instance_c.a = 0.5;
	}

    if ((cell_flag & CFL_PATH) != 0) {
        float float_speed     = 4.0;
        float float_amplitude = 0.2;
        float float_wave_x    = 2.0;
        VERTEX.y += sin(TIME * float_speed + VERTEX.x * float_wave_x) * float_amplitude;
		instance_c.a = 0.5;
    }

    if ((cell_flag & CFL_HOVERED) != 0) {
        float speed = 4.0;
        VERTEX.y += sin(TIME * speed) * 0.2;
		instance_c.a = 0.5;
    }

	if ((cell_flag & CFL_VISIBLE) == 0) { // walkable.
		instance_c.a = 0.0;
	}
}

void fragment() {
    // Base color.
    ALBEDO = instance_c.rgb;
    EMISSION = instance_c.rgb;
    ALPHA = instance_c.a;
}
```


## TODO Minor

- [ ] Define hexagonal layout method.
- [ ] godot::Array get_neighbors(cell_index)
- [ ] void clear_path(path)
- [ ] Add vaultable cells
  + [ ] A Define `CFL_VAULT` flag for cells that can be vaulted/climbed.
  + [ ] Assign vaultable cells to a specific collision layer checked by `scan_environnement_obstacles` method.
  + [ ] Add a visual indicator (color) for vaultable cells using `ADD_PROPERTY`.

## Acknowledgments & References (Videos & Websites)

A big thank you to the creators of the YouTube tutorials and websites that guided me during this project:

- BornCG. (2024, August 4). Godot 4 3D Platformer Lesson #13: Align Player with Ground! [Video]. YouTube. https://www.youtube.com/watch?v=Y5OiChOukfg
- jitspoe. (2022, May 11). Godot 3D Spatial Shaders: Getting started [Video]. YouTube. https://www.youtube.com/watch?v=6-eIEFPcvrU
- jmbiv. (2021, October 5). How to make a 3D hexagon grid in Godot
        (Tutorial) [Video]. YouTube. 
		https://www.youtube.com/watch?v=3Lt2TfP8WEw
- Patel, A. J. (2013). Hexagonal grids. 
  	   https://www.redblobgames.com/grids/hexagons/#neighbors


## Great Videos on Grid-Based Game Design

- Chaff Games. (2025, July 23). I remade Final Fantasy Tactics in Godot [Video]. YouTube. https://www.youtube.com/watch?v=iXnKYtTZrAo