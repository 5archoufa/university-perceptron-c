#version 460 core

in vec3 texCoord;
out vec4 FragColor;

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

// Configurable sun parameters
uniform float sunSize = 0.15;           // Sun radius (0.1 - 0.3)
uniform vec3 sunColorNoon = vec3(1.0, 0.95, 0.8);      // Noon sun color (bright yellow)
uniform vec3 sunColorSunrise = vec3(1.0, 0.4, 0.2);    // Sunrise/sunset sun color (orange)
uniform vec3 sunColorNight = vec3(0.8, 0.8, 0.9);      // Moon color (pale blue-white)

// Pixel art quantization
const int colorSteps = 16;  // Number of color steps for pixel art look

// Quantize color to pixel art palette
vec3 quantizeColor(vec3 color) {
    return floor(color * float(colorSteps)) / float(colorSteps);
}

// Smooth interpolation for time of day
float smoothTimeTransition(float t, float center, float width) {
    return smoothstep(center - width, center + width, t);
}

void main() {
    vec3 viewDir = normalize(texCoord);
    float verticalGradient = viewDir.y; // -1 (bottom) to +1 (top)
    
    // ===== TIME OF DAY CALCULATIONS =====
    // timeOfDay: 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset, 1.0 = midnight
    float tod = timeOfDay;
    
    // Calculate day/night blend
    float dayAmount = 0.0;
    if (tod < 0.25) {
        // Night to sunrise (0.0 -> 0.25)
        dayAmount = smoothstep(0.15, 0.25, tod);
    } else if (tod < 0.75) {
        // Day (0.25 -> 0.75)
        dayAmount = 1.0;
    } else {
        // Sunset to night (0.75 -> 1.0)
        dayAmount = 1.0 - smoothstep(0.75, 0.85, tod);
    }
    
    // ===== SKY COLOR GRADIENTS =====
    // Day colors
    vec3 dayTopColor = vec3(0.4, 0.7, 1.0);        // Sky blue
    vec3 dayHorizonColor = vec3(0.7, 0.85, 1.0);   // Light blue
    vec3 dayBottomColor = vec3(0.8, 0.9, 0.95);    // Very light blue
    
    // Sunrise/sunset colors
    vec3 sunriseTopColor = vec3(0.3, 0.4, 0.7);    // Purple-blue
    vec3 sunriseHorizonColor = vec3(1.0, 0.6, 0.3); // Orange
    vec3 sunriseBottomColor = vec3(0.9, 0.7, 0.5);  // Warm yellow
    
    // Night colors
    vec3 nightTopColor = vec3(0.05, 0.05, 0.15);   // Dark blue
    vec3 nightHorizonColor = vec3(0.1, 0.1, 0.2);  // Slightly lighter
    vec3 nightBottomColor = vec3(0.15, 0.15, 0.25); // Even lighter at bottom
    
    // Calculate sunrise/sunset intensity
    float sunriseSunsetAmount = 0.0;
    if (tod < 0.25) {
        sunriseSunsetAmount = smoothstep(0.15, 0.25, tod) * (1.0 - smoothstep(0.25, 0.35, tod));
    } else if (tod > 0.75) {
        sunriseSunsetAmount = smoothstep(0.65, 0.75, tod) * (1.0 - smoothstep(0.75, 0.85, tod));
    }
    
    // Blend between night and day colors
    vec3 topColor = mix(nightTopColor, dayTopColor, dayAmount);
    vec3 horizonColor = mix(nightHorizonColor, dayHorizonColor, dayAmount);
    vec3 bottomColor = mix(nightBottomColor, dayBottomColor, dayAmount);
    
    // Add sunrise/sunset colors
    topColor = mix(topColor, sunriseTopColor, sunriseSunsetAmount);
    horizonColor = mix(horizonColor, sunriseHorizonColor, sunriseSunsetAmount);
    bottomColor = mix(bottomColor, sunriseBottomColor, sunriseSunsetAmount);
    
    // Create gradient
    vec3 skyColor;
    if (verticalGradient > 0.0) {
        // Top half: horizon to top
        skyColor = mix(horizonColor, topColor, verticalGradient);
    } else {
        // Bottom half: bottom to horizon
        skyColor = mix(horizonColor, bottomColor, -verticalGradient);
    }
    
    // ===== SUN/MOON POSITION =====
    // Sun moves from east (-X) to west (+X), rising from bottom to top
    float sunAngle = (tod - 0.25) * 3.14159265 * 2.0; // Sunrise at 0 radians
    vec3 sunDirection = vec3(
        -cos(sunAngle),  // X: moves from -1 to +1
        sin(sunAngle),   // Y: rises and falls
        0.3              // Z: slightly in front
    );
    sunDirection = normalize(sunDirection);
    
    // Moon position (opposite of sun)
    vec3 moonDirection = -sunDirection;
    
    // ===== SUN/MOON RENDERING =====
    float sunDot = dot(viewDir, sunDirection);
    float moonDot = dot(viewDir, moonDirection);
    
    // Determine if we should show sun or moon
    bool showSun = (tod > 0.1 && tod < 0.9);
    vec3 celestialDir = showSun ? sunDirection : moonDirection;
    float celestialDot = showSun ? sunDot : moonDot;
    
    // Calculate sun/moon color based on time
    vec3 celestialColor;
    if (showSun) {
        // Blend between sunrise, noon, and sunset colors
        if (tod < 0.5) {
            float t = (tod - 0.25) / 0.25; // 0 to 1 from sunrise to noon
            celestialColor = mix(sunColorSunrise, sunColorNoon, smoothstep(0.0, 1.0, t));
        } else {
            float t = (tod - 0.5) / 0.25; // 0 to 1 from noon to sunset
            celestialColor = mix(sunColorNoon, sunColorSunrise, smoothstep(0.0, 1.0, t));
        }
    } else {
        celestialColor = sunColorNight; // Moon
    }
    
    // Sun/moon disc with smooth edge
    float celestialMask = smoothstep(sunSize + 0.02, sunSize, acos(celestialDot));
    
    // Add glow around sun/moon
    float glowSize = sunSize * 2.5;
    float glowMask = smoothstep(glowSize, sunSize, acos(celestialDot)) * 0.5;
    
    // Combine sky with sun/moon
    vec3 finalColor = mix(skyColor, celestialColor, celestialMask);
    finalColor += celestialColor * glowMask * 0.3; // Add glow
    
    // ===== PIXEL ART QUANTIZATION =====
    finalColor = quantizeColor(finalColor);
    
    FragColor = vec4(finalColor, 1.0);
}