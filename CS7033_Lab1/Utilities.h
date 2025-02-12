#pragma once 
#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include "Antons_maths_funcs.h"
#include <GL/freeglut.h>
#include "text.h"
#include <string>
#include <assert.h>
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "Defines.h"


// Assimp includes
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

//uncomment if needed
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

#define DEFAULT_VERT	"../Shaders/simpleVertexShader.txt"
#define DEFAULT_FRAG	"../Shaders/simpleFragmentShader.txt"

#define CUBEMAP_RIGHT	"-posx.png"
#define CUBEMAP_LEFT	"-negx.png"
#define CUBEMAP_TOP		"-posy.png"
#define CUBEMAP_BOT		"-negy.png"
#define CUBEMAP_FRONT	"-posz.png"
#define CUBEMAP_BACK	"-negz.png"

#define TEXTURE_FOLDER "../Textures/"

#pragma region FRAMEBUFFER

class Framebuffer {

public:
	GLuint framebuffer, tex;
	Framebuffer()
	{

	}
	void init(int width, int height, int index)
	{
		glGenFramebuffers(1, &framebuffer);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		GLuint depth_tex;
		glGenTextures(1, &depth_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depth_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach depth texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
		GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_bufs);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != status) {
			fprintf(stderr, "ERROR: incomplete framebuffer\n");
			if (GL_FRAMEBUFFER_UNDEFINED == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_UNDEFINED\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
			}
			else if (GL_FRAMEBUFFER_UNSUPPORTED == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_UNSUPPORTED\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
			}
			else if (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS == status) {
				fprintf(stderr, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
			}
			else {
				fprintf(stderr, "unspecified error\n");
			}
			exit(-1);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, index);
	}
};

#pragma endregion

#pragma region MESH
/**
Please Note: Objects with multiple meshes don't work in this. Believe me, I've tried, and failed, and failed again. So make your object all one mesh.
Requirements:
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your Init Function:
    torch_mesh.init(shaderProgramID, TORCH_MESH, torch_file);
    TORCH_MESH is defined at the start of main.cpp as the location of the object. torch_file is a char* holding the location of the texture file.
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your Display() Function:
    torch_mesh.activate();
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

Many parts of this region were created alongside the learnopengl.com tutorials
**/

class Mesh{

	bool update_mesh();
	vector<GLfloat> newpoints; // array of vertex points
	vector<GLfloat> newnormals; // array of vertex normals
	mat4 orientation = identity_mat4();


public:
	Mesh();

	//loading and binding vao and tex
	bool load_mesh(const char* file_name);
	bool load_texture(const char* file_name, GLuint* tex);

	//whats called from main
	void init(const char* mesh_file, const char* tex_file, const char* normal_file);
	void initCubeMap(GLfloat vertices[], int num_vertices, string texture);
	void loadCubeFace(GLuint textureID, GLenum side, const char* filename);

	GLuint VAO[20], tex, norm;
	int mesh_vertex_count;

	void moveObject(vec3 linVel, vec3 angVel, float delta);
	mat4 star(vec3& a);
	vec3 position;
};

Mesh::Mesh(){}

void Mesh::init(const char* mesh_file, const char* tex_file = NULL, const char* normal_file = NULL){

	if (!load_mesh(mesh_file))
		exit(1);

	if (tex_file != NULL)
	{
		load_texture(tex_file, &tex);
		glUniform1i(tex, 0);
	}

	if(normal_file != NULL)
	{
		load_texture(normal_file, &norm);
		glUniform1i(norm, 1);
	}

}

