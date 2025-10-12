#version 450 core

in vec4 vertexColor;
in vec2 texCoord;
in vec3 fragNormal;
in vec3 fragPos;
in vec3 worldPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

// Water parameters
uniform vec3 deepWaterColor = vec3(0.05, 0.25, 0.45);   // Dark blue
uniform vec3 shallowWaterColor = vec3(0.15, 0.55, 0.75); // Light blue
uniform vec3 highlightColor = vec3(0.6, 0.8, 0.95);      // Light cyan highlights
uniform float ambientStrength = 0.45;
uniform float specularStrength = 1.2;  // Higher for more shimmer
uniform float shininess = 64.0;        // Higher for tighter highlights
uniform float flowSpeed = 1;         // How fast water flows
uniform float flowScale = 5.0;         // Scale of flow patterns

// === Global UBO ===
layout(std140, binding = 0) uniform ShaderGlobalData {
    mat4 view;
    mat4 projection;
    vec4 camera_position;

    float time;
    float _pad_world_0;
    float _pad_world_1;
    float _pad_world_2;

    int light_directional_count;
    float _pad_dir_count_0;
    float _pad_dir_count_1;
    float _pad_dir_count_2;

    vec4 light_directional_directions[4];
    vec4 light_directional_colors[4]; // .xyz = color, .w = intensity

    int light_point_count;
    float _pad_point_0;
    float _pad_point_1;
    float _pad_point_2;

    vec4 light_point_colors[8];      // .xyz = color, .w = intensity
    vec4 light_point_positions[8];   // .xyz = position, .w = range
};

// Pixel art noise functions
float hash(vec2 p) {
    p = fract(p * vec2(127.34, 259.21));
    p += dot(p, p + 47.32);
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

// Calculate flowing water patterns
vec2 flowPattern(vec2 worldPos, float time) {
    // Create flowing direction based on multiple sine waves
    vec2 flow1 = vec2(sin(worldPos.y * 0.5 + time * 0.3), cos(worldPos.x * 0.4 + time * 0.25));
    vec2 flow2 = vec2(cos(worldPos.x * 0.3 - time * 0.2), sin(worldPos.y * 0.6 - time * 0.35));
    return (flow1 + flow2) * flowSpeed;
}

// Calculate water flow highlights
float calculateFlowHighlights(vec2 worldPos, float time) {
    vec2 flowDir = flowPattern(worldPos, time);
    
    // Create flowing highlight streaks (pixel/noise based)
    vec2 highlightPos = worldPos * flowScale + flowDir * 12.0;
    float highlight = pixelNoise(highlightPos + vec2(time * 0.8, time * 0.6));
    
    // Add directional flow lines (quantized effect)
    float flowLines = sin(worldPos.x * 8.0 + worldPos.y * 6.0 + time * 3.0) * 0.5 + 0.5;
    flowLines = step(0.8, flowLines);
    
    return (highlight * 0.7 + flowLines * 0.3);
}

vec3 calcDirectionalLight(vec3 direction, vec3 color, float intensity, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-direction);
    
    // Smooth diffuse
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NdotL * color * intensity;
    
    // Smooth specular (for water sparkle and shimmer)
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * color * intensity;
    
    return diffuse + specular;
}

vec3 calcPointLight(vec3 position, vec3 color, float intensity, float range, vec3 fragPos, vec3 normal, vec3 viewDir) {
    vec3 lightDir = position - fragPos;
    float distance = length(lightDir);
    float attenuation = clamp(1.0 - (distance / range), 0.0, 1.0);
    attenuation *= attenuation;
    if (attenuation <= 0.0) return vec3(0.0);

    lightDir = normalize(lightDir);
    
    // Smooth diffuse
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NdotL * color * intensity * attenuation;
    
    // Smooth specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * color * intensity * attenuation;
    
    return diffuse + specular;
}

void main() {
    vec3 viewPos = camera_position.xyz;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 norm = normalize(fragNormal);
    
    // Calculate animated depth variation with flow (pixelated fbm retained)
    vec2 flowDir = flowPattern(worldPos.xz, time);
    float depthNoise = fbm(worldPos.xz * 0.6 + flowDir * 2.0);
    float depthFactor = depthNoise * 0.7 + 0.3; // Keep some base depth
    vec3 baseWaterColor = vec3(0.0, 0.3, 0.5); // Pretty Ocean blue

    // Calculate flow highlights (liquid shimmer effect)
    float flowHighlight = calculateFlowHighlights(worldPos.xz, time);
    
    // Start with ambient lighting
    vec3 lighting = ambientStrength * vec3(1.0);

    // Directional lights
    for (int i = 0; i < light_directional_count && i < 4; i++) {
        lighting += calcDirectionalLight(
            light_directional_directions[i].xyz,
            light_directional_colors[i].xyz,
            light_directional_colors[i].w,
            norm,
            viewDir
        );
    }

    // Point lights
    for (int i = 0; i < light_point_count && i < 8; i++) {
        lighting += calcPointLight(
            light_point_positions[i].xyz,
            light_point_colors[i].xyz,
            light_point_colors[i].w,
            light_point_positions[i].w,
            fragPos,
            norm,
            viewDir
        );
    }
    
    // Apply lighting to water
    vec3 waterColor = baseWaterColor * lighting;
    
    // Add flowing highlights for liquid shimmer
    waterColor = mix(waterColor, highlightColor, flowHighlight * 0.15);
    
    // Final color (foam removed)
    vec3 finalColor = waterColor;
    
    // Alpha based only on depthFactor (deeper = more opaque)
    float alpha = mix(0.8, 1.0, depthFactor * 0.5);
    
    FragColor = vec4(finalColor, alpha);
}
