#version 450 // specifies tbe version of glsl we’re using. In this case, version 4.5 of glsl
vec3 positions[3] = vec3[3]( //an array of 3, 3-item vectors
		vec3(0.0f,0.0f, 0.0f),   //top  of triangle
		vec3(-0.5f, -0.5f, 0.0f), // the bottom left of the triangle.
		vec3(0.5f, -0.5f, 0.0f)); // bottom right of the triangle

void main(){ //output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}