void Mesh::initCubeMap(GLfloat vertices[], int num_vertices, string texture)
{
	VAO[0] = NULL;
	GLuint VBO;
	mesh_vertex_count = num_vertices;
	glGenVertexArrays(1, &VAO[0]);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * num_vertices, vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO[0]);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);	//Creating a Texture Buffer

	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_X, (TEXTURE_FOLDER + texture + CUBEMAP_RIGHT).c_str());
	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, (TEXTURE_FOLDER + texture + CUBEMAP_LEFT).c_str());
	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, (TEXTURE_FOLDER + texture + CUBEMAP_TOP).c_str());
	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, (TEXTURE_FOLDER + texture + CUBEMAP_BOT).c_str());
	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, (TEXTURE_FOLDER + texture + CUBEMAP_FRONT).c_str());
	loadCubeFace(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, (TEXTURE_FOLDER + texture + CUBEMAP_BACK).c_str());

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//Changing the many options for textures to suit this situation
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Mesh::loadCubeFace(GLuint textureID, GLenum side, const char* filename)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	int w;
	int h;
	int comp;
	unsigned char* image = stbi_load(filename, &w, &h, &comp, STBI_rgb);
	std::string name = filename;
	if (image == NULL)
	{
		cout << "Failed to load image" << endl;
		return;
	}
	//else
		//cout << name.substr(name.find_last_of('/') + 1, name.length()) << " [width, height]: [" << w << ", " << h << "]" << endl;
	if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0)
	{
		fprintf(stderr, "WARNING: image %s is not power-of-2 dimensions\n", filename);
	}

	glTexImage2D(side, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
}

bool Mesh::load_mesh(const char* file_name) 
{
	string name = file_name;
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	if (!scene) 
	{
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return false;
	}
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		//printf("    %i vertices in mesh[%i]\n", mesh->mNumVertices, i);

		/* pass back number of vertex points in mesh */
		mesh_vertex_count = mesh->mNumVertices;

		/* generate a VAO, using the pass-by-reference parameter that we give to the
		function */
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);

		/* we really need to copy out all the data from AssImp's funny little data
		structures into pure contiguous arrays before we copy it into data buffers
		because assimp's texture coordinates are not really contiguous in memory.
		i allocate some dynamic memory to do this. */
		GLfloat* points = NULL; // array of vertex points
		GLfloat* normals = NULL; // array of vertex normals
		GLfloat* texcoords = NULL; // array of texture coordinates
		GLfloat* tangents = NULL;
		if (mesh->HasPositions())
		{
			points = (GLfloat*)malloc(mesh_vertex_count * 3 * sizeof(GLfloat));
			for (int i = 0; i < mesh_vertex_count; i++) 
			{
				const aiVector3D* vp = &(mesh->mVertices[i]);
				points[i * 3] = (GLfloat)vp->x;
				points[i * 3 + 1] = (GLfloat)vp->y;
				points[i * 3 + 2] = (GLfloat)vp->z;
				newpoints.push_back(points[i * 3]);
				newpoints.push_back(points[i * 3 + 1]);
				newpoints.push_back(points[i * 3 + 2]);
			}
		}
		if (mesh->HasNormals())
		{
			normals = (GLfloat*)malloc(mesh_vertex_count * 3 * sizeof(GLfloat));
			for (int i = 0; i < mesh_vertex_count; i++)
			{
				const aiVector3D* vn = &(mesh->mNormals[i]);
				normals[i * 3] = (GLfloat)vn->x;
				normals[i * 3 + 1] = (GLfloat)vn->y;
				normals[i * 3 + 2] = (GLfloat)vn->z;
				newnormals.push_back(normals[i * 3]);
				newnormals.push_back(normals[i * 3 + 1]);
				newnormals.push_back(normals[i * 3 + 2]);
			}
		}
		if (mesh->HasTextureCoords(0))
		{
			//cout << "There are texture vertexes for mesh " << name.substr(name.find_last_of('/') + 1, name.length()) << endl;
			texcoords = (GLfloat*)malloc(mesh_vertex_count * 2 * sizeof(GLfloat));
			for (int i = 0; i < mesh_vertex_count; i++)
			{
				const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
				texcoords[i * 2] = (GLfloat)vt->x;
				texcoords[i * 2 + 1] = (GLfloat)vt->y;
				//cout << "[" << texcoords[i * 2] << "," << texcoords[i * 2 + 1] << "]" << endl;
			}
		}

		if (mesh->HasTangentsAndBitangents()) {
			//cout << "There are tangent vertexes for mesh " << name.substr(name.find_last_of('/') + 1, name.length()) << endl;
			tangents = (GLfloat*)malloc(mesh_vertex_count * 4 * sizeof(GLfloat));
			for (int i = 0; i < mesh_vertex_count; i++)
			{
				const aiVector3D* tangent = &(mesh->mTangents[i]);
				const aiVector3D* bitangent = &(mesh->mBitangents[i]);
				const aiVector3D* normal = &(mesh->mNormals[i]);

				// put the three vectors into my vec3 struct format for doing maths
				vec3 t(tangent->x, tangent->y, tangent->z);
				vec3 n(normal->x, normal->y, normal->z);
				vec3 b(bitangent->x, bitangent->y, bitangent->z);
				// orthogonalise and normalise the tangent so we can use it in something
				// approximating a T,N,B inverse matrix
				vec3 t_i = normalise(t - n * dot(n, t));

				// get determinant of T,B,N 3x3 matrix by dot*cross method
				float det = (dot(cross(n, t), b));
				if (det < 0.0f) {
					det = -1.0f;
				}
				else {
					det = 1.0f;
				}

				// push back 4d vector for inverse tangent with determinant
				tangents[i * 4] = t_i.v[0];
				tangents[i * 4 + 1] = t_i.v[1];
				tangents[i * 4 + 2] = t_i.v[2];
				tangents[i * 4 + 3] = det;
			}
		}
		//else
			//cout << "There are no tangent vertexes for mesh " << name.substr(name.find_last_of('/') + 1, name.length()) << endl;

		/* copy mesh data into VBOs */
		if (mesh->HasPositions())
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh_vertex_count * 3 * sizeof(GLfloat), points, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
			free(points);
		}
		if (mesh->HasNormals())
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 3 * mesh_vertex_count * sizeof(GLfloat), normals, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(1);
			free(normals);
		}
		if (mesh->HasTextureCoords(0))
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 2 * mesh_vertex_count * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(2);
			free(texcoords);
		}
		if (mesh->HasTangentsAndBitangents())
		{
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 4 * mesh_vertex_count * sizeof(GLfloat), tangents, GL_STATIC_DRAW);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(3);
			free(tangents);
		}
		
	}
	aiReleaseImport(scene);
	return true;
}

