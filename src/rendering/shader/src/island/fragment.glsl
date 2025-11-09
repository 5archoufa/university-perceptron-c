#version 460 core

flat in vec4 vertexColor;
in vec2 texCoord;
flat in vec3 fragNormal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

uniform int toonLevels = 4;
uniform float ambientStrength = 0.2;
uniform vec3 ambientColor = vec3(1.0, 1.0, 1.0);
uniform float detailScale = 10.0;
uniform float detailStrength = 0.1;
uniform float noiseScale = 50.0;
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

float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float pixelNoise(vec2 p) { 
    return hash(floor(p)); 
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 3; i++) {
        value += amplitude * pixelNoise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float toonShade(float value) {
    float toonLevel = floor(value * float(toonLevels));
    return toonLevel / max(float(toonLevels - 1), 1.0);
}

vec3 calcDirectionalLight(vec3 direction, vec3 color, float intensity, vec3 normal) {
    // The direction from transform is the FORWARD vector
    // For lighting, we need the direction TO the light (opposite of forward)
    vec3 lightDir = normalize(-direction);
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    NdotL = pow(NdotL, 0.7);
    
    float toonDiffuse = toonShade(NdotL);
    
    // Apply intensity to the light contribution
    return toonDiffuse * color * intensity;
}

float calcShadow(vec3 normal, vec3 lightDir) {
    float NdotL = dot(normal, lightDir);
    float shadow = smoothstep(-shadowSoftness, shadowSoftness, NdotL);
    return shadow;
}

vec3 calcPointLight(vec3 position, vec3 color, float intensity, float range, vec3 fragPos, vec3 normal) {
    vec3 lightDir = position - fragPos;
    float distance = length(lightDir);
    float attenuation = clamp(1.0 - (distance / range), 0.0, 1.0);
    attenuation *= attenuation;
    if (attenuation <= 0.0) return vec3(0.0);

    lightDir = normalize(lightDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float toonDiffuse = toonShade(NdotL * attenuation);
    
    // Apply intensity to the light contribution
    return toonDiffuse * color * intensity * attenuation;
}

void main() {
    vec4 baseColor = useTexture ? texture(textureSampler, texCoord) * vertexColor : vertexColor;

    vec3 viewPos = camera_position.xyz;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 norm = normalize(fragNormal);

    vec3 lighting = ambientStrength * ambientColor;
    
    float shadowFactor = 1.0;
    if (light_directional_count > 0) {
        vec3 mainLightDir = normalize(-light_directional_directions[0].xyz);
        shadowFactor = calcShadow(norm, mainLightDir);
    }

    // Directional lights - now using intensity from .w component
    for (int i = 0; i < light_directional_count && i < 4; i++) {
        vec3 dirLight = calcDirectionalLight(
            light_directional_directions[i].xyz,
            light_directional_colors[i].xyz,
            light_directional_colors[i].w,  // Intensity from w component
            norm
        );
        
        if (i == 0) {
            dirLight *= shadowFactor;
        }
        
        lighting += dirLight;
    }

    // Point lights - already using intensity correctly
    for (int i = 0; i < light_point_count && i < 8; i++) {
        lighting += calcPointLight(
            light_point_positions[i].xyz,
            light_point_colors[i].xyz,
            light_point_colors[i].w,  // Intensity from w component
            light_point_positions[i].w,
            fragPos,
            norm
        );
    }

    // Detail & noise
    float detail = fbm(fragPos.xz * detailScale);
    float noise = pixelNoise(fragPos.xz * noiseScale);
    float combinedDetail = (mix(detail, noise, 0.5) - 0.5) * detailStrength;

    // Ensure a minimum brightness to prevent pure black in unlit areas
    vec3 minBrightness = vec3(0.3); // 30% minimum to preserve base color
    vec3 finalLighting = max(lighting, minBrightness);
    
    if (light_directional_count > 0) {
        // Use a darker version of the shadow color that still preserves some hue
        vec3 darkerShadowColor = shadowColor * 0.5 + vec3(0.15);
        finalLighting = max(mix(ambientStrength * darkerShadowColor, lighting, shadowFactor), minBrightness);
    }

    vec3 result = baseColor.rgb * (finalLighting + vec3(combinedDetail));
    
    FragColor = vec4(result, baseColor.a);
}