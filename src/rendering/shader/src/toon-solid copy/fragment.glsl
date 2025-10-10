#version 460 core

flat in vec4 vertexColor;
in vec2 texCoord;
flat in vec3 fragNormal;
in vec3 fragPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

// Toon shader parameters
uniform int toonLevels = 3;
uniform float ambientStrength = 0.5;
uniform vec3 ambientColor = vec3(1.0, 1.0, 1.0);
uniform float rimThreshold = 0.7;
uniform float rimStrength = 0.25;
uniform float detailScale = 10.0;
uniform float detailStrength = 0.1;
uniform float noiseScale = 50.0;

// Shadow parameters
uniform float shadowSoftness = 0.3;  // How soft shadow edges are
uniform vec3 shadowColor = vec3(0.3, 0.3, 0.4);  // Tint for shadows (slightly blue)

// === Global UBO ===
layout(std140, binding = 0) uniform ShaderGlobalData {
    mat4 view;
    mat4 projection;
    vec4 camera_position;

    float time;
    float _pad_world_1;
    float _pad_world_2;
    float _pad_world_3;

    int light_directional_count;
    float _pad_light_dir_1;
    float _pad_light_dir_2;
    float _pad_light_dir_3;

    vec4 light_directional_directions[4];
    vec4 light_directional_colors[4]; // .xyz = color, .w = intensity

    int light_point_count;
    float _pad_light_point_1;
    float _pad_light_point_2;
    float _pad_light_point_3;

    vec4 light_point_colors[8];      // .xyz = color, .w = intensity
    vec4 light_point_positions[8];   // .xyz = position, .w = range
};

// Simple hash noise
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
    // Don't invert - use direction as-is (opposite of sea shader)
    vec3 lightDir = normalize(direction);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float toonDiffuse = toonShade(NdotL);
    return toonDiffuse * color * intensity;
}

// Calculate if fragment is in shadow (for main directional light)
float calcShadow(vec3 normal, vec3 lightDir) {
    float NdotL = dot(normal, lightDir);
    
    // If surface faces away from light (NdotL < 0), it's in shadow
    // Smooth transition around 0
    float shadow = smoothstep(-shadowSoftness, shadowSoftness, NdotL);
    
    return shadow; // 0 = full shadow, 1 = fully lit
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
    return toonDiffuse * color * intensity * attenuation;
}

void main() {
    vec4 baseColor = useTexture ? texture(textureSampler, texCoord) * vertexColor : vertexColor;

    vec3 viewPos = camera_position.xyz;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 norm = normalize(fragNormal);

    vec3 lighting = ambientStrength * ambientColor;
    
    // Calculate shadow factor from main directional light
    float shadowFactor = 1.0;
    if (light_directional_count > 0) {
        // Don't invert - use direction as-is
        vec3 mainLightDir = normalize(light_directional_directions[0].xyz);
        shadowFactor = calcShadow(norm, mainLightDir);
    }

    // Directional lights
    for (int i = 0; i < light_directional_count && i < 4; i++) {
        // Pass raw direction, function will invert (same as sea shader)
        vec3 dirLight = calcDirectionalLight(
            light_directional_directions[i].xyz,
            light_directional_colors[i].xyz,
            light_directional_colors[i].w,
            norm
        );
        
        // Apply shadow to first directional light only
        if (i == 0) {
            dirLight *= shadowFactor;
        }
        
        lighting += dirLight;
    }

    // Point lights
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

    // Detail & noise
    float detail = fbm(fragPos.xz * detailScale);
    float noise = pixelNoise(fragPos.xz * noiseScale);
    float combinedDetail = (mix(detail, noise, 0.5) - 0.5) * detailStrength;

    // Apply shadow color tint to shadowed areas
    vec3 finalLighting = lighting;
    if (light_directional_count > 0) {
        // Only apply shadow tint if actually in shadow (shadowFactor < 1.0)
        if (shadowFactor < 0.99) {
            // Mix between shadow ambient and full lighting
            finalLighting = mix(ambientStrength * shadowColor, lighting, shadowFactor);
        }
    }

    // Rim lighting
    float rimIntensity = 0.0;
    if (light_directional_count > 0) {
        // Don't invert - use direction as-is
        vec3 mainLightDir = normalize(light_directional_directions[0].xyz);
        float NdotL = max(dot(norm, mainLightDir), 0.0);
        float rimDot = 1.0 - max(dot(viewDir, norm), 0.0);
        rimIntensity = smoothstep(rimThreshold - 0.05, rimThreshold + 0.05, rimDot);
        rimIntensity *= step(0.2, NdotL);
    }

    vec3 result = baseColor.rgb * (finalLighting + vec3(combinedDetail));
    if (light_directional_count > 0) {
        result += rimIntensity * rimStrength * light_directional_colors[0].xyz * light_directional_colors[0].w * shadowFactor;
    }
    
    // Debug: Uncomment to visualize normals
    // FragColor = vec4(norm * 0.5 + 0.5, 1.0); return;
    
    // Debug: Uncomment to visualize lighting value
    // FragColor = vec4(vec3(finalLighting), 1.0); return;
    
    // Debug: Uncomment to visualize shadows
    // FragColor = vec4(vec3(shadowFactor), 1.0); return;
    
    FragColor = vec4(result, baseColor.a);
}