bool Mesh::load_texture(const char* file_name, GLuint* tex) 
{
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
	if (!image_data) 
	{
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) 
	{
		fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name);
	}
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) 
	{
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) 
		{
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures(1, tex);
	//glActiveTexture(mode);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,	x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	return true;
}

void Mesh::moveObject(vec3 linVel, vec3 angVel, float delta)
{
	mat4 rDot = star(angVel*delta)*orientation;
	orientation.m[0] += rDot.m[0];
	orientation.m[1] += rDot.m[1];
	orientation.m[2] += rDot.m[2];
	orientation.m[3] = 0;

	orientation.m[4] += rDot.m[4];
	orientation.m[5] += rDot.m[5];
	orientation.m[6] += rDot.m[6];
	orientation.m[7] = 0;

	orientation.m[8] += rDot.m[8];
	orientation.m[9] += rDot.m[9];
	orientation.m[10] += rDot.m[10];
	orientation.m[11] = 0;

	orientation.m[12] += rDot.m[12];
	orientation.m[13] += rDot.m[13];
	orientation.m[14] += rDot.m[14];
	orientation.m[15] = 1;

	//Orthonormalisation
	vec3 Cx = vec3(orientation.m[0], orientation.m[1], orientation.m[2]) / length(vec3(orientation.m[0], orientation.m[1], orientation.m[2]));
	vec3 Cz = vec3(orientation.m[8], orientation.m[9], orientation.m[10]) / length(vec3(orientation.m[8], orientation.m[9], orientation.m[10]));
	vec3 Cy = cross(Cz, Cx);
	Cy = Cy / length(Cy);
	Cz = cross(Cx, Cy);
	Cz = Cz / length(Cz);
	orientation.m[0] = Cx.v[0];
	orientation.m[1] = Cx.v[1];
	orientation.m[2] = Cx.v[2];

	orientation.m[4] = Cy.v[0];
	orientation.m[5] = Cy.v[1];
	orientation.m[6] = Cy.v[2];

	orientation.m[8] = Cz.v[0];
	orientation.m[9] = Cz.v[1];
	orientation.m[10] = Cz.v[2];

	position += linVel * delta;

	update_mesh();
}

