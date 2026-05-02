#version 450

layout(location = 0) in vec2 vertUv;
layout(location = 0) out vec4 fragColor;

// constants translated pretty from my WGSL project
const float EPS = 0.01;
const int MAX_STEPS = 128;
const float SURFACE_DIST = 0.001;
const float MAX_DIST = 100.0;

// helper functions
float get_dist(vec3 p) {
    float sphere_radius = 1.0;
    vec3 sphere_center = vec3(0.0, 0.0, 0.0);

    // distance function for sphere
    return length(p - sphere_center) - sphere_radius;
}

vec3 calc_norm(vec3 p) {
    vec2 e = vec2(EPS, 0.0);
    vec3 grad_impl = vec3(
        get_dist(p + e.xyy) - get_dist(p - e.xyy),
        get_dist(p + e.yxy) - get_dist(p - e.yxy),
        get_dist(p + e.yyx) - get_dist(p - e.yyx)
    ) / (2.0 * EPS);

    return normalize(grad_impl);
}

void main() {
    float screen_ratio = fwidth(vertUv.y) / fwidth(vertUv.x);
    vec2 xy = (vertUv * 2.0 - 1.0) * vec2(screen_ratio, 1.0);

    vec3 r_o = vec3(0.0, 0.0, -3.0); // ray origin
    vec3 r_d = normalize(vec3(xy, 1.0)); // ray direction

    // Raymarching loop
    float d_o = 0.0; 
    bool hit = false;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = r_o + r_d * d_o;
        float d_s = get_dist(p);
        d_o += d_s;

        if (d_o > MAX_DIST || d_s < SURFACE_DIST) {
            if (d_s < SURFACE_DIST) {
                hit = true;
            }
            break;
        }
    }

    // Shading
    vec3 color = vec3(0.1, 0.1, 0.1); // Background color

    if (hit) {
        vec3 p = r_o + r_d * d_o;
        vec3 n = calc_norm(p);

        // Lambertian diffusion
        vec3 light_position = vec3(4.0, 4.0, -5.0);
        vec3 l = normalize(light_position - p);
        float diffuse = max(dot(n, l), 0.0);
        
        color = vec3(0.3, 0.7, 1.0) * diffuse;
        color += vec3(0.02); // ambient
    }

    fragColor = vec4(color, 1.0);
}
