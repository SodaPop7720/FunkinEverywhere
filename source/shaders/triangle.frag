#version 450

layout (location = 0) out vec4 sentcolor; //this allows us to send pixel data to the window.

void main (){

sentcolor = vec4(1.0, 0.0, 0.0,1.0); //(r,g,b, transparency)
}