bool Mesh::update_mesh()
{
	for (int i = 0; i < mesh_vertex_count; i++)
	{
		static vector<GLfloat> initPoints = newpoints;
		vec3 vertice = vec3(initPoints[i * 3], initPoints[i * 3 + 1], initPoints[i * 3 + 2]);
		vertice = multiply(orientation, vertice) + position;
		newpoints[i * 3] = vertice.v[0];
		newpoints[i * 3 + 1] = vertice.v[1];
		newpoints[i * 3 + 2] = vertice.v[2];

		static vector<GLfloat> initNormals = newnormals;
		vertice = vec3(initNormals[i * 3], initNormals[i * 3 + 1], initNormals[i * 3 + 2]);
		vertice = multiply(orientation, vertice) + position;
		newnormals[i * 3] = vertice.v[0];
		newnormals[i * 3 + 1] = vertice.v[1];
		newnormals[i * 3 + 2] = vertice.v[2];
	}



	glBindVertexArray(VAO[0]);
	/* copy mesh data into VBOs */
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_vertex_count * 3 * sizeof(GLfloat), newpoints.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	GLuint vbo2;
	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh_vertex_count * sizeof(GLfloat), newnormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	return true;
}

mat4 Mesh::star(vec3& a)
{
	mat4 star = mat4(
		0, -a.v[2], a.v[1], 0,
		a.v[2], 0, -a.v[0], 0,
		-a.v[1], a.v[0], 0, 0,
		0, 0, 0, 0
	);
	return transpose(star);	//converting matrix into Anton's poxy way of doing things.
}

#pragma endregion

#pragma region EULER_CAMERA
/**
Please Note: This Camera Implementation does not currently Support Movement on the Roll Axis.

Requirements:
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your Init() Function:
    cam.setSensitivity(<insert sensitivity value here>);
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your Input Functions:
    pitCam, yawCam, rolCam - These become 1 to move in the positive direction, and -1 to move in the negative direction. The reset to 0 during the function call.
    I recommend that they are changed via the mouse input option, in some shape or form. Set rolCam to 0 and never touch it, I didn't finish it yet.

	frontCam, sideCam - These two become either 1 to go in the positive axis direction, 0 to not move, and -1 to go in the negative axis direction.
    I recommend that they are changed via the glutKeyboardFunc and glutKeyboardUpFunc, or the GLFW equivalent.
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your UpdateScene() Function:
    Call to cam.movForward(frontCam), cam.movRight(sideCam), and cam.changeFront(pitCam, yawCam, rolCam);. That's it.
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your display() Function:
    view = look_at(cam.getPosition(), cam.getPosition() + cam.getFront(), cam.getUp());
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
FAQ
    Camera doesn't move!
       Did you remember to use setSensitivity()?
**/

class EulerCamera{

public:
	EulerCamera(vec3 pos, GLfloat y, GLfloat p, GLfloat r);
	void setSensitivity(GLfloat value);
	inline void changeFront(GLfloat pi, GLfloat ya, GLfloat ro);
	inline mat4 getFreeView();
	void movForward(GLfloat value);
	void movRight(GLfloat value);
	void setPosition(vec3 value);
	void setFront(vec3 value, GLfloat y, GLfloat p);
	vec3 getPosition();
	vec3 getFront();
	vec3 getUp();
	void move(GLfloat value);
	void jump(bool& jumping);
	GLuint cam_buffer;

	GLfloat yaw, pitch, roll;
private:
	vec3 position, front, up, right;
	GLfloat sensitivity, degrees;

	void changeFront();

	GLfloat initYaw, initPitch, initRoll;
};

