#include <stdio.h>
#include <cstring>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <time.h>
#include "box.h"
#include "LoadShaders.h"
#include "utils.h"
#include "loadobj.h"
#include <iostream>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>




#define UVAR(name, i)        glGetUniformLocation(program[i], name)
#define UVARS(name)          UVAR(name, shading_mode)
#define UMATS(name, M)       glUniformMatrix4fv(UVARS(name), 1, GL_FALSE, value_ptr(M))

///////////////////////////////////////////////////////////////////////////////////////////////////
//objState
struct ObjState
{
	GLfloat theta;
	GLfloat orbit_theta;
	GLfloat orbit_radius;
	GLfloat up_down;

	ObjState() :
		theta(0),
		orbit_theta(0),
		orbit_radius(0) {}

	glm::mat4 get_transf()
	{
		using namespace glm;
		mat4 M(1.0f);
		M = rotate(M, orbit_theta, vec3(0.0f, 1.0f, 0.0f));
		
		M = rotate(M, theta, vec3(0.0f, 1.0f, 0.0f));
		M = translate(M, vec3(orbit_radius, up_down, orbit_radius));
		return M;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//Cube -manage cube state
struct Cube
{
	glm::vec3 color;
	bool is_visible = true;          
	bool is_true = true;


}cube[10][10][10];


//// Variable initialization
enum { MODEL_RABBIT, MODEL_BACKGROUND, NUM_OF_MODELS };

const char* model_files[NUM_OF_MODELS] = { "models/rabbit_lighting.obj", "models/background_2.obj" };
float model_scales[NUM_OF_MODELS] = { 3.0f,3.3f };

ObjState mstate[NUM_OF_MODELS];

std::vector<tinyobj::real_t>     vertices[NUM_OF_MODELS];
std::vector<tinyobj::real_t>     normals[NUM_OF_MODELS];
std::vector<tinyobj::real_t>     colors[NUM_OF_MODELS];
std::vector<size_t>              vertex_map[NUM_OF_MODELS];
std::vector<tinyobj::shape_t>    shapes[NUM_OF_MODELS];
std::vector<tinyobj::material_t> materials[NUM_OF_MODELS];
std::vector<tinyobj::real_t>     texcoords[NUM_OF_MODELS];
std::map<std::string, size_t>    texmap[NUM_OF_MODELS];

typedef std::vector<GLfloat> GLvec;

bool is_obj_valid = false;
bool is_tex_valid = false;

GLuint vao[NUM_OF_MODELS][3] = { 0 };
GLuint vbo[NUM_OF_MODELS][4] = { 0 };



GLuint box_vao;
GLuint box_vbo[2];
GLvec box_vert;
std::vector<GLfloat> box_col;


GLuint program[4] = { 0 };
GLuint location;

GLuint ColorMode = 0;					   // Initialize wireframe
int projection_mode = 0;				   // Initialize projection mode
int shading_mode = 0;					   // Initialize shading mode
int plane_mode = 0;						   // initialize 3d plane_mode

int button_pressed[3] = { GLUT_UP };       // Initiallize mouse button
int mouse_pos[2] = { 0 };                  // Initiallize mouse position to (0,0)

std::vector<glm::vec3> idx_deleted;        // detecive cube position

void render(int color_mode);

glm::vec3 detective_position = glm::vec3(0.0f, 0.0f, 0.0f); //dective box position

int flag_color = 1;						   //flag for Initializing cube color  
bool flag_win = true;					   //flag for win_sentece	

///////////////////////////////////////////////////////////////////////////////////////////////////
// Viewing transformation structure
struct Camera
{
	glm::vec3 eye;                                                      // look-from
	glm::vec3 center;                                                   // look-at
	glm::vec3 up;                                                       // look-up

	Camera() : eye(0, 0, 5), center(0, 0, 0), up(0, 1, 0) {}            // Camera constructor
	glm::mat4 get_transf() { return lookAt(eye, center, up); }          // Return transformation matrix
}
camera;



///////////////////////////////////////////////////////////////////////////////////////////////////
// check cube state
bool check()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			for (int k = 0; k < 10; k++)
			{
				if ((cube[i][j][k].is_true != cube[i][j][k].is_visible)) {
					return 0;
				}
			}
		}
	}
	if (flag_win) {
		std::cout << "win!!!!!!!!!!" << std::endl;
		flag_win = false;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Keep tracking of the mouse state
void mouse(int button, int state, int x, int y)
{
	

	if (state == GLUT_UP) {
		unsigned char res[4];
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);    // Revert y-axis
		int i=0;
		int j = 0;
		int k = 0;
		if (res[0] == 0.0f) {
			i = 0;
		}
		else {
			i = int(res[0] / 20);
		}
		if (res[1] == 0.0f) {
			 j = 0;
		}
		else {
			j = int(res[1] / 20);
		}
		if (res[2] == 0.0f) {
			 k = 0;
		}
		else {
			k = int(res[2] / 20);
		}
	
		
		if (i < 10 && j < 10 && k < 10&&!check()) {
			cube[i][j][k].is_visible = 0;
			idx_deleted.push_back(glm::vec3{ i,j,k });
			std::cout << " ( x : " << idx_deleted.back()[0] << ", y : " << idx_deleted.back()[0] << ", z : " << idx_deleted.back()[0] <<" ) cube is deleted" << "\n"<< std::endl;


		}
	}
	
	button_pressed[button] = state;                              // Change the state of mouse button
	mouse_pos[0] = x;                   mouse_pos[1] = y;        // Current mouse position
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//gamepad
void gamepad(unsigned int buttonMask, int x, int y, int z)
{
	if (x > -100 && x < 100) x = 0;
	if (y > -100 && y < 100) y = 0;
	if (z > -100 && z < 100) z = 0;
	GLfloat tx = x / 1000.f;
	GLfloat ty = -y / 1000.f;
	GLfloat tz = z / 1000.f;


	if (tx > 0.0f) detective_position[0] += 1;			//x+1
	else if (tx<0.0f) detective_position[0] -= 1;		//x-1
	else if (ty>0.0f) detective_position[1] += 1;		//y+1
	else if (ty<0.0f) detective_position[1] -= 1;		//y-1
	else if (tz>0.0f) detective_position[2] += 1;		//z+1
	else if (tz<0.0f) detective_position[2] -= 1;		//z-1



	glutPostRedisplay();
}

// Dragging mouse
void motion(int x, int y)
{
	using namespace glm;
	int modifiers = glutGetModifiers();                          // Returns the state of modifier keys
	int is_ctrl_active = modifiers & GLUT_ACTIVE_CTRL;           // If the user pressed CTRL key, assign the value 1 (Bitwise operation)
	int w = glutGet(GLUT_WINDOW_WIDTH);                          // Get window width  (512)
	int h = glutGet(GLUT_WINDOW_HEIGHT);                         // Get window height (512)
	GLfloat dx =  1.0f * (x - mouse_pos[0]) / w;                 // Normalize to 0 ~ 1
	GLfloat dy = -1.0f * (y - mouse_pos[1]) / h;                 // Normalize to 0 ~ 1

	if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN) {
		if (is_ctrl_active) {
			// If the user pressed CTRL button, move the camera horizontally without any rotation
			mat4 VT = transpose(lookAt(camera.eye, camera.center, camera.up));   // transpose to match the same coordinate system
			camera.eye += vec3(dx*VT[0] + dy*VT[1]);                             // Update "look-from" 
			camera.center += vec3(dx*VT[0] + dy*VT[1]);                          // Update "look-at"
		}
		else {
			// Rotate the camera while fixing the target position
			vec4 disp(camera.eye - camera.center, 1);                            // Calculate the distance
			GLfloat alpha = 2.0f;                                                // Scaling factor, 커질 수록 꺾이는 정도가 커진다          
			mat4 V = lookAt(camera.eye, camera.center, camera.up);
			mat4 R = rotate(mat4(), -alpha*dx, vec3(0, 1, 0));                   // Rotation matrix
			R = rotate(R, alpha*dy, vec3(transpose(V)[0]));
			camera.eye = camera.center + vec3(R * disp);                         // Update "look-from" 
			camera.up = mat3(R)*camera.up;                                       // Update new "look-up"
		}
	}
	mouse_pos[0] = x;              mouse_pos[1] = y;                             // Update current mouse postion
}

