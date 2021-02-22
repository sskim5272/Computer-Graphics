#version 430 core

in vec4 fColor;

uniform int MODE=0;


void main()
{
	if(MODE==0){
	gl_FragColor = fColor;
	}
	else if(MODE==1){
	gl_FragColor=vec4(1.0f);
	}
}