EulerCamera::EulerCamera(vec3 pos, GLfloat y, GLfloat p, GLfloat r)
{
	position = pos;
	front = vec3(0.0, 0.0, 0.0);
	right = vec3(0.0, 0.0, 0.0);
	up = vec3(0.0, 1.0, 0.0);
	changeFront();
	yaw = y;
	pitch = p;
	roll = r;
	degrees = 0;
}
inline void EulerCamera::changeFront(GLfloat pi, GLfloat ya, GLfloat ro){

	pi *= sensitivity;
	ya *= sensitivity;
	ro *= sensitivity;

	pitch += (GLfloat)pi;
	yaw += (GLfloat)ya;
	roll += (GLfloat)ro;
	if (pitch > 359)
		pitch = 0;
	if (pitch < 0)
		pitch = 359;
	if (yaw > 359)
		yaw = 0;
	else if (yaw < 0)
		yaw = 359;
	if (roll > 359)
		roll = 0;
	else if (roll < 0)
		roll = 359;

	changeFront();

}

inline mat4 EulerCamera::getFreeView()
{
	mat4 view = translate(identity_mat4(), vec3(0.0, 0.0, -10.0));
	view = rotate_z_deg(view, roll);
	view = rotate_x_deg(view, -pitch);
	view = rotate_y_deg(view, yaw);
	return view;
}

inline void EulerCamera::changeFront()
{
	GLfloat rpitch = pitch * GLfloat(ONE_DEG_IN_RAD);
	GLfloat ryaw = yaw  * GLfloat(ONE_DEG_IN_RAD);
	GLfloat rroll = roll * GLfloat(ONE_DEG_IN_RAD);
	GLfloat ninety = 90 * GLfloat(ONE_DEG_IN_RAD);

	vec3 f = vec3();
	f.v[0] = cos(ryaw) * cos(rpitch);
	f.v[1] = sin(rpitch);
	f.v[2] = sin(ryaw) * cos(rpitch);

	front = normalise(f);


	right = normalise(cross(front, up));
	up = normalise(cross(right, front));
}
void EulerCamera::movForward(GLfloat value){ position += front * value / 10.0f;}
void EulerCamera::movRight(GLfloat value){ position += normalise(cross(front, up))*(value / 10.0f); }
void EulerCamera::move(GLfloat value){ position += vec3(front.v[0] * value / 10.0f, 0.0f, front.v[2] * value / 10.0f); }
void EulerCamera::setPosition(vec3 value){ position = value; }
vec3 EulerCamera::getPosition(){ return position; }
vec3 EulerCamera::getFront(){ return front; }
void EulerCamera::setFront(vec3 value, GLfloat y, GLfloat p){ 
	front = normalise(value);
	yaw = y;
	pitch = p;
}
vec3 EulerCamera::getUp(){ return up; }
void EulerCamera::setSensitivity(GLfloat value){ sensitivity = value; }
void EulerCamera::jump(bool& jumping){
	if (jumping)
	{
		position += vec3(0.0f, 0.3f*cos(degrees * float(ONE_DEG_IN_RAD)), 0.0f);
		front += vec3(0.0f, 0.3f*cos(degrees * float(ONE_DEG_IN_RAD)), 0.0f);
		if (degrees >= 180)
		{
			jumping = false;
			degrees = 0;
		}
		degrees += 5.0f;
	}
	else
		return;
}
#pragma endregion

#pragma region QUATERNION_CAMERA
class QuatCam {
public:
	GLfloat heading, panSpeed = 100.0f, movSpeed = 5.0f;
	vec4 up = vec4(0.0, 1.0, 0.0, 0.0), right = vec4(1.0, 0.0, 0.0, 0.0), front = vec4(0.0, 0.0, -1.0, 0.0);
	versor rotation;
	mat4 T;
	mat4 R;
	mat4 viewMat;
	vec3 cam_pos;
	GLfloat yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
	vec3 initPos;