///////////////////////////////////////////////////////////////////
// Zoom in/out the scene
void mouse_wheel(int wheel, int dir, int x, int y)
{
	using namespace glm;
	vec4 disp(camera.eye - camera.center, 1);                  // Calculate the distance
	if (dir > 0) {                                             // direction = +1 (Scrolling up): zoom in
		camera.eye = camera.center + vec3(0.95f*disp);
	}
	else {                                                     // direction = -1 (Scrolling up): zoom out
		camera.eye = camera.center + vec3(1.05f*disp);
	}
}

//////////////////////////////////////////////////////////////////////
// When the user pressed ASCII character
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		// Manage the wireframe
	case 'w': ColorMode == 0 ? ColorMode = 1 : ColorMode = 0;
		printf("show_wireframe = %d\n", ColorMode);
		glutPostRedisplay();
		break;
		//Manage the 3d plane
	case 'a': plane_mode == 0 ? plane_mode = 1 : plane_mode = 0;
		printf("plane_mode = %d\n", plane_mode);
		glutPostRedisplay();
		break;

		// Change the shading mode
	case '1':
	case '2':
	case '3':
		shading_mode = key - '1';
		printf("shading mode: %d\n", shading_mode);
		glutPostRedisplay();
		break;
		//delete cube at detective cube position and add it's positom to vector
	case'-': 
		if ((detective_position[0] < 10) && (detective_position[1] < 10) && (detective_position[2] < 10) && !check()&&(cube[int(detective_position[0])][int(detective_position[1])][int(detective_position[2])].is_visible == 1))
		{
			cube[int(detective_position[0])][int(detective_position[1])][int(detective_position[2])].is_visible = 0;
			idx_deleted.push_back(glm::vec3{int(detective_position[0]), int(detective_position[1]), int(detective_position[2])
		});
			std::cout << " ( x : " << int(detective_position[0]) << ", y : " << int(detective_position[1]) << ", z : " << int(detective_position[2]) << " ) cube is deleted" << "\n" << std::endl;
		}
		  glutPostRedisplay(); break;
		  //generate cube at detective cube position and add it's positom to vector
	case'+': if ((detective_position[0] < 10) && (detective_position[1] < 10) && (detective_position[2] < 10) && !check() && (cube[int(detective_position[0])][int(detective_position[1])][int(detective_position[2])].is_visible == 0))
	{
		cube[int(detective_position[0])][int(detective_position[1])][int(detective_position[2])].is_visible = 1;
		idx_deleted.push_back(glm::vec3{ int(detective_position[0]), int(detective_position[1]), int(detective_position[2])
			});
		std::cout << " ( x : " << int(detective_position[0]) << ", y : " << int(detective_position[1]) << ", z : " << int(detective_position[2]) << " ) cube is regenerationed" << "\n" << std::endl;
	}
			 glutPostRedisplay(); break;
		//undo 
	case'z':if (!idx_deleted.empty()) {
		cube[int(idx_deleted.back()[0])][int(idx_deleted.back()[1])][int(idx_deleted.back()[2])].is_visible = 1;
		std::cout << " ( x : " << idx_deleted.back()[0] << ", y : " << idx_deleted.back()[0] << ", z : " << idx_deleted.back()[0] << " ) cube is regenerationed" << "\n" << std::endl;
		idx_deleted.pop_back();
	}
			else {
		std::cout << "no more cube deleted!!" << "\n" << std::endl;
	}
		glutPostRedisplay(); break;
	case'j': detective_position[0] += 1.0f; glutPostRedisplay(); break; //x+1
	case'l': detective_position[0] -= 1.0f; glutPostRedisplay(); break;	//x-1
	case'y': detective_position[1] += 1.0f; glutPostRedisplay(); break;	//y+1
	case'h': detective_position[1] -= 1.0f; glutPostRedisplay(); break;	//y-1
	case'i': detective_position[2] += 1.0f; glutPostRedisplay(); break;	//z+1
	case'k': detective_position[2] -= 1.0f; glutPostRedisplay(); break;	//z-1
	};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Overloading bind_buffer functions : phong shading
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
	GLuint location = glGetAttribLocation(program, attri_name);
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
}

