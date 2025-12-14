// interractive_grid.gdshader

shader_type spatial;
render_mode unshaded, cull_disabled, depth_draw_opaque;

varying vec4 instance_c;
varying vec4 instance_c_default;
varying float alpha;

// Default cell flags:
const int CFL_WALKABLE = 1 << 0;
const int CFL_REACHABLE = 1 << 1;
const int CFL_IN_VOID = 1 << 2;
const int CFL_HOVERED = 1 << 3;
const int CFL_SELECTED  = 1 << 4;
const int CFL_PATH = 1 << 5;
const int CFL_VISIBLE = 1 << 6;

// Custom cell data:
const int CFL_PLAYER = 1 << 7;
const int CFL_NEIGHBORS = 1 << 9;
const int CFL_TRAP = 1 << 10;
const int CFL_PULSE = 1 << 8;

vec3 pulse_color(vec3 base_color, float speed, float min_val, float max_val) {
    float center = (min_val + max_val) * 0.5;
    float range  = (max_val - min_val) * 0.5;
    return base_color * (sin(TIME * speed) * range + center);
}

void vertex() {
    instance_c = INSTANCE_CUSTOM;
    int cell_flag = int(instance_c.a);
	alpha = 0.5;
	
    if ((cell_flag & CFL_WALKABLE) == 0) {
        alpha = 0.20;
    }

	 if ((cell_flag & CFL_NEIGHBORS) != 0
	    && (cell_flag & CFL_PATH) == 0
	    && (cell_flag & CFL_TRAP) == 0)
		{
	        if ((cell_flag & CFL_WALKABLE) != 0) {
				alpha = 0.40;
				instance_c.r = 0.2;
		        instance_c.g = 0.5;
		        instance_c.b = 1.0;
			}
	    }
	
    if ((cell_flag & CFL_PATH) != 0) {
        VERTEX.y += sin(TIME * 4.0 + VERTEX.x * 2.0) * 0.2;
    }
	
    if ((cell_flag & CFL_HOVERED) != 0) {
        VERTEX.y += sin(TIME * 4.0) * 0.2;
    }
	
	if ((cell_flag & CFL_TRAP) != 0) {
		VERTEX.y += sin(TIME * 4.0 + VERTEX.x * 2.0) * 0.2;
    }
	
	if ((cell_flag & CFL_PULSE) != 0) {
		instance_c.rgb = pulse_color(vec3(instance_c.rgb), 4.0, 0.3, 0.8);
	}
	
    if ((cell_flag & CFL_VISIBLE) == 0) {
        alpha = 0.0; // invisible
    }
	
    if ((cell_flag & CFL_REACHABLE) == 0) {
        alpha = 0.0; // invisible
    }
	
	if ((cell_flag & CFL_IN_VOID) != 0) {
        alpha = 0.0; // invisible
    }
}

void fragment() {
    if (alpha == 0.0) {
        discard;
    }
    ALBEDO = instance_c.rgb;
    EMISSION = instance_c.rgb;
    ALPHA = alpha;
}