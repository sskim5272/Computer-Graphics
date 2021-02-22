#version 430 core

in vec4 fColor;
in vec3 fPosition_cs;
in vec3 fNormal_cs;
in vec2 fTexcoord;

uniform int special;
uniform int ColorMode;
uniform int ObjectCode;
uniform sampler2D sampler;

uniform vec3 Ka;			              // ambient reflectivity
uniform vec3 Kd;						  // diffuse reflectivity
uniform vec3 Ks;						  // specular reflectivity
uniform float n;						  // specular intensity factor

uniform vec3 LightPos_cs;				  // light source position in camera space
uniform vec3 Il;						  // light source intensity
uniform vec3 Ia;						  // ambient light intensity
uniform vec3 c;							  // coefficients for light source attenuation


vec4 shading(vec3 fPosition_cs, vec3 fNormal_cs)
{          
	vec3 N = normalize(fNormal_cs);												   // normal vector
	vec3 L = LightPos_cs - fPosition_cs;										   // light vector

	float d = length(L);														   // distance from light source to object
	L = L/d;																	   // normalize the light vector

	vec3 V = normalize(vec3(0.0) - fPosition_cs);								   // view vector
	vec3 R = reflect(-L, N);													   // reflection vector

	float fatt = min(1.0 / (c[0] + c[1] * d + c[2] * d * d), 1.0);				   // the farther away, the light is attenuated

	float cos_theta = max(dot(N, L), 0);										   // max cos value = 1,
	float cos_alpha = max(dot(V, R), 0);										   // if the angle exceeds 90 degrees, set to 0

	vec3 I = (Ia * Ka) + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) ;
	I = I * vec3(fColor);
	return vec4(I,1);
}

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
		if ( gl_FrontFacing ) gl_FragColor = shading(fPosition_cs, fNormal_cs);               // Fragment is originated from a front-facing face
		else				  gl_FragColor = shading(fPosition_cs, -fNormal_cs);              // Revert the normal
		
	}
	if(special == 0)
	{
	gl_FragColor *= texture(sampler, fTexcoord);
	}
	else {
	gl_FragColor = texture(sampler, fTexcoord);
	}
}
