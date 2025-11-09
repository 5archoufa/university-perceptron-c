// ui-text/vertex.glsl - Updated to use UBO
#version 460 core

// Input attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uint aColor;

// Use the global UBO instead of individual uniforms
uniform mat4 uModel;

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

// Output to fragment shader
out vec2 TexCoord;
out vec4 VertexColor;

void main()
{
    // Transform position using UBO matrices
    gl_Position = projection * view * uModel * vec4(aPosition, 1.0);
    
    // Pass texture coordinates and color to fragment shader
    TexCoord = aTexCoord;
    
    // Convert packed RGBA color to vec4
    VertexColor = vec4(
        float((aColor >> 24) & 0xFF) / 255.0,  // R
        float((aColor >> 16) & 0xFF) / 255.0,  // G
        float((aColor >> 8) & 0xFF) / 255.0,   // B
        float(aColor & 0xFF) / 255.0           // A
    );
}