#version 450

layout(location = 0) in vec2 vertUv;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0) uniform sampler3D noiseTex;

layout(push_constant) uniform uPushedConstants {
    vec3 camPos;
    float pad0;
    vec3 camTarget;
    float pad1;
    float time;
} uPushed;

const float EPS = 0.01;
const int MAX_STEPS = 128;
const float SURFACE_DIST = 0.001;
const float MAX_DIST = 100.0;
const vec3 ZERO_VECTOR = vec3(0.0);
const vec3 WORLD_UP = vec3(0.0, -1.0, 0.0);
const vec3 LIGHT_POSITION = vec3(5.0);
const vec3 BACKGROUND_INTEG_COLOR = vec3(0.1); // grey background
const vec3 SKY_COLOR = vec3(0.5, 0.7, 0.9); // blue sky

struct PlotConfig {
    vec3 min_bounds;
    vec3 max_bounds;

};
// initialize boudary box
PlotConfig boundariesCloud = PlotConfig(vec3(-30.0, -2.0, -30.0), vec3(30.0, 5.0, 30.0));
PlotConfig boundariesInteg = PlotConfig(vec3(-10.0), vec3(10.0));

// helper functions
float boundary_box(vec3 p, vec3 min_bounds, vec3 max_bounds) {
    vec3 half_length = (max_bounds - min_bounds) * 0.5;
    vec3 center = (max_bounds + min_bounds) * 0.5;
    vec3 q = abs(p - center) - half_length;
    return length(max(q, ZERO_VECTOR)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float implicitFormula(vec3 p, bool cloud) {
    float BaseShape;

    if (!cloud) {
        return BaseShape = cos(p.x) + cos(p.y) + cos(p.z) - cos(uPushed.time);

    } else {
        BaseShape = p.y - sin(p.x * 0.8 + uPushed.time * 0.2) * cos(p.z * 0.8 + uPushed.time * 0.1) * 1.5;

        vec3 fluffVec = (p * 0.3 + uPushed.time * 0.2) / 10.0;
        vec3 erosionVec = (p * 0.2 - uPushed.time * 0.3) / 10.0;
        
        float fluff = texture(noiseTex, fluffVec).r * 5.5;
        float erosion = texture(noiseTex, erosionVec).g * 6.5;
        
        float baseCloud = BaseShape - fluff;
        return baseCloud + erosion;
    }
}

vec3 calcNorm(vec3 p, bool cloud) {
    vec2 e = vec2(EPS, 0.0);
    vec3 grad_impl = vec3(
        implicitFormula(p + e.xyy, cloud) - implicitFormula(p - e.xyy, cloud),
        implicitFormula(p + e.yxy, cloud) - implicitFormula(p - e.yxy, cloud),
        implicitFormula(p + e.yyx, cloud) - implicitFormula(p - e.yyx, cloud)
    ) / (2.0 * EPS);

    return normalize(grad_impl);
}

float getHartDist(vec3 p, bool cloud) {
    float clippedShapeDist;
    float f = implicitFormula(p, cloud);

    float boxDist;
    if (!cloud) {
        vec3 g = calcNorm(p, cloud);
        boxDist = boundary_box(p, boundariesInteg.min_bounds, boundariesInteg.max_bounds);
        clippedShapeDist = max(boxDist, abs(f) / max(length(g), 0.0001));
    } else {
        boxDist = boundary_box(p, boundariesCloud.min_bounds, boundariesCloud.max_bounds);
        clippedShapeDist = max(boxDist, f * 0.8);
    }
    
    return clippedShapeDist;
}


void main() {
    bool cloud = false;
    float fluff;
    float shading;
    float coneSpread;

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

    if (cloud) {
        fluff = 0.5;
        coneSpread = 0.05;
    } else {
        fluff = 1.0;
        coneSpread = 0.01;
    }
    
    float accumulatedAlpha = 0.0;
    vec3 accumulatedColor = vec3(0.0);

    // only needed for pure raymarching
    vec3 objectColor;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = rO + rD * dO; // ray equation
        float dS = getHartDist(p, cloud);
        float coneRadius = dO * coneSpread;

        // only needed for cloud rendering
        vec3 cloudBaseCol = vec3(0.8, 0.8, 0.9);

        if (coneRadius > dS) {
            // how much of the cone is inside the shape
            float insidePortion = clamp((coneRadius - dS) / coneRadius, 0.0, 1.0);

            // color based on the cone coverage * fluffy-ness
            float opacity = insidePortion * (1.0 - accumulatedAlpha) * fluff;
            
            // top of the cloud is brighter
            if (cloud) {
                shading = smoothstep(-1.0, 1.0, p.y);
                objectColor = mix(cloudBaseCol * 0.5, vec3(1.0), shading);
            } else {
                objectColor = vec3(0.3, 0.7, 1.0);
            }

            accumulatedColor += objectColor * opacity;
            accumulatedAlpha += opacity;
        }
        // next step
        if (cloud) {
            dO += max(dS * 0.5, 0.05);
        } else {
            dO += dS * 0.5;
        }
       
        if (dO > MAX_DIST || accumulatedAlpha >= 0.99) {
            break;
        }
    }

    if (cloud) {
        vec3 finalColor = mix(SKY_COLOR, accumulatedColor, accumulatedAlpha);
        fragColor = vec4(finalColor, 1.0);
    } else {
        // lambertian
        vec3 p = rO + rD * dO;
        // Lambertian diffusion
        vec3 n = calcNorm(p, cloud);
        vec3 l = normalize(LIGHT_POSITION - p);
        float diffusion = max(dot(n, l), 0.0);
        accumulatedColor = accumulatedColor * (diffusion + vec3(0.01));

        vec3 finalColor = accumulatedColor + (1.0 - accumulatedAlpha) * BACKGROUND_INTEG_COLOR;

        fragColor = vec4(finalColor, 1.0);
    }
}
