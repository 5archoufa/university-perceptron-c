#version 460 core

// Input from vertex shader
in vec2 TexCoord;
in vec4 VertexColor;

// Uniforms
uniform sampler2D uTexture;    // Font atlas texture
uniform vec4 uColor;           // Tint color

// Output
out vec4 FragColor;

void main()
{
    // Sample the font atlas (grayscale texture)
    float alpha = texture(uTexture, TexCoord).r;
    
    // Apply the color from the uColor uniform
    vec3 finalColor = uColor.rgb;
    float finalAlpha = alpha * uColor.a;
    
    // Output final color
    FragColor = vec4(finalColor, finalAlpha);
    
    // Discard fully transparent pixels for better performance
    if (finalAlpha < 0.01)
        discard;
}