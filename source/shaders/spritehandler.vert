#version 450


layout (set = 1, binding = 0) uniform scren { vec2 u_resolution;} screen;
struct SpriteData {
    vec2 s_uv;
    vec2 s_size;
    float xpos;
    float ypos;
    float scalex;
    float scaley;
};



layout (set = 2, binding = 0) readonly buffer SpriteBuffer {
SpriteData sprites[];
};
layout (location = 0) out vec2 v_uv;
const vec2 corners[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(0,1),
    vec2(0,1), vec2(1,0), vec2(1,1)
);
void main(){
   vec2 a_pos = corners[gl_VertexIndex];
    SpriteData spr = sprites[gl_InstanceIndex];

    vec2 world = vec2(spr.xpos, spr.ypos) + vec2(spr.scalex, spr.scaley) * a_pos; //i dont get why making y pos  corner dependentbut x be constant, worked .m.
    vec2 butt  = (world / screen.u_resolution)* 2 - 1; //because were gonna put it in vec4 space, which is clipspace

    gl_Position = vec4(butt, 0.0, 1.0);
    v_uv = spr.s_uv + spr.s_size * a_pos;                }