	QuatCam(vec4 u, GLfloat heading, vec4 position)
	{
		cam_pos = position;
		initPos = position;
		T = translate(identity_mat4(), vec3(-position.v[0], -position.v[1], -position.v[2]));
		rotation = quat_from_axis_deg(-heading, front.v[0], front.v[1], front.v[2]);
		R = quat_to_mat4(rotation);
		viewMat = R * T;
	}

	void reset()
	{
		up = vec4(0.0, 1.0, 0.0, 0.0);
		right = vec4(1.0, 0.0, 0.0, 0.0);
		front = vec4(0.0, 0.0, -1.0, 0.0);
		cam_pos = initPos;
		T = translate(identity_mat4(), vec3(-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2]));
		rotation = quat_from_axis_deg(-0.0, front.v[0], front.v[1], front.v[2]);
		R = quat_to_mat4(rotation);
		viewMat = R * T;
		yaw = 0.0f;
		pitch = 0.0f;
		roll = 0.0f;
	}

	vec3 getUp()
	{
		return vec3(up.v[0], up.v[1], up.v[2]);
	}

	vec3 getFront()
	{
		return vec3(front.v[0], front.v[1], front.v[2]);
	}

	vec3 getPosition()
	{
		return cam_pos;
	}

	vec3 move(GLfloat _yaw, GLfloat _pitch, GLfloat _roll, GLfloat frontCam, GLfloat sideCam)
	{
		yaw += _yaw;
		pitch += _pitch;
		roll += _roll;

		versor q = quat_from_axis_deg(yaw, up.v[0], up.v[1], up.v[2]);
		q = quat_from_axis_deg(-pitch, right.v[0], right.v[1], right.v[2]) * q;
		q = quat_from_axis_deg(roll, front.v[0], front.v[1], front.v[2]) * q;

		heading += _yaw;

		R = quat_to_mat4(normalise(q));

		front = R * vec4(0.0, 0.0, -1.0, 0.0);
		right = R * vec4(1.0, 0.0, 0.0, 0.0);
		up = R * vec4(0.0, 1.0, 0.0, 0.0);

		cam_pos = cam_pos + vec3(front) * frontCam;
		cam_pos = cam_pos + vec3(right) * sideCam;
		T = translate(identity_mat4(), vec3(cam_pos));

		viewMat = (R) * (T);

		//Orthonormalisation
		vec3 Cx = vec3(viewMat.m[0], viewMat.m[1], viewMat.m[2]) / length(vec3(viewMat.m[0], viewMat.m[1], viewMat.m[2]));
		vec3 Cz = vec3(viewMat.m[8], viewMat.m[9], viewMat.m[10]);
		vec3 Cy = cross(Cz, Cx);
		Cy = Cy / length(Cy);
		Cz = cross(Cx, Cy);
		Cz = Cz / length(Cz);
		viewMat.m[0] = Cx.v[0];
		viewMat.m[1] = Cx.v[1];
		viewMat.m[2] = Cx.v[2];

		viewMat.m[4] = Cy.v[0];
		viewMat.m[5] = Cy.v[1];
		viewMat.m[6] = Cy.v[2];

		viewMat.m[8] = Cz.v[0];
		viewMat.m[9] = Cz.v[1];
		viewMat.m[10] = Cz.v[2];
	}
};
#pragma endregion

