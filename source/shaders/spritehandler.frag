#version 450


layout (set = 2, binding = 0) uniform sampler2D uTexture;
layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 outColor;

void main(){
    vec4 color = texture(uTexture, v_uv);

    outColor = color;
}