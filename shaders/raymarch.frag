#version 450

layout(location = 0) in vec2 vertUv;
layout(location = 0) out vec4 fragColor;

// constants translated pretty from my WGSL project
const float EPS = 0.01;
const int MAX_STEPS = 128;
const float SURFACE_DIST = 0.001;
const float MAX_DIST = 100.0;
const vec3 ZERO_VECTOR = vec3(0.0);
const vec3 WORLD_UP = vec3(0.0, -1.0, 0.0);
const float AXIS_THINKNESS = 0.1;
const vec3 LIGHT_POSITION = vec3(5.0);
const vec3 BACKGROUND_COLOR = vec3(0.1); // grey background

struct PlotConfig {
    vec3 min_bounds;
    vec3 max_bounds;

};
// initialize boudary box
PlotConfig boundaries = PlotConfig(vec3(-15.0), vec3(15.0));

// camera
layout(push_constant) uniform CameraData {
    vec3 position;
    vec3 target;
} camera;

// helper functions
float boundary_box(vec3 p, vec3 min_bounds, vec3 max_bounds) {
    vec3 half_length = (max_bounds - min_bounds) * 0.5;
    vec3 center = (max_bounds + min_bounds) * 0.5;
    vec3 q = abs(p - center) - half_length;
    return length(max(q, ZERO_VECTOR)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float implicitFormula(vec3 p) {
    return (p.y - sin(p.x) - cos(p.z));
}

vec3 calcNorm(vec3 p) {
    vec2 e = vec2(EPS, 0.0);
    vec3 grad_impl = vec3(
        implicitFormula(p + e.xyy) - implicitFormula(p - e.xyy),
        implicitFormula(p + e.yxy) - implicitFormula(p - e.yxy),
        implicitFormula(p + e.yyx) - implicitFormula(p - e.yyx)
    ) / (2.0 * EPS);

    return normalize(grad_impl);
}

float getHartDist(vec3 p) {
    float boxDist = boundary_box(p, boundaries.min_bounds, boundaries.max_bounds);

    float f = implicitFormula(p);
    vec3 g = calcNorm(p);
    float clippedShapeDist = max(boxDist, abs(f) / max(length(g), 0.0001));

    return clippedShapeDist;
}


void main() {
    float screenRatio = fwidth(vertUv.y) / fwidth(vertUv.x);
    vec2 xy = (vertUv * 2.0 - 1.0) * vec2(screenRatio, 1.0);
    
    vec3 cameraForward = normalize(camera.target - camera.position);
    vec3 cameraRight = normalize(cross(cameraForward, WORLD_UP));
    vec3 cameraUp = cross(cameraRight, cameraForward);

    vec3 rO = camera.position;
    vec3 rD = normalize(
        cameraRight * xy.x + cameraUp * xy.y + cameraForward
    ); // ray direction

    // cone march
    float dO = 0.0;
    float halfAngle = 0.01; // rad
    float coneSpread = tan(halfAngle);
    float accumulatedAlpha = 0.0;
    vec3 accumulatedColor = vec3(0.0);
    vec3 objectColor = vec3(0.3, 0.7, 1.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = rO + rD * dO; // ray equation
        float dS = getHartDist(p);
        float coneRadius = dO * coneSpread;

        if (coneRadius > dS) {
            // how much of the cone is inside the shape
            float insidePortion = clamp((coneRadius - dS) / coneRadius, 0.0, 1.0);

            // color based on the cone coverage
            float opacity = insidePortion * (1.0 - accumulatedAlpha);
            accumulatedColor += objectColor * opacity;
            accumulatedAlpha += opacity;
        }
        // next step
        dO += dS*0.5;
        if (dO > MAX_DIST || accumulatedAlpha >= 0.99) {
            break;
        }
    }

    // lambertian
    vec3 p = rO + rD * dO;
    // Lambertian diffusion
    vec3 n = calcNorm(p);
    vec3 l = normalize(LIGHT_POSITION - p);
    float diffusion = max(dot(n, l), 0.0);
    accumulatedColor = accumulatedColor * (diffusion + vec3(0.01));

    vec3 finalColor = accumulatedColor + (1.0 - accumulatedAlpha) * BACKGROUND_COLOR;

    fragColor = vec4(finalColor, 1.0);
}