#pragma region SHADER
/**
Requirements:
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
In your Init() Function:
    shaderProgramID = basic.CompileShader(NULL, NULL);
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
Then do all your regular mumbo jumbo inside your display file (linking uniforms and such)
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
I've given two options for the shaders. You can either give your own, and put the path in the correct variable, or leave it NULL, and take the default one.


This class was originally created by following the tutorial steps outlined on learnopengl.com
**/
class Shader{
	
public:
	GLuint CompileShader(char* vertex, char* fragment);

private:
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	char*readShaderSource(const char* shaderFile);

};
// Create a NULL-terminated string by reading the provided file
char* Shader::readShaderSource(const char* shaderFile) {

	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");
	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint Shader::CompileShader(char* vertex, char* fragment){

	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	if (vertex)
		AddShader(shaderProgramID, vertex, GL_VERTEX_SHADER);
	else
	{
		cout << "No Vertex Shader found" << endl;
		AddShader(shaderProgramID, DEFAULT_VERT, GL_VERTEX_SHADER);
	}
	if (fragment)
		AddShader(shaderProgramID, fragment, GL_FRAGMENT_SHADER);
	else
	{
		cout << "No Fragment Shader found" << endl;
		AddShader(shaderProgramID, DEFAULT_FRAG, GL_FRAGMENT_SHADER);
	}

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;

}
#pragma endregion

#pragma region EXTRA FUNCTIONS

/*----------------------------------------------------------------------------
								DEFINITIONS
----------------------------------------------------------------------------*/

void drawObject(GLuint shaderID, mat4 view, mat4 proj, mat4 model, vec3 light, vec3 Ls, vec3 La, vec3 Ld, vec3 Ks, vec3 Ka, vec3 Kd, float exp, EulerCamera cam, Mesh mesh, float coneAngle, vec3 coneDirection, GLenum mode);
void drawCubeMap(GLuint shaderID, GLuint textureID, mat4 view, mat4 proj, mat4 model, vec3 Ld, vec3 La, EulerCamera cam, Mesh skybox, GLenum mode);

/*----------------------------------------------------------------------------
								IMPLEMENTATIONS
----------------------------------------------------------------------------*/

void drawObject(GLuint shaderID, mat4 view, mat4 proj, mat4 model, vec3 light, vec3 Ls, vec3 La, vec3 Ld, vec3 Ks, vec3 Ka, vec3 Kd, float exp, EulerCamera cam, Mesh mesh, float coneAngle, vec3 coneDirection, GLenum mode)
{

	//for (int i = 0; i < 6; i++)
	//{
		glUseProgram(shaderID);
		glBindVertexArray(mesh.VAO[0]);
		glUniform3fv(glGetUniformLocation(shaderID, "Ls"), 1, Ls.v);
		glUniform3fv(glGetUniformLocation(shaderID, "Ld"), 1, Ld.v);
		glUniform3fv(glGetUniformLocation(shaderID, "La"), 1, La.v);
		glUniform3fv(glGetUniformLocation(shaderID, "Ks"), 1, Ks.v);
		glUniform3fv(glGetUniformLocation(shaderID, "Kd"), 1, Kd.v);
		glUniform3fv(glGetUniformLocation(shaderID, "Ka"), 1, Ka.v);
		glUniform1f(glGetUniformLocation(shaderID, "specular_exponent"), exp);
		glUniform3fv(glGetUniformLocation(shaderID, "light"), 1, light.v);
		glUniform4fv(glGetUniformLocation(shaderID, "camPos"), 1, cam.getPosition().v);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, view.m);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "proj"), 1, GL_FALSE, proj.m);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, model.m);

		if (coneAngle != NULL)
		{
			glUniform3fv(glGetUniformLocation(shaderID, "coneDirection"), 1, coneDirection.v);
			glUniform1f(glGetUniformLocation(shaderID, "coneAngle"), coneAngle);
		}

		if (mesh.tex)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.tex);
			glUniform1i(glGetUniformLocation(shaderID, "texture_primary"), 0);

		}
		if (mesh.norm)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh.norm);
			glUniform1i(glGetUniformLocation(shaderID, "normal_map"), 1);

		}

		glDrawArrays(mode, 0, mesh.mesh_vertex_count);
	//}
}

void drawCubeMap(GLuint shaderID, GLuint textureID, mat4 view, mat4 proj, mat4 model, vec3 Ld, vec3 La, EulerCamera cam, Mesh skybox, GLenum mode) 
{
	glDepthMask(GL_FALSE);
	glUseProgram(shaderID);
	glBindVertexArray(skybox.VAO[0]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "cube_texture"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "proj"), 1, GL_FALSE, proj.m);
	view.m[12] = 0;
	view.m[13] = 0;
	view.m[14] = 0;
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, view.m);
	glDrawArrays(mode, 0, skybox.mesh_vertex_count);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}


#pragma endregion