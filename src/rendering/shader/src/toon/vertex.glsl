#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aColor;

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
    vec4 light_directional_colors[4];
    int light_point_count;
    float _pad_light_point_1;
    float _pad_light_point_2;
    float _pad_light_point_3;
    vec4 light_point_colors[8];
    vec4 light_point_positions[8];
};

out vec4 vertexColor;
out vec2 texCoord;
out vec3 fragNormal;
out vec3 fragPos;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPos;
    vec4 clipPos = projection * viewPos;
    
    gl_Position = clipPos;
    fragPos = vec3(worldPos);
    
    // CRITICAL FIX: For uniform scale, just use the upper 3x3
    // For non-uniform scale, you need the normal matrix
    // Since we're now using proper column-major matrices:
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    
    // Transform and normalize the normal
    fragNormal = normalize(normalMatrix * aNormal);
    
    vertexColor = aColor;
    texCoord = aTexCoord;
}