#version 460 core

flat in vec4 vertexColor;
in vec2 texCoord;
flat in vec3 fragNormal;
in vec3 fragPos;
in vec3 worldPos;

out vec4 FragColor;

// === Global UBO ===
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
    vec4 light_directional_colors[4]; // .xyz = color, .w = intensity

    int light_point_count;
    float _pad_light_point_1;
    float _pad_light_point_2;
    float _pad_light_point_3;

    vec4 light_point_colors[8];      // .xyz = color, .w = intensity
    vec4 light_point_positions[8];   // .xyz = position, .w = range
};

// Debug collider parameters
uniform float edgeThickness = 0.05;    // How thick the wireframe edges are
uniform vec3 edgeColor = vec3(1.0, 1.0, 1.0);  // White opaque edges
uniform vec3 faceColor = vec3(1.0, 0.2, 0.2);  // Red semi-transparent faces
uniform float faceAlpha = 0.2;         // Face transparency (more transparent)

void main() {
    // Simple approach: just render semi-transparent red faces with pulsing
    vec3 finalColor = faceColor;
    float finalAlpha = faceAlpha;
    
    // Add pulsing effect to the entire collider
    float pulse = 0.7 + 0.3 * sin(time * 3.0);
    finalColor *= pulse;
    
    FragColor = vec4(finalColor, finalAlpha);
}