// Overloading bind_buffer functions : gouraud shading, flat shading
void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	GLuint location = glGetAttribLocation(program, attri_name);
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
}


///////////////////////////////////////////////////////////////////////////////////////////////
 //make cube-Initialize cube state
void make_cube() {
	for (int i = 0; i < 10; i++) {
		cube[0][0][i].is_true = false;
		for (int j = 0; j < 10; j++) {
			cube[0][j][i].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		cube[1][0][i].is_true = false;
		for (int j = 5; j < 10; j++) {
			cube[1][j][i].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		cube[1][0][i].is_true = false;
		for (int j = 5; j < 10; j++) {
			cube[1][j][i].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int k = 2; k < 10; k += 5) {
			for (int j = 8; j < 10; j++) {
				cube[k][j][i].is_true = false;
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 3; j < 7; j++) {
			cube[j][9][i].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		cube[8][0][i].is_true = false;
		for (int j = 6; j < 10; j++) {
			cube[8][j][i].is_true = false;
		}
	}


	for (int i = 0; i < 10; i++) {
		cube[9][0][i].is_true = false;
		cube[9][1][i].is_true = false;
		for (int j = 3; j < 10; j++) {
			cube[9][j][i].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 10; k += 9) {
			cube[i][0][k].is_true = false;
			cube[i][1][k].is_true = false;
			for (int j = 4; j < 10; j++) {
				cube[i][j][k].is_true = false;
			}
		}
	}
	for (int i = 0; i < 10; i++) {
		for (int k = 1; k < 10; k += 7) {
			cube[i][0][k].is_true = false;
			for (int j = 5; j < 10; j++) {
				cube[i][j][k].is_true = false;
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 2; j < 4; j++) {
			cube[i][9][j].is_true = false;
			cube[i][9][9 - j].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 6; j < 10; j++) {
			cube[i][j][4].is_true = false;
			cube[i][j][5].is_true = false;
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 10; k += 9) {
				cube[j][i][k].is_true = false;
				cube[9 - j][i][k].is_true = false;
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 1; k < 10; k += 7) {
				cube[j][i][k].is_true = false;
				cube[9 - j][i][k].is_true = false;
				cube[7][i][k].is_true = false;
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 10; k += 9) {
			for (int j = 2; j < 4; j++) {
				cube[k][i][j].is_true = false;
				cube[k][i][9 - j].is_true = false;
			}
		}
	}

	for (int i = 0; i < 10; i++) {
		for (int j = 4; j < 6; j++) {
			cube[0][i][j].is_true = false;
		}
	}


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization function
void init()
{
	make_cube();
	// Attach the menu
	
	glutAddMenuEntry("Orthogonalprojection", 0);
	glutAddMenuEntry("Perspective projection", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Shader informations
	ShaderInfo shaders[4][3] = {
		{ { GL_VERTEX_SHADER, "phong.vert" },{ GL_FRAGMENT_SHADER, "phong.frag" },{ GL_NONE, NULL } },
		{ { GL_VERTEX_SHADER, "gouraud.vert" },{ GL_FRAGMENT_SHADER, "gouraud.frag" },{ GL_NONE, NULL } },
		{ { GL_VERTEX_SHADER, "flat.vert" },{ GL_FRAGMENT_SHADER, "flat.frag" },{ GL_NONE, NULL } },
		{ { GL_VERTEX_SHADER, "box.vert" },{ GL_FRAGMENT_SHADER, "box.frag" },{ GL_NONE, NULL } }
	};

	// Load four shaders
	for (int i = 0; i < 4; ++i) {
		program[i] = LoadShaders(shaders[i]);
	}
		

	// Load obj & texture
	mstate[MODEL_RABBIT].orbit_radius = -1.5f;
	mstate[MODEL_RABBIT].up_down = 0.0f;

	//get box vertices % colors
	get_box_3d(box_vert);
	box_col.resize(box_vert.size());
	for (size_t i = 0; i < box_col.size(); ++i) { box_col[i] = 1; }
	glGenVertexArrays(1, &box_vao);
	glGenBuffers(2, box_vbo);
	glBindVertexArray(box_vao);
	bind_buffer(box_vbo[0], box_vert, program[3], "vPosition", 3);
	bind_buffer(box_vbo[1], box_col, program[3], "vColor", 3);



	for (size_t k = 0; k < NUM_OF_MODELS; ++k)
	{
		using namespace tinyobj;
		attrib_t attrib;

		is_obj_valid = load_obj(model_files[k], "models/", vertices[k], normals[k], vertex_map[k], attrib,
								shapes[k], materials[k], model_scales[k]);              // load obj data

		glActiveTexture(GL_TEXTURE0);                                                   // Active texture unit 0
		is_tex_valid = load_tex("models/", texcoords[k], texmap[k], attrib.texcoords, shapes[k], materials[k],
								 GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);     // load texture data

		colors[k].resize(vertices[k].size());
		for (size_t i = 0; i < colors[k].size(); ++i) { colors[k][i] = 1; }             // Initialize vertex colors

		glGenVertexArrays(3, vao[k]);
		glGenBuffers(4, vbo[k]);

		// Bind to VAO - phong shading
		glBindVertexArray(vao[k][0]);
		bind_buffer(vbo[k][0], vertices[k], program[0], "vPosition_ms", 3);    // Vertex coor. information
		bind_buffer(vbo[k][1], normals[k], program[0], "vNormal_ms", 3);       // Re-ordered normal information
		bind_buffer(vbo[k][2], colors[k], program[0], "vColor", 3);            // Vertex color information
		bind_buffer(vbo[k][3], texcoords[k], program[0], "vTexcoord", 2);      // Texture coor.
		
		// Bind to VAO - gouraud shading, flat shading
		for (int i = 1; i <= 2; ++i) {
			glBindVertexArray(vao[k][i]);
			bind_buffer(vbo[k][0], program[i], "vPosition_ms", 3);
			bind_buffer(vbo[k][1], program[i], "vNormal_ms", 3);
			bind_buffer(vbo[k][2], program[i], "vColor", 3);
			bind_buffer(vbo[k][3], program[i], "vTexcoord", 2);
		}
	}
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//d
void draw_obj_model(int model_idx, int color_mode, int object_code = 0)
{
	glUniform1i(UVARS("ColorMode"), color_mode);
	glUniform1i(UVARS("ObjectCode"), object_code);

	glBindVertexArray(vao[model_idx][shading_mode]);
	
	using namespace tinyobj;
	std::vector<shape_t>&			 _shapes = shapes[model_idx];
	std::vector<material_t>&		 _materials = materials[model_idx];
	std::vector<size_t>&			 _vertex_map = vertex_map[model_idx];
	std::map<std::string, size_t>&	 _texmap = texmap[model_idx];
	std::vector<real_t>&			 _vertices = vertices[model_idx];

	for (size_t i = 0; i < _shapes.size(); ++i)
	{
		// Set material properties for shape i
		int m_id = _shapes[i].mesh.material_ids[0];
		if (m_id < 0) { // If there are no material properties (no MTL file)
			glUniform1f(UVARS("n"), 10.0f);									 // Specular intensity factor - shininess
			glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f);						 // Ambient reflectivity
			glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f);						 // Diffuse reflectivity
			glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f);						 // Specular reflectivity
		}
		else {          // Using matrials array            
			glUniform1f(UVARS("n"), _materials[m_id].shininess);
			glUniform3fv(UVARS("Ka"), 1, _materials[m_id].ambient);
			glUniform3fv(UVARS("Kd"), 1, _materials[m_id].diffuse);
			glUniform3fv(UVARS("Ks"), 1, _materials[m_id].specular);

			auto texitem = _texmap.find(_materials[m_id].diffuse_texname);
			if (texitem != _texmap.end()) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texitem->second);
				glUniform1i(UVARS("sampler"), 0);
			}
		}

		glUniform3f(UVARS("Il"), 15.0f, 15.0f, 15.0f);					// Light source intensity
		glUniform3f(UVARS("Ia"), 1.0f, 1.0f, 1.0f);				 		// Ambient light intensity - the larger the value, the less the sense of depth
		glUniform3f(UVARS("c"), 0.2f, 0.2f, 0.2f);						// Coefficients for light source attenuation
		glUniform3f(UVARS("LightPos_cs"), 3.0f, 3.0f, 3.0f);

		
		using namespace glm;
		mat4 M = mstate[model_idx].get_transf();
		mat4 V = camera.get_transf();       UMATS("V", V);				// Viewing transformation 	
		mat4 P;                                                         // Projection transformation                                 
		      // Orthographic projection
	
		
		P = frustum(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 20.0f);         // Perspective projection
		
		UMATS("P", P);
		mat4 VM = V * M;                    UMATS("VM", VM);            // VM
		mat4 U = transpose(inverse(VM));    UMATS("U", U);              // Transformation of normal vectors
		

		// Draw an object
		glDrawArrays(GL_TRIANGLES, _vertex_map[i], _vertex_map[i + 1] - _vertex_map[i]);
		
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Render
void render(int color_mode) {
	using namespace glm;
	using namespace tinyobj;
	

	
		if (color_mode != 1) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}


		if (check()) {

			if (is_obj_valid) {
				
					glUseProgram(program[shading_mode]);
					location = glGetUniformLocation(program[shading_mode], "special");
					glUniform1i(location, 0);
					draw_obj_model(0, color_mode, 1);
			
			}
		}

		else{
	
			if (is_obj_valid&&(plane_mode==0)) {
				
					glUseProgram(program[shading_mode]);
					location = glGetUniformLocation(program[shading_mode], "special");
					glUniform1i(location, 1);
					draw_obj_model(1, color_mode, 2);
			
			}
		}
	}
	



///////////////////////////////////////////////////////////////////////////////////////////////////
// draw primitive
void transform_primitive(GLuint active_type,
	GLfloat xscale, GLfloat  yscale, GLfloat  zscale,
	GLfloat xtrans, GLfloat  ytrans, GLfloat  ztrans,
	GLfloat  theta, glm::mat4 &T2, GLfloat angle,
	GLint color = 0, GLint Line = 0,
	GLfloat rotate_x = 0.0f, GLfloat rotate_y = 0.0f, GLfloat rotate_z = 0.0f,
	GLfloat x_angle = 0.0f, GLfloat y_angle = 0.0f, GLfloat z_angle = 0.0f)
{

	using namespace glm;
	glUseProgram(program[3]);


	mat4 T1(1.0f);
	//T1 = rotate(T1, theta, vec3(0.0f, 1.0f, 0.0f));
	T1 = translate(T1, 0.25f*vec3(xtrans, ytrans, ztrans));

	if (rotate_x == 1.0f) {
		T1 = rotate(T1, angle, vec3(1.0f, 0.0f, 0.0f));
	}
	else if (rotate_y == 1.0f) {
		T1 = rotate(T1, angle, vec3(0.0f, 1.0f, 0.0f));
	}
	else if (rotate_z == 1.0f) {
		T1 = rotate(T1, angle, vec3(0.0f, 0.0f, 1.0f));
	}
	if (angle != 0) {
		T1 = rotate(T1, theta, vec3(0.0f, -1.0f, 0.0f));
	}

	T1 = scale(T1, 0.25f*vec3(xscale, yscale, zscale));
	location = glGetUniformLocation(program[3], "T");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T1));

	mat4 V = camera.get_transf();
	mat4 P;                                                         // Projection transformation                                 
	P = frustum(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 20.0f);         // Perspective projection

	glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(mat4(1.0f)));
	glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(P));
	glBindVertexArray(box_vao);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(3);
	glPointSize(2);
	int i = int(xtrans);
	int j = int(ytrans);
	int k = int(ztrans);


	if (active_type == 0) {
		glUniform1i(2, color);
		glUniform1i(7, i);     //red
		glUniform1i(8, j);	   //green
		glUniform1i(9, k);     //blue

		if (Line == 0) {
			glUniform1i(6, 1);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 1);
			glDrawArrays(GL_TRIANGLES, 0, box_vert.size());
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
		else {																			//detective box color
			if (color > 3) {									
				location = glGetUniformLocation(program[3], "MODE");
				glUniform1i(location, 0);
			}
			else {
				location = glGetUniformLocation(program[3], "MODE");
				glUniform1i(location, 1);
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_TRIANGLES, 0, box_vert.size());
		}
		
		location = glGetUniformLocation(program[3], "MODE");
		glUniform1i(location,1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glDrawArrays(GL_TRIANGLES, 0, box_vert.size());
		location = glGetUniformLocation(program[3], "MODE");
		glUniform1i(location, 0);

	}
	
}





///////////////////////////////////////////////////////////////////////////////////////////////////
// draw_cubes
void draw_cubes()
{	
	glBindVertexArray(box_vao);
	glUseProgram(program[3]);

	using namespace glm;
	
	mat4 T1(1.0);
	if (!check()) {
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				for (int k = 0; k < 10; k++)
				{
					if (flag_color == 1)
					{
						cube[i][j][k].color = vec3(10.0*double(i), 10.0*double(j), 10.0*double(k));
					}
					if (cube[i][j][k].is_visible == 1) {
						transform_primitive(0, 1.0f, 1.0f, 1.0f, 1.0f*i, 1.0f*j, 1.0f*k, 0, T1, 0.0f, 1, 0, 0, 0, 0);//box_top

					}

				}
			}
		}
		transform_primitive(0, 1.0f, 1.0f, 1.0f, 1.0f*detective_position[0], 1.0f*detective_position[1], 1.0f*detective_position[2], 0, T1, 0.0f, 1, 1, 0, 0, 0);//detective box


		flag_color = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Display
void display()
{
	glClearColor(224.0 / 255.0f, 234.0 / 255.0f, 154.0 / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	render(0);

	if (check()) {
		transform_primitive(0, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0, glm::mat4(1.0), 0.0f, 1, 0, 0, 0, 0);
		mstate[0].theta = 0.001f * clock();
		mstate[0].orbit_theta = 0.0001f * clock();

		mstate[MODEL_RABBIT].up_down += 0.0005f;
	}
	
	draw_cubes();
	glFlush();
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA); 
	glutInitWindowSize(1024, 1024); // (512, 512);
	glutCreateWindow("3D Rogic");
	GLenum err = glewInit();
	if (err != GLEW_OK) { fprintf(stderr, "Error: %s\n", glewGetErrorString(err));  exit(EXIT_FAILURE); }

	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);          // Register keyboard func.
	glutMotionFunc(motion);              // Register motion func.
	glutMouseFunc(mouse);                // Register mouse func.
	glutMouseWheelFunc(mouse_wheel);     // Register mouse_wheel func.
	glutJoystickFunc(gamepad, 100);
	glutMainLoop();                      // Keep running
}