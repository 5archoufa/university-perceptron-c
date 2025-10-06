#version 460 core
flat in vec4 vertexColor;
in vec2 texCoord;
flat in vec3 fragNormal;
in vec3 fragPos;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

// Toon shader parameters
uniform int toonLevels = 3;
uniform float ambientStrength = 0.35;
uniform vec3 ambientColor = vec3(1.0, 1.0, 1.0);

// Directional lights
#define MAX_DIR_LIGHTS 4
uniform int numDirLights = 0;
uniform vec3 dirLightDirections[MAX_DIR_LIGHTS];
uniform vec3 dirLightColors[MAX_DIR_LIGHTS];
uniform float dirLightIntensities[MAX_DIR_LIGHTS];

// Point lights
#define MAX_POINT_LIGHTS 8
uniform int numPointLights = 0;
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform float pointLightIntensities[MAX_POINT_LIGHTS];
uniform float pointLightRanges[MAX_POINT_LIGHTS];

// Rim lighting
uniform float rimThreshold = 0.7;
uniform float rimStrength = 0.25;

// Detail noise
uniform float detailScale = 10.0;
uniform float detailStrength = 0.1;
uniform float noiseScale = 50.0;

// Camera position
uniform vec3 viewPos = vec3(0.0, 5.0, 10.0);

out vec4 FragColor;

// Hash function for noise
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Sharp pixel noise
float pixelNoise(vec2 p) {
    return hash(floor(p));
}

// Layered noise
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

// Toon quantization
float toonShade(float value) {
    float toonLevel = floor(value * float(toonLevels));
    return toonLevel / max(float(toonLevels - 1), 1.0);
}

// Calculate directional light contribution
vec3 calcDirectionalLight(vec3 direction, vec3 color, float intensity, vec3 normal) {
    vec3 lightDir = normalize(-direction);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float toonDiffuse = toonShade(NdotL);
    return toonDiffuse * color * intensity;
}

// Calculate point light contribution
vec3 calcPointLight(vec3 position, vec3 color, float intensity, float range, vec3 fragPos, vec3 normal) {
    vec3 lightDir = position - fragPos;
    float distance = length(lightDir);
    
    // Attenuation
    float attenuation = clamp(1.0 - (distance / range), 0.0, 1.0);
    attenuation *= attenuation; // Quadratic falloff
    
    if (attenuation <= 0.0) return vec3(0.0);
    
    lightDir = normalize(lightDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float toonDiffuse = toonShade(NdotL * attenuation);
    
    return toonDiffuse * color * intensity * attenuation;
}

void main() {
    // Base color
    vec4 baseColor = useTexture ? texture(textureSampler, texCoord) * vertexColor : vertexColor;
    
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 norm = normalize(fragNormal);
    
    // Start with ambient lighting
    vec3 lighting = ambientStrength * ambientColor;
    
    // Add all directional lights
    for (int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        lighting += calcDirectionalLight(
            dirLightDirections[i],
            dirLightColors[i],
            dirLightIntensities[i],
            norm
        );
    }
    
    // Add all point lights
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        lighting += calcPointLight(
            pointLightPositions[i],
            pointLightColors[i],
            pointLightIntensities[i],
            pointLightRanges[i],
            fragPos,
            norm
        );
    }
    
    // Procedural detail
    float detail = fbm(fragPos.xz * detailScale);
    float noise = pixelNoise(fragPos.xz * noiseScale);
    float combinedDetail = (mix(detail, noise, 0.5) - 0.5) * detailStrength;
    
    // Rim lighting (using first directional light if available)
    float rimIntensity = 0.0;
    if (numDirLights > 0) {
        vec3 mainLightDir = normalize(-dirLightDirections[0]);
        float NdotL = max(dot(norm, mainLightDir), 0.0);
        float rimDot = 1.0 - max(dot(viewDir, norm), 0.0);
        rimIntensity = smoothstep(rimThreshold - 0.05, rimThreshold + 0.05, rimDot);
        rimIntensity *= step(0.2, NdotL);
    }
    
    // Final color
    vec3 result = baseColor.rgb * (lighting + vec3(combinedDetail));
    if (numDirLights > 0) {
        result += rimIntensity * rimStrength * dirLightColors[0] * dirLightIntensities[0];
    }
    
    FragColor = vec4(result, baseColor.a);
}