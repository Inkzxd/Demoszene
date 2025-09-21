#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 fragCoord;
out vec2 texCoord;

void main() {
    fragCoord = aPos * vec2(1600.0, 1200.0); // Adjust these numbers to match your desired resolution
    texCoord = aTexCoord;
    gl_Position = vec4(aPos * 2.0 - 1.0, 0.0, 1.0);
}