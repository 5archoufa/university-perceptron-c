#version 460 core

in vec2 texCoord;
in vec4 vertexColor;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform bool useTexture = false;

// Custom user-defined color
uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    vec4 baseColor = useTexture ? texture(textureSampler, texCoord) * vertexColor : color * vertexColor;
    FragColor = baseColor;
}
