#version 450

layout(location = 0) out vec2 vTexCoord;

void main() {
    // Generate a triangle that covers the whole screen using only gl_VertexIndex.
    // gl_VertexIndex is 0, 1, 2 when you draw 3 vertices.
    //
    // (gl_VertexIndex << 1) & 2 gives:
    //   index 0 -> 0
    //   index 1 -> 2
    //   index 2 -> 2
    //
    // gl_VertexIndex & 2 gives:
    //   index 0 -> 0
    //   index 1 -> 0
    //   index 2 -> 2
    //
    // So the three positions are (0,0), (2,0), (0,2).
    // Multiplied by 2 and shifted by -1:
    //   (0,0) -> (-1,-1)  bottom-left
    //   (2,0) -> ( 3,-1)  far right
    //   (0,2) -> (-1, 3)  far top
    //
    // That triangle is way bigger than the screen, so only the part
    // that overlaps the screen [-1,1] x [-1,1] is rasterized.
    // This is the standard "fullscreen triangle" trick.
    
    vec2 pos = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
    
    // Pass the position as UV. The visible part of the triangle
    // corresponds to UV range [0,1] x [0,1], which is exactly
    // the texture coordinate space we want.
    vTexCoord = pos;
}