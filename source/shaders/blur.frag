#version 450

layout(set = 2, binding = 0) uniform sampler2D uTexture; // La textura a desenfocar
layout(set = 3, binding = 0) uniform BlurUniforms { // Uniforms
    vec2 texelSize; // 1.0 / ancho_textura, 1.0 / alto_textura
    float radius;   // Radio del desenfoque en píxeles
} ubo;

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = vec4(0.0);
    float totalWeight = 0.0;
    
    // Kernel de desenfoque (promedio simple)
    for (float x = -ubo.radius; x <= ubo.radius; x += 1.0) {
        for (float y = -ubo.radius; y <= ubo.radius; y += 1.0) {
            vec2 offset = vec2(x, y) * ubo.texelSize;
            color += texture(uTexture, vTexCoord + offset);
            totalWeight += 1.0;
        }
    }
    
    outColor = color / totalWeight; // Normalizar el color
}