#version 330 core
out vec4 FragColor;
in vec2 fragCoord;
in vec2 texCoord;

uniform vec2 iResolution;
uniform float iTime;
uniform float closeAnim;
uniform sampler2D screenTexture;

void main() {
    vec2 uv = texCoord;
    
    // Faster collapse animation (0.0-1.0 range)
    float anim = clamp(closeAnim * 2.0, 0.0, 1.0); // Double the speed
    
    // Collapsing tube effect
    float centerY = 0.5;
    float edgeDist = abs(uv.y - centerY);
    
    // Calculate the visible band height (collapsing to a line)
    float bandHeight = 1.0 - anim; // Inverted for faster collapse
    float visibleHeight = bandHeight * 0.5;
    
    // Create mask for the visible area
    float mask = smoothstep(visibleHeight + 0.01, visibleHeight, edgeDist);
    
    // Final color - blend between CRT and collapse effect
    vec3 finalColor;
    
    if (anim < 0.8) {
        // During collapse - show CRT content within the visible band
        finalColor = texture(screenTexture, uv).rgb * mask;
        
        // Add white scanline in the center when almost collapsed
        if (anim > 0.6) {
            float scanLine = 1.0 - smoothstep(0.003, 0.006, edgeDist);
            finalColor = mix(finalColor, vec3(1.0), scanLine * 0.9);
        }
    } else if (anim < 1.0) {
        // Final collapse to white line then black
        float scanLine = 1.0 - smoothstep(0.001, 0.002, edgeDist);
        finalColor = vec3(scanLine);
        
        // Quick fade to black
        float fade = 1.0 - smoothstep(0.9, 1.0, anim);
        finalColor *= fade;
    } else {
        // Fully collapsed - complete black
        finalColor = vec3(0.0);
    }
    
    FragColor = vec4(finalColor, 1.0);
}