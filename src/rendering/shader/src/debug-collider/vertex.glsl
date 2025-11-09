#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aColor;

// Model matrix as separate uniform (per-object data)
uniform mat4 model;

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

flat out vec4 vertexColor;
out vec2 texCoord;
flat out vec3 fragNormal;
out vec3 fragPos;
out vec3 worldPos;

void main() {
    vec4 worldPosition = model * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPosition;
    vec4 clipPos = projection * viewPos;
    
    gl_Position = clipPos;
    
    worldPos = vec3(worldPosition);
    fragPos = vec3(worldPosition);
    fragNormal = mat3(transpose(inverse(model))) * aNormal;
    vertexColor = aColor;
    texCoord = aTexCoord;
}