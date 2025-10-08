/*
dynamic.gdshader
jitspoe. (2022, May 11). Godot 3D Spatial Shaders: Getting started [Video]. YouTube. https://www.youtube.com/watch?v=6-eIEFPcvrU
*/

shader_type spatial;
render_mode unshaded, cull_disabled, depth_draw_opaque;

varying vec4 instance_c;

// Using alpha as a flag due to GPU/shader limitations:
// In Godot shaders, each instance has limited properties like INSTANCE_CUSTOM
// or INSTANCE_COLOR (vec4). Storing extra data per instance can be costly,
// so using the alpha channel as a flag is a lightweight solution.
const float valid_alpha         = 240.0 / 255.0;
const float unvalid_alpha       = 241.0 / 255.0;
const float inaccessible_alpha  = 0.0 / 255.0;
const float selected_alpha      = 243.0 / 255.0;
const float path_alpha          = 244.0 / 255.0;
const float hovered_alpha       = 245.0 / 255.0;

void vertex() {
    instance_c = INSTANCE_CUSTOM;

    // Red pulse for invalid.
    if (abs(instance_c.a - unvalid_alpha) < 0.001) {
        float float_speed = 4.0;
        float color_min = 0.3;
        float color_max = 0.8;
        float color_center = (color_max + color_min) * 0.5;
        float color_range  = (color_max - color_min) * 0.5;
        instance_c.r = sin(TIME * float_speed) * color_range + color_center;
        instance_c.g = 0.0;
        instance_c.b = 0.0;
    }

    // Vertical movement for path and hovered.
    if (abs(instance_c.a - path_alpha) < 0.001) {
        float float_speed     = 4.0;
        float float_amplitude = 0.2;
        float float_wave_x    = 2.0;
        VERTEX.y += sin(TIME * float_speed + VERTEX.x * float_wave_x) * float_amplitude;
    }

    if (abs(instance_c.a - hovered_alpha) < 0.001) {
        float speed = 4.0;
        VERTEX.y += sin(TIME * speed) * 0.2;
    }
}

void fragment() {
    // Base color.
    ALBEDO = instance_c.rgb;
    EMISSION = instance_c.rgb;
    ALPHA = instance_c.a;
}