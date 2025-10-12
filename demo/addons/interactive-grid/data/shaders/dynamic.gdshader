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
