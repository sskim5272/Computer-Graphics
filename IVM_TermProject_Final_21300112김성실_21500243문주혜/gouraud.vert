#version 430 core

out vec4 fColorFront;                     // color for the front-facing fragment
out vec4 fColorBack;                      // color for the back-facing fragment
out vec2 fTexcoord;

in vec4 vPosition_ms;					  // vertex position in model space
in vec4 vNormal_ms;					      // normal in model space
in vec4 vColor;					          // vertex color
in vec2 vTexcoord;                        // texture coordinates

uniform vec3 Ka;						  // ambient reflectivity
uniform vec3 Kd;						  // diffuse reflectivity
uniform vec3 Ks;						  // specular reflectivity
uniform float n;						  // specular intensity factor

uniform vec3 LightPos_cs;		          // light source position in camera space
uniform vec3 Il;						  // light source intensity
uniform vec3 Ia;						  // ambient light intensity
uniform vec3 c;							  // coefficients for light source attenuation

uniform mat4 P;							  // projection matrix
uniform mat4 V;							  // viewing transformation matrix
uniform mat4 VM;				          // VM = V*M
uniform mat4 U;						      // normal matrix (= transpose(inverse(VM)))


vec4 shading(vec3 vPosition_cs, vec3 vNormal_cs)
{          
	vec3 N = normalize(vNormal_cs);										          // normal vector
	vec3 L = LightPos_cs - vPosition_cs;										  // light vector

	float d = length(L);														  // distance from light source to object
	L = L/d;																	  // normalize the light vector

	vec3 V = normalize(vec3(0.0) - vPosition_cs);								  // view vector
	vec3 R = reflect(-L, N);													  // reflection vector

	float fatt = min(1.0 / (c[0] + c[1] * d + c[2] * d * d), 1.0);				  // the farther away, the light is attenuated

	float cos_theta = max(dot(N, L), 0);										  // max cos value = 1,
	float cos_alpha = max(dot(V, R), 0);										  // if the angle exceeds 90 degrees, set to 0

	vec3 I = (Ia * Ka) + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) ;
	I = I * vec3(vColor);                                                         // Apply intensity
	return vec4(I,1);
}

void main()
{
	vec3 vNormal_cs = vec3(normalize(U * vNormal_ms));							  // normal transformation to camera space
	vec3 vPosition_cs = vec3(VM * vPosition_ms);								  // vertex transformation to camera space

	gl_Position = P * vec4(vPosition_cs, 1);									  // apply projection transformation

	fColorFront = shading(vPosition_cs,  vNormal_cs);							  // fragment is originated from a front-facing face
	fColorBack  = shading(vPosition_cs, -vNormal_cs);							  // revert the normal
	fTexcoord = vTexcoord;
}