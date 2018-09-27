// New version by Ingemar 2010
// Removed all dependencies of the Wild Magic (wml) library.
// Replaced it with VectorUtils2 (in source)
// Replaced old shader module with the simpler "ShaderUtils" unit.

// 2013: Adapted to VectorUtils3 and MicroGlut.

// gcc skinning.c ../common/*.c -lGL -o skinning -I../common
// not working any more. This is untested but closer to the truth:
// gcc skinning.c -o skinning ../common/*.c ../common/Linux/MicroGlut.c -I../common -I../common/Linux -DGL_GLEXT_PROTOTYPES -lXt -lX11 -lGL -lm

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifdef __APPLE__
// Mac
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
//uses framework Cocoa
#else
	#ifdef WIN32
// MS
		#include <stdio.h>
		#include <GL/glew.h>
		#include <GL/glut.h>
	#else
// Linux
		#include <GL/gl.h>
		#include "MicroGlut.h" // #include <GL/glut.h>
	#endif
#endif

#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"

// Ref till shader
GLuint g_shader;

typedef struct Triangle
{
	GLuint				v1;
	GLuint				v2;
	GLuint				v3;
} Triangle;

#define kMaxRow 10
#define kMaxCorners 8
#define kMaxg_poly ((kMaxRow-1) * kMaxCorners * 2)
#ifndef Pi
#define Pi 3.1416
#endif
#ifndef true
#define true 1
#endif


Triangle g_poly[kMaxg_poly];

// vertices
Point3D g_vertsOrg[kMaxRow][kMaxCorners];
Point3D g_normalsOrg[kMaxRow][kMaxCorners];

// vertices sent to OpenGL
Point3D g_vertsRes[kMaxRow][kMaxCorners];
Point3D g_normalsRes[kMaxRow][kMaxCorners];

// vertex attributes sent to OpenGL
Point3D g_boneWeights[kMaxRow][kMaxCorners];

float weight[kMaxRow] = {0.0, 0.0, 0.0, 0.15, 0.35, 0.65, 0.85, 1.0, 1.0, 1.0};

Model *cylinderModel; // Collects all the above for drawing with glDrawElements

mat4 modelViewMatrix, projectionMatrix;

///////////////////////////////////////////////////
//		B U I L D	C Y L I N D E R
// Desc:	bygger upp cylindern
//
void BuildCylinder()
{
	long	row, corner, cornerIndex;
	float g_vertstex[kMaxRow][kMaxCorners][2];

	// s�tter v�rden till alla vertexar i meshen
	for (row = 0; row < kMaxRow; row++)
		for (corner = 0; corner < kMaxCorners; corner++)
		{
			g_vertsOrg[row][corner].x = row;
			g_vertsOrg[row][corner].y = cos(corner * 2*Pi / kMaxCorners);
			g_vertsOrg[row][corner].z = sin(corner * 2*Pi / kMaxCorners);

			g_normalsOrg[row][corner].x = 0;
			g_normalsOrg[row][corner].y = cos(corner * 2*Pi / kMaxCorners);
			g_normalsOrg[row][corner].z = sin(corner * 2*Pi / kMaxCorners);

			g_boneWeights[row][corner].x = (1-weight[row]);
			g_boneWeights[row][corner].y = weight[row];
			g_boneWeights[row][corner].z = 0.0;
		};

	// g_poly definerar mellan vilka vertexar som
	// trianglarna ska ritas
	for (row = 0; row < kMaxRow-1; row++)
		for (corner = 0; corner < kMaxCorners; corner++)
		{
	// Quads built from two triangles

			if (corner < kMaxCorners-1)
			{
				cornerIndex = row * kMaxCorners + corner;
				g_poly[cornerIndex * 2].v1 = cornerIndex;
				g_poly[cornerIndex * 2].v2 = cornerIndex + 1;
				g_poly[cornerIndex * 2].v3 = cornerIndex + kMaxCorners + 1;

				g_poly[cornerIndex * 2 + 1].v1 = cornerIndex;
				g_poly[cornerIndex * 2 + 1].v2 = cornerIndex + kMaxCorners + 1;
				g_poly[cornerIndex * 2 + 1].v3 = cornerIndex + kMaxCorners;
			}
			else
			{ // Specialfall: sista i varvet, g�u runt h�rnet korrekt
				cornerIndex = row * kMaxCorners + corner;
				g_poly[cornerIndex * 2].v1 = cornerIndex;
				g_poly[cornerIndex * 2].v2 = cornerIndex + 1 - kMaxCorners;
				g_poly[cornerIndex * 2].v3 = cornerIndex + kMaxCorners + 1 - kMaxCorners;

				g_poly[cornerIndex * 2 + 1].v1 = cornerIndex;
				g_poly[cornerIndex * 2 + 1].v2 = cornerIndex + kMaxCorners + 1 - kMaxCorners;
				g_poly[cornerIndex * 2 + 1].v3 = cornerIndex + kMaxCorners;
			}
		}

	// l�gger en kopia av originalmodellen i g_vertsRes & g_normalsRes

	for (row = 0; row < kMaxRow; row++)
		for (corner = 0; corner < kMaxCorners; corner++)
		{
			g_vertsRes[row][corner] = g_vertsOrg[row][corner];
			g_normalsRes[row][corner] = g_normalsOrg[row][corner];
			g_vertstex[row][corner][0]=(1-weight[row]);
			g_vertstex[row][corner][1]=weight[row];
		}

	// Build Model from cylinder data
	cylinderModel = LoadDataToModel(
			(GLfloat*) g_vertsRes,
			(GLfloat*) g_normalsRes,
			(GLfloat*) g_vertstex, // texCoords
			(GLfloat*) g_vertstex, // colors
			(GLuint*) g_poly, // indices
			kMaxRow*kMaxCorners,
			kMaxg_poly * 3);
}


//////////////////////////////////////
//		B O N E
// Desc:	en enkel ben-struct med en
//			pos-vektor och en rot-vektor
typedef struct Bone
{
	vec3 pos;
	mat4 rot;
} Bone;

///////////////////////////////////////
//		G _ B O N E S
// v�rt skelett; just nu inneh�ller det 2 ben ...
Bone g_bones[2];


///////////////////////////////////////////////////////
//		S E T U P	B O N E S
//
// Desc:	s�tter ut ben 0 i origo och
//			ben 1 p� pos (4.5, 0, 0)
void setupBones(void)
{
	g_bones[0].pos = SetVector(0.0f, 0.0f, 0.0f);
	g_bones[1].pos = SetVector(4.5f, 0.0f, 0.0f);
	g_bones[0].rot = IdentityMatrix();
	g_bones[1].rot = IdentityMatrix();
}


///////////////////////////////////////////////////////
//		D E F O R M	C Y L I N D E R
//
// Desc:	deformera cylindermeshen enligt skelettet
void DeformCylinder()
{
	// Point3D v1, v2;
	int row, corner;

	// f�r samtliga vertexar
	for (row = 0; row < kMaxRow; row++)
	{
		//float w = weight[row];
		mat4 Mbone1,Mbone2;

                
                /* Uppgift 1 lösning
		if(w == 0.0f){
                    
			Bone b = g_bones[0];
			mat4 trans1 = T(-b.pos.x,-b.pos.y,-b.pos.z);
			mat4 trans2 = T(b.pos.x,b.pos.y,b.pos.z);
			Mbone1 = Mult(trans2,(Mult(b.rot, trans1)));
		}

		else if(w == 1.0f){
			Bone b = g_bones[1];
			mat4 trans1 = T(-b.pos.x,-b.pos.y,-b.pos.z);
			mat4 trans2 = T(b.pos.x,b.pos.y,b.pos.z);
			Mbone2 = Mult(trans2,(Mult(b.rot, trans1)));
		}*/
	
                    
			Bone b1 = g_bones[0];
			mat4 trans1 = T(-b1.pos.x,-b1.pos.y,-b1.pos.z);
			mat4 trans2 = T(b1.pos.x,b1.pos.y,b1.pos.z);
			Mbone1 = Mult(trans2,(Mult(b1.rot, trans1)));
		

			Bone b2 = g_bones[1];
			trans1 = T(-b2.pos.x,-b2.pos.y,-b2.pos.z);
			trans2 = T(b2.pos.x,b2.pos.y,b2.pos.z);
			Mbone2 = Mult(trans2,(Mult(b2.rot, trans1)));
	
		
		


		for (corner = 0; corner < kMaxCorners; corner++)
		{

                    
                    
			//g_vertsRes[row][corner] = g_vertsOrg[row][corner]; // no transformation just copy original

			// ----=========	Uppgift 1: Hard skinning (stitching) i CPU ===========-----
			// Deformera cylindern enligt det skelett som finns
			// i g_bones.
			//
			// G�r hard skinning.
			//
			// g_bones inneh�ller benen.
			// g_vertsOrg inneh�ller ursprunglig vertexdata.
			// g_vertsRes inneh�ller den vertexdata som skickas till OpenGL.
			//
			// row traverserar i cylinderns l�ngdriktning,
			// corner traverserar "runt" cylindern
			vec3 vm = g_vertsOrg[row][corner];
			//g_vertsRes[row][corner] = MultVec3(Mbone2, vm);


			// ---=========	Uppgift 2: Soft skinning i CPU ===========------
			// Deformera cylindern enligt det skelett som finns
			// i g_bones.
			//
			// G�r soft skinning.
			//
			// g_bones inneh�ller benen.
			// g_boneWeights inneh�ller blendvikter f�r benen.
			// g_vertsOrg inneh�ller ursprunglig vertexdata.
			// g_vertsRes inneh�ller den vertexdata som skickas till OpenGL.
			vec3 vm1 = ScalarMult(MultVec3(Mbone1,vm),g_boneWeights[row][corner].x);
			vec3 vm2 = ScalarMult(MultVec3(Mbone2,vm),g_boneWeights[row][corner].y);
			g_vertsRes[row][corner] = VectorAdd(vm1,vm2);
		}
	}
}


/////////////////////////////////////////////
//		A N I M A T E	B O N E S
// Desc:	en v�ldigt enkel amination av skelettet
//			vrider ben 1 i en sin(counter)
void animateBones(void)
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	// Hur mycket skall vi vrida?
	float angle = sin(time * 3.f) / 2.0f * 3.0f;

	// rotera p� ben 1
	g_bones[1].rot = Rz(angle);
	//printf("%f %f\n", angle, time);float
}


///////////////////////////////////////////////
//		S E T	B O N E	R O T A T I O N
// Desc:	s�tter bone rotationen i vertex shadern
void setBoneRotation(void)
{
	// Uppgift 3 TODO: H�r beh�ver du skicka �ver benens rotation
	// till vertexshadern
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "rotation0"), 1, GL_TRUE, g_bones[0].rot.m);
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "rotation1"), 1, GL_TRUE, g_bones[1].rot.m);
        
        mat4 M0,M1;
        Bone b1 = g_bones[0];
        
        mat4 trans1 = T(-b1.pos.x,-b1.pos.y,-b1.pos.z);
        mat4 trans2 = T(b1.pos.x,b1.pos.y,b1.pos.z);
        M0 = Mult(trans2,(Mult(b1.rot, trans1)));


        Bone b2 = g_bones[1];
        trans1 = T(-b2.pos.x,-b2.pos.y,-b2.pos.z);
        trans2 = T(b2.pos.x,b2.pos.y,b2.pos.z);
        M1 = Mult(trans2,(Mult(b2.rot, trans1)));       
        
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "M0"), 1, GL_TRUE, M0.m);
        glUniformMatrix4fv(glGetUniformLocation(g_shader, "M1"), 1, GL_TRUE, M1.m);
    
}


///////////////////////////////////////////////
//		 S E T	B O N E	L O C A T I O N
// Desc:	s�tter bone positionen i vertex shadern
void setBoneLocation(void)
{
	// Uppgift 3 TODO: H�r beh�ver du skicka �ver benens position
	// till vertexshadern
        glUniform3f(glGetUniformLocation(g_shader, "pos0"),  g_bones[0].pos.x,g_bones[0].pos.y,g_bones[0].pos.z);
        glUniform3f(glGetUniformLocation(g_shader, "pos1"),  g_bones[1].pos.x,g_bones[1].pos.y,g_bones[1].pos.z);
}


///////////////////////////////////////////////
//		 D R A W	C Y L I N D E R
// Desc:	s�tter bone positionen i vertex shadern
void DrawCylinder()
{
	animateBones();

	// ---------=========	UPG 2 ===========---------
	// Ers�tt DeformCylinder med en vertex shader som g�r vad DeformCylinder g�r.
	// Begynnelsen till shaderkoden ligger i filen "shader.vert" ...

	//DeformCylinder();

	setBoneLocation();
	setBoneRotation();

// update cylinder vertices:
	glBindVertexArray(cylinderModel->vao);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderModel->vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3D)*kMaxRow*kMaxCorners, g_vertsRes, GL_DYNAMIC_DRAW);

	DrawModel(cylinderModel, g_shader, "in_Position", "in_Normal", "in_TexCoord");
}


void DisplayWindow()
{
	mat4 m;

	glClearColor(0.4, 0.4, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);

  m = Mult(projectionMatrix, modelViewMatrix);
  glUniformMatrix4fv(glGetUniformLocation(g_shader, "matrix"), 1, GL_TRUE, m.m);

	DrawCylinder();

	glutSwapBuffers();
}


void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(20, &OnTimer, value);
}

void keyboardFunc( unsigned char key, int x, int y)
{
// Add any keyboard control you want here
	if(key == 27)	//Esc
		exit(-1);
}

void reshape(GLsizei w, GLsizei h)
{
	Point3D cam = {5,0,8};
	Point3D look = {5,0,0};

    glViewport(0, 0, w, h);
    GLfloat ratio = (GLfloat) w / (GLfloat) h;
    projectionMatrix = perspective(90, ratio, 0.1, 1000);
 //   glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"), 1, GL_TRUE, projectionMatrix);
	modelViewMatrix = lookAt(cam.x, cam.y, cam.z,
							look.x, look.y, look.z,
							0,1,0);
}

/////////////////////////////////////////
//		M A I N
//
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2); // Might not be needed in Linux
	glutCreateWindow("Them bones");

	glutDisplayFunc(DisplayWindow);
	glutTimerFunc(20, &OnTimer, 0);
	glutKeyboardFunc( keyboardFunc );
	glutReshapeFunc(reshape);

	// Set up depth buffer
	glEnable(GL_DEPTH_TEST);

	// initiering
#ifdef WIN32
	glewInit();
#endif
	BuildCylinder();
	setupBones();
	g_shader = loadShaders("shader.vert" , "shader.frag");

	glutMainLoop();
	exit(0);
}
