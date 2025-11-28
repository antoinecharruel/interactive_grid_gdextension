shader_type spatial;
render_mode unshaded, cull_disabled, depth_draw_opaque;

varying vec4 instance_c;

const int CFL_WALKABLE = 1 << 0;
const int CFL_REACHABLE = 1 << 1;
const int CFL_IN_VOID = 1 << 2;
const int CFL_HOVERED = 1 << 3;
const int CFL_SELECTED  = 1 << 4;
const int CFL_PATH = 1 << 5;
const int CFL_VISIBLE = 1 << 6;

//uniform float glow_strength : hint_range(0.0, 300.0) = 50.0;
uniform float glow_strength : hint_range(0.0, 10.0) = 2.0;
uniform float halo_intensity : hint_range(0.0, 5.0) = 1.5;
uniform float halo_spread : hint_range(0.1, 2.0) = 1.0;
uniform float pulse_speed : hint_range(0.0, 10.0) = 1.0;

void vertex() {
    instance_c = INSTANCE_CUSTOM;
    int cell_flag = int(instance_c.a);

    if ((cell_flag & CFL_WALKABLE) != 0) {
        instance_c.a = 0.33;
    }

    if ((cell_flag & CFL_WALKABLE) == 0) {
        float float_speed = 4.0;
        float color_min = 0.3;
        float color_max = 0.8;
        float color_center = (color_max + color_min) * 0.5;
        float color_range  = (color_max - color_min) * 0.5;
        instance_c.r = sin(TIME * float_speed) * color_range + color_center;
        instance_c.g = 0.0;
        instance_c.b = 0.0;
        instance_c.a = 0.15;
    }

    if ((cell_flag & CFL_REACHABLE) == 0) instance_c.a = 0.0;
    if ((cell_flag & CFL_VISIBLE) == 0) instance_c.a = 0.0;

    if ((cell_flag & CFL_PATH) != 0) {
        float float_speed = 4.0;
        float float_amplitude = 0.2;
        float float_wave_x = 2.0;
        VERTEX.y += sin(TIME * float_speed + VERTEX.x * float_wave_x) * float_amplitude;
        instance_c.a = 0.5;
    }

    if ((cell_flag & CFL_HOVERED) != 0) {
        float speed = 4.0;
        VERTEX.y += sin(TIME * speed) * 0.2;
        instance_c.a = 0.5;
    }

    if ((cell_flag & CFL_VISIBLE) == 0) instance_c.a = 0.0;
}

void fragment() {
    vec2 centered_uv = UV - vec2(0.5);
    float dist = length(centered_uv) * halo_spread;

    float halo_alpha = max(0.0, 1.0 - dist);
    halo_alpha *= (1.0 + 0.2 * sin(TIME * pulse_speed));
    halo_alpha = clamp(halo_alpha * halo_intensity, 0.0, 1.0);

    ALBEDO = instance_c.rgb;
    EMISSION = instance_c.rgb * glow_strength * halo_alpha;

    ALPHA = instance_c.a * halo_alpha;
}