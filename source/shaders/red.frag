#version 450

layout(set = 2, binding = 0) uniform sampler2D uTexture;
layout (set = 3, binding = 0) uniform colorblock{
vec4 color2;

float tintintensity;

} ubo;
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    outColor = vec4((color.r + (ubo.color2.r * ubo.tintintensity)) /(1 + ubo.tintintensity)* color.a, (color.g + (ubo.color2.g * ubo.tintintensity))/(1 + ubo.tintintensity) * color.a, (color.b + (ubo.color2.b * ubo.tintintensity))/(1 + ubo.tintintensity) * color.a, ubo.color2.a);} //avoid values above 1