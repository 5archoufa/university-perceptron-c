#version 460 core

in vec4 vertexColor;
in vec2 texCoord;
in vec3 fragNormal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

uniform float ambientStrength = 0.2;
uniform vec3 ambientColor = vec3(1.0, 1.0, 1.0);
uniform float shadowSoftness = 0.05;
uniform vec3 shadowColor = vec3(0.2, 0.2, 0.25);

layout(std140, binding = 0) uniform ShaderGlobalData {
    mat4 view;
    mat4 projection;
    vec4 camera_position;
    float time;
    float timeOfDay;
    float _pad_world_2;
    float _pad_world_3;
    int light_directional_count;
    float _pad_light_dir_1;
    float _pad_light_dir_2;
    float _pad_light_dir_3;
    vec4 light_directional_directions[4];
    vec4 light_directional_colors[4];
    int light_point_count;
    float _pad_light_point_1;
    float _pad_light_point_2;
    float _pad_light_point_3;
    vec4 light_point_colors[8];
    vec4 light_point_positions[8];
};

// ----- Improved Toon shading ramp (5 levels) -----
// Now returns multipliers that preserve base color hue
vec3 toonRamp(float NdotL) {
    if (NdotL < 0.2) return vec3(0.3);       // Dark but not black (30%)
    else if (NdotL < 0.4) return vec3(0.45); // Mid-dark (45%)
    else if (NdotL < 0.6) return vec3(0.65); // Mid (65%)
    else if (NdotL < 0.8) return vec3(0.85); // Mid-bright (85%)
    else return vec3(1.0);                   // Full brightness
}

// ----- Directional Light -----
vec3 calcDirectionalLight(vec3 direction, vec3 color, float intensity, vec3 normal) {
    vec3 lightDir = normalize(-direction);
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 toon = toonRamp(NdotL);
    return toon * color * intensity;
}

// ----- Shadow -----
float calcShadow(vec3 normal, vec3 lightDir) {
    float NdotL = dot(normal, lightDir);
    return smoothstep(-shadowSoftness, shadowSoftness, NdotL);
}

// ----- Point Light -----
vec3 calcPointLight(vec3 position, vec3 color, float intensity, float range, vec3 fragPos, vec3 normal) {
    vec3 lightDir = position - fragPos;
    float distance = length(lightDir);
    float attenuation = clamp(1.0 - (distance / range), 0.0, 1.0);
    attenuation *= attenuation;
    if (attenuation <= 0.0) return vec3(0.0);

    lightDir = normalize(lightDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 toon = toonRamp(NdotL * attenuation);
    return toon * color * intensity * attenuation;
}

void main() {
    vec4 baseColor = useTexture ? texture(textureSampler, texCoord) * vertexColor : vertexColor;
    vec3 norm = normalize(fragNormal);

    // ----- Ambient -----
    vec3 lighting = ambientStrength * ambientColor;

    // ----- Shadows -----
    float shadowFactor = 1.0;
    if (light_directional_count > 0) {
        vec3 mainLightDir = normalize(-light_directional_directions[0].xyz);
        shadowFactor = calcShadow(norm, mainLightDir);
    }

    // ----- Directional Lights -----
    for (int i = 0; i < light_directional_count && i < 4; i++) {
        vec3 dirLight = calcDirectionalLight(
            light_directional_directions[i].xyz,
            light_directional_colors[i].xyz,
            light_directional_colors[i].w,
            norm
        );
        if (i == 0) dirLight *= shadowFactor;
        lighting += dirLight;
    }

    // ----- Point Lights -----
    for (int i = 0; i < light_point_count && i < 8; i++) {
        lighting += calcPointLight(
            light_point_positions[i].xyz,
            light_point_colors[i].xyz,
            light_point_colors[i].w,
            light_point_positions[i].w,
            fragPos,
            norm
        );
    }

    // ----- Final Color -----
    // Ensure a minimum brightness to prevent pure black in unlit areas
    vec3 minBrightness = vec3(0.3); // 30% minimum to preserve base color
    vec3 finalLighting = max(lighting, minBrightness);
    
    if (light_directional_count > 0) {
        // Use a darker version of the shadow color that still preserves some hue
        vec3 darkerShadowColor = shadowColor * 0.5 + vec3(0.15);
        finalLighting = max(mix(ambientStrength * darkerShadowColor, lighting, shadowFactor), minBrightness);
    }

    vec3 result = baseColor.rgb * finalLighting;
    FragColor = vec4(result, baseColor.a);
}