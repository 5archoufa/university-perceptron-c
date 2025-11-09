#version 460 core

layout(location = 0) in vec3 aPos;

out vec3 texCoord;

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

void main() {
    texCoord = aPos;
    
    // Remove translation from view matrix for skybox
    mat4 viewNoTranslation = mat4(mat3(view));
    
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    
    // Set z = w so that after perspective divide, z/w = 1.0 (max depth)
    gl_Position = pos.xyww;
}