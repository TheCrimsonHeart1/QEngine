#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex0;  // Changed from spriteTexture
uniform vec4 spriteColor; // tint (e.g. white = no tint)

void main() {
    vec4 texColor = texture(tex0, TexCoord);
    FragColor = texColor * spriteColor;
}