#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float time;

// Simple noise function
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Barrel distortion
vec2 barrelDistort(vec2 uv, float amt) {
    uv = uv * 2.0 - 1.0;
    float r2 = uv.x * uv.x + uv.y * uv.y;
    uv *= 1.0 + amt * r2;
    return (uv + 1.0) * 0.5;
}

void main()
{
    // Barrel distortion (screen curve)
    vec2 uv = barrelDistort(TexCoords, 0.18);

    // RGB color offset (chromatic aberration)
    float offset = 0.0007; 
    float r = texture(screenTexture, uv + vec2(offset, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(offset, 0.0)).b;
    vec3 color = vec3(r, g, b);

    // Rolling scanline effect
    float scan = 0.7 + 0.3 * sin((uv.y + time * 1.1) * 900.0);
    color *= scan;

    // Vignette (darken edges)
    float vignette = smoothstep(0.8, 0.45, length(uv - 0.5));
    color *= vignette;

    // Add noise
    float noise = (rand(uv * time) - 0.5) * 0.12;
    color += noise;

    color = clamp(color, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}