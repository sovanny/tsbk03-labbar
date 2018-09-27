// Bump mapping lab by Ingemar
// Revised 2013 to use MicroGlut, VectorUtils3 and zpr

// gcc lab1-2.c ../common/*.c -lGL -o lab1-2 -I../common
// eller snarare (nyare system):
// gcc -Wall -o lab1-1 -DGL_GLEXT_PROTOTYPES lab1-1.c ../common/*.c  ../common/Linux/MicroGlut.c -I../common -I../common/Linux -lXt -lX11 -lm -lGL

// 2018: No zpr, trackball code added in the main program.

#ifdef __APPLE__
// Mac
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// uses framework Cocoa
#else
	#ifdef WIN32
// MS
		#include <windows.h>
		#include <stdio.h>
		#include <GL/glew.h>
		#include <GL/glut.h>
	#else
// Linux
		#include <stdio.h>
		#include <GL/gl.h>
		#include "MicroGlut.h"
//		#include <GL/glut.h>
	#endif
#endif

#include "LoadTGA.h"
#include "VectorUtils3.h"
#include "GL_utilities.h"
#include "loadobj.h"

// initial width and heights
#define W 512
#define H 512

#define NEAR 1.0
#define FAR 150.0
#define RIGHT 0.5
#define LEFT -0.5
#define TOP 0.5
#define BOTTOM -0.5

#define NUM_LIGHTS 4

void onTimer(int value);


mat4 projectionMatrix,
        viewMatrix, modelToWorldMatrix; // modelToWorldMatrix controlled by trackball code

// The cube has 24 vertices. We pass Vs and Vt by vertex - 4 times per quad
GLfloat Vs[24][3] = {
							// 1-4
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							// 5-8
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							// 5-1
							{0.0,-1.0,0.0},
							{0.0,-1.0,0.0},
							{0.0,-1.0,0.0},
							{0.0,-1.0,0.0},
							// 2-3
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							// 8-4
							{0.0,-1.0,0.0}, // ??
							{0.0,-1.0,0.0},
							{0.0,-1.0,0.0},
							{0.0,-1.0,0.0},
							// 1-4
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},
							{-1.0,0.0,0.0},

							};
GLfloat Vt[24][3] = {
							// 3-4
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							// 7-8
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							// 2-1
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							// 7-3
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},
							// 3-4
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							{0.0,0.0,1.0},
							// 8-4
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},
							{0.0,1.0,0.0},

							};

//----------------------Globals-------------------------------------------------
Model *cube;
GLuint shader = 0;
GLuint bumpTex;
unsigned int vsBuffer, vtBuffer; // Attribute buffers for Vs and Vt

//-------------------------------------------------------------------------------------

void init(void)
{
	dumpInfo();  // shader info

	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Load shader
  shader = loadShaders("lab1-2.vert", "lab1-2.frag");

  // Load bump map (you are encouraged to try different ones)
  LoadTGATextureSimple("bumpmaps/uppochner.tga", &bumpTex);

	// load the model
  cube = LoadModelPlus("cubeexp.obj");
  printf("%d vertices\n", cube->numVertices);
  printf("%d indices\n", cube->numIndices);

  vec3 cam = SetVector(3, 2, 3);
  vec3 point = SetVector(0, 0, 0);
	vec3 up = {0, 1, 0};
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	// Upload Vs and Vt arrays to VBOs
	glBindVertexArray(cube->vao);
	glGenBuffers(1, &vsBuffer);
	glGenBuffers(1, &vtBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vsBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(GLfloat), Vs, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader, "Vs"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(shader, "Vs"));

	glBindBuffer(GL_ARRAY_BUFFER, vtBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24*3*sizeof(GLfloat), Vt, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader, "Vt"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(glGetAttribLocation(shader, "Vt"));
}

//-------------------------------callback functions------------------------------------------

void display(void)
{
    // This function is called whenever it is time to render
    //  a new frame; due to the onTimer()-function below, this
    //  function will get called several times per second

    // Clear framebuffer & zbuffer
	glClearColor(0.1, 0.1, 0.3, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 vm2 = Mult(viewMatrix, modelToWorldMatrix);

    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_TRUE, vm2.m);
    glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);

    DrawModel(cube, shader, "in_Position", "in_Normal", "in_TexCoord");

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat) w / (GLfloat) h;
    projectionMatrix = perspective(70, ratio, 0.2, 1000.0);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

void onTimer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(5, &onTimer, value);
}

// Trackball

int prevx = 0, prevy = 0;

void mouseUpDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		prevx = x;
		prevy = y;
	}
}

void mouseDragged(int x, int y)
{
	vec3 p;
	mat4 m;

	// This is a simple and IMHO really nice trackball system:

	// Use the movement direction to create an orthogonal rotation axis

	p.y = x - prevx;
	p.x = -(prevy - y);
	p.z = 0;

	// Create a rotation around this axis and premultiply it on the model-to-world matrix
	// Limited to fixed camera! Will be wrong if the camera is moved!

	m = ArbRotate(p, sqrt(p.x*p.x + p.y*p.y) / 50.0); // Rotation in view coordinates
	modelToWorldMatrix = Mult(m, modelToWorldMatrix);

	prevx = x;
	prevy = y;

	glutPostRedisplay();
}

//-----------------------------main-----------------------------------------------
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitContextVersion(3, 2); // Might not be needed in Linux
    glutInitWindowSize(W, H);
    glutCreateWindow ("bump mapping lab");
    glutDisplayFunc(display);

    glutTimerFunc(5, &onTimer, 0);
    glutReshapeFunc(reshape);
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);

    init();
//    zprInit(&viewMatrix, cam, point);

    glutMainLoop();
    exit(0);
}
