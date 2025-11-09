#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aColor;

out vec4 vertexColor;
out vec2 texCoord;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 worldPos;

// === Global UBO ===
layout(std140, binding = 0) uniform ShaderGlobalData {
    mat4 view;
    mat4 projection;
    vec4 camera_position;

    float time;
    float timeOfDay;
    float _pad_world_1;
    float _pad_world_2;

    int light_directional_count;
    float _pad_dir_count_0;
    float _pad_dir_count_1;
    float _pad_dir_count_2;

    vec4 light_directional_directions[4];
    vec4 light_directional_colors[4];

    int light_point_count;
    float _pad_point_0;
    float _pad_point_1;
    float _pad_point_2;

    vec4 light_point_colors[8];
    vec4 light_point_positions[8];
};

uniform mat4 model;

// ── Wave Parameters ────────────────────────────────────────────────
uniform float waveAmplitude = 0.2;
uniform float waveFreq = 1.5;
uniform float pixelQuant = 4.0;  // quantization strength for spatial pixelation
uniform float fdEps = 0.02;

// ── Helper Functions ───────────────────────────────────────────────
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

// quantize position for pixelated appearance (spatial only)
vec2 quantize(vec2 p) {
    return floor(p * pixelQuant) / pixelQuant;
}

// smoothed wave height
float waveHeight(vec2 posXZ, float t) {
    // quantize only the spatial part to preserve blocky look
    vec2 q = quantize(posXZ);

    // smooth oscillation with multiple wave layers
    float w1 = sin(q.x * (waveFreq * 1.1) + t * 0.7);
    float w2 = cos(q.y * (waveFreq * 0.8) - t * 0.6);
    float w3 = sin((q.x + q.y) * (waveFreq * 0.6) + t * 0.9);

    // mix smooth sine waves with a touch of spatial randomness
    float n = hash(floor(q * 5.0));
    float base = (w1 + w2 + w3 * 0.5) / 2.5;
    base += n * 0.2; // add subtle irregularity (doesn't flicker)

    return base * waveAmplitude;
}

// ── Main ──────────────────────────────────────────────────────────
void main()
{
    vec3 objPos = aPos;

    // sample displaced height
    float h0 = waveHeight(objPos.xz, time);
    float hX = waveHeight(objPos.xz + vec2(fdEps, 0.0), time);
    float hZ = waveHeight(objPos.xz + vec2(0.0, fdEps), time);

    // displace vertically
    vec3 displaced = vec3(objPos.x, objPos.y + h0, objPos.z);

    // compute normal from finite differences
    vec3 T_x = vec3(fdEps, hX - h0, 0.0);
    vec3 T_z = vec3(0.0, hZ - h0, fdEps);
    // FIXED: Reversed cross product order to make normals point upward
    vec3 normalObj = normalize(cross(T_z, T_x));

    // transform to world space
    vec4 worldPosition = model * vec4(displaced, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 worldNormal = normalize(normalMatrix * normalObj);

    // outputs
    worldPos = worldPosition.xyz;
    fragPos = worldPosition.xyz;
    fragNormal = worldNormal;
    texCoord = aTexCoord;
    vertexColor = aColor;

    gl_Position = projection * view * worldPosition;
}