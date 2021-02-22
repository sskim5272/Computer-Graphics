#version 430 core

out vec4 fColor;
out vec3 fPosition_cs;
out vec3 fNormal_cs;
out vec2 fTexcoord;


in vec4 vPosition_ms;					  // vertex position in model space
in vec4 vNormal_ms;					      // normal in model space
in vec4 vColor;					          // vertex color
in vec2 vTexcoord;                        // texture coordinates

uniform mat4 P;							  // projection matrix
uniform mat4 V;							  // viewing transformation matrix
uniform mat4 VM;				          // VM = V*M
uniform mat4 U;						      // normal matrix (= transpose(inverse(VM)))

void main()
{
	fPosition_cs = vec3(VM * vPosition_ms);
	fNormal_cs = vec3(normalize(U * vNormal_ms));

	gl_Position = P * vec4(fPosition_cs, 1);
	fColor = vColor;
	fTexcoord = vTexcoord;
}