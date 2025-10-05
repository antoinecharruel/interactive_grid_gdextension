// grid_default_shader_material.gdshader
shader_type spatial;
render_mode unshaded, cull_disabled, depth_draw_opaque;
varying vec4 instance_c;

void vertex() {
	instance_c = INSTANCE_CUSTOM;
}

void fragment() {
	ALBEDO = instance_c.rgb;
	ALPHA = instance_c.a;
}