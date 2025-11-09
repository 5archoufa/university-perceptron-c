#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D cameraTexture;

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

// --- Helpers: Pixelated noise ---
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Low-frequency dirt blotches
float blotch(vec2 uv) {
    vec2 id = floor(uv * 5.0);
    float h = hash(id);
    return smoothstep(0.3, 0.8, h);
}

// Pixelation helper
vec2 pixelate(vec2 uv, float pixels) {
    return floor(uv * pixels) / pixels;
}

void main()
{
    vec4 baseColor = texture(cameraTexture, TexCoords);

    if (baseColor.a == 0.0) {
        FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    // --- Pixelation level (bigger = chunkier effect) ---
    vec2 uv = pixelate(TexCoords, 120.0);

    // --- Base paper tint ---
    vec3 paperTint = vec3(0.94, 0.88, 0.76); // warm vintage beige
    vec3 color = mix(baseColor.rgb, paperTint, 0.4);

    // --- Add static dirt blotches ---
    float dirt = blotch(uv * 2.0 + 0.37);
    dirt += blotch(uv * 3.3 - 1.1);
    dirt *= 0.6;
    color *= (1.0 - dirt * 0.1);

    // --- Add vignette ---
    float distToCenter = distance(uv, vec2(0.5));
    float vignette = smoothstep(0.9, 0.4, distToCenter);
    color *= mix(0.8, 1.0, vignette);

    // --- Add static pixel grain (stronger & bigger) ---
    vec2 grainUV = pixelate(uv * 220.0, 220.0);
    float grain = hash(grainUV + vec2(0.47, 0.28));
    color *= 1.0 - (grain - 0.5) * 0.05;

    // --- Slight desaturation for old worn look ---
    float gray = dot(color, vec3(0.3, 0.59, 0.11));
    color = mix(vec3(gray), color, 0.82);

    FragColor = vec4(color, baseColor.a);
}
