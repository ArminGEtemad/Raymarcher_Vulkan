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
const vec3 BACKGROUND_COLOR = vec3(0.5, 0.7, 0.9); // blue sky

struct PlotConfig {
    vec3 min_bounds;
    vec3 max_bounds;

};
// initialize boudary box
PlotConfig boundaries = PlotConfig(vec3(-30.0, -2.0, -30.0), vec3(30.0, 5.0, 30.0));

layout(push_constant) uniform uPushedConstants {
    vec3 camPos;
    float pad0;
    vec3 camTarget;
    float pad1;
    float time;
} uPushed;

// helper functions
// https://www.shadertoy.com/view/4djSRW
float hash13(vec3 p) {
    p = fract(p * .1031);
    p += dot(p, p.zyx + 33.33);
    return fract((p.x + p.y) * p.z);
}

vec3 hash33(vec3 p) {
	p = fract(p * vec3(.1031, .1030, .0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.xxy + p.yxx) * p.zyx);

}

float noisePerlin(vec3 p) {
    vec3 fl = floor(p); 
    vec3 fr = fract(p);
    fr = fr * fr * fr * (fr * (fr * 6.0 - 15.0) + 10.0);
    return mix(mix(mix(hash13(fl + vec3(0,0,0)), hash13(fl + vec3(1,0,0)), fr.x),
               mix(hash13(fl + vec3(0,1,0)), hash13(fl + vec3(1,1,0)), fr.x), fr.y),
               mix(mix(hash13(fl + vec3(0,0,1)), hash13(fl + vec3(1,0,1)), fr.x),
               mix(hash13(fl + vec3(0, 1, 1)), hash13(fl + vec3(1,1,1)), fr.x), fr.y), fr.z);
}

float noiseWorley(vec3 p) {
    vec3 fl = floor(p);
    vec3 fr = fract(p);
    float minDist = 1.0;
    
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                vec3 offset = vec3(float(x), float(y), float(z));
                vec3 h = hash33(fl + offset);
                vec3 r = offset + h - fr;
                float d = dot(r, r);
                minDist = min(minDist, d);
            }
        }
    }
    return sqrt(minDist);
}

float fbm(vec3 p) {
    float v = 0.0; 
    float a = 0.5;
    for(int i=0; i<4; i++) {
        v += a * noisePerlin(p);
        p *= 2.3; 
        a *= 0.5;
    }
    return v;
}

float boundary_box(vec3 p, vec3 min_bounds, vec3 max_bounds) {
    vec3 half_length = (max_bounds - min_bounds) * 0.5;
    vec3 center = (max_bounds + min_bounds) * 0.5;
    vec3 q = abs(p - center) - half_length;
    return length(max(q, ZERO_VECTOR)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float implicitFormula(vec3 p) {

    float wave = p.y - sin(p.x * 0.8 + uPushed.time * 0.5) * cos(p.z * 0.8 + uPushed.time * 0.5);
    
    float fluff = fbm(p * 0.3 + uPushed.time * 0.1) * 2.5;
    float baseCloud = wave - fluff;
    
    float erosion = noiseWorley(p * 0.2 - uPushed.time * 0.2) * 4.5;
    
    return baseCloud + erosion;
}

float getHartDist(vec3 p) {
    float boxDist = boundary_box(p, boundaries.min_bounds, boundaries.max_bounds);

    float f = implicitFormula(p);
    float clippedShapeDist = max(boxDist, f * 0.8);

    return clippedShapeDist;
}


void main() {
    float screenRatio = fwidth(vertUv.y) / fwidth(vertUv.x);
    vec2 xy = (vertUv * 2.0 - 1.0) * vec2(screenRatio, 1.0);
    
    vec3 cameraForward = normalize(uPushed.camTarget - uPushed.camPos);
    vec3 cameraRight = normalize(cross(cameraForward, WORLD_UP));
    vec3 cameraUp = cross(cameraRight, cameraForward);

    vec3 rO = uPushed.camPos;
    vec3 rD = normalize(
        cameraRight * xy.x + cameraUp * xy.y + cameraForward
    ); // ray direction

    // cone march
    float dO = 0.0;
    float coneSpread = 0.05;

    float accumulatedAlpha = 0.0;
    vec3 accumulatedColor = vec3(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = rO + rD * dO; // ray equation
        float dS = getHartDist(p);
        float coneRadius = dO * coneSpread;
        vec3 cloudBaseCol = vec3(0.8, 0.8, 0.9);

        if (coneRadius > dS) {
            // how much of the cone is inside the shape
            float insidePortion = clamp((coneRadius - dS) / coneRadius, 0.0, 1.0);

            // color based on the cone coverage * fluffy-ness
            float opacity = insidePortion * (1.0 - accumulatedAlpha) * 0.3;
            // top of the cloud is brighter
            float shading = smoothstep(-1.0, 1.0, p.y); 

            vec3 color = mix(cloudBaseCol * 0.5, vec3(1.0), shading);

            accumulatedColor += color * opacity;
            accumulatedAlpha += opacity;
        }
        // next step
        dO += max(dS * 0.5, 0.05);
        if (dO > MAX_DIST || accumulatedAlpha >= 0.99) {
            break;
        }
    }

    vec3 finalColor = mix(BACKGROUND_COLOR, accumulatedColor, accumulatedAlpha);

    fragColor = vec4(finalColor, 1.0);
}
