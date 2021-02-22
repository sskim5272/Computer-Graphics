#version 430 core

in vec4 fColorFront, fColorBack;
in vec2 fTexcoord;

uniform int special;
uniform int ColorMode;
uniform int ObjectCode;
uniform sampler2D sampler;

void main()
{
    if (ColorMode == 1){
		gl_FragColor = vec4(0,0,1,1);             // Set wireframe colors to blue
	}
	else if (ColorMode == 2){
		float nc = ObjectCode / 255.0;
		gl_FragColor = vec4(nc,nc,nc,1); 
	}
	else{
		if ( gl_FrontFacing ) gl_FragColor = fColorFront;               // Fragment is originated from a front-facing face
		else				  gl_FragColor = fColorBack;                // Revert the normal
		
	}
	if(special == 0)
	{
	gl_FragColor *= texture(sampler, fTexcoord);
	}
	else {
	gl_FragColor = texture(sampler, fTexcoord);
	}
}
