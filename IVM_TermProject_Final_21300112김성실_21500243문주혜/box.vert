#version 430 core

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;

uniform mat4 T;

layout(location=1) uniform mat4 M;
layout(location=2) uniform mat4 V;
layout(location=3) uniform mat4 P;
layout(location=7) uniform int r;
layout(location=8) uniform int g;
layout(location=9) uniform int b;

void main()
{
	gl_Position = P*V*M *T* vPosition;
	fColor=vec4(20*r/255.0, 20*g/255.0,20*b/255.0,1);
}