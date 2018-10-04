// Demo of heavily simplified sprite engine
// by Ingemar Ragnemalm 2009
// used as base for lab 4 in TSBK03.
// OpenGL 3 conversion 2013.

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// uses framework Cocoa
#else
	#include <GL/gl.h>
	#include "MicroGlut.h"
#endif

#include <stdlib.h>
#include "LoadTGA.h"
#include "SpriteLight.h"
#include "GL_utilities.h"

#include <math.h>

// L�gg till egna globaler h�r efter behov.
float kCohesionRadius = 150.0;
float kAvoidanceRadius = 100.0;
float kCohesionWeight = 0.001;
float kAvoidanceWeight = 0.01;
float kAlignmentWeight = 0.01;
float kRandomWeight = 0.01;
int frameCounter = 0;

TextureData *sheepFace, *blackFace, *dogFace, *foodFace;

FPoint subtract(FPoint a, FPoint b){
	FPoint res = {a.h-b.h, a.v - b.v};

	return res;
}
FPoint mult(FPoint a, float b){
	FPoint res = {a.h*b, a.v*b};
	return res;
}
FPoint add(FPoint a, FPoint b){
	FPoint res = {a.h + b.h, a.v + b.v};
	return res;
}
FPoint divideByScalar(FPoint a, float b){
	FPoint res = {a.h/b, a.v/b};
	return res;
}


float calculateDistanceFPoint(FPoint fp1, FPoint fp2){
	return sqrt( pow(fp1.h - fp2.h, 2.0) + pow(fp1.v - fp2.v, 2.0) );
}



void SpriteBehavior() // Din kod!
{
// L�gg till din labbkod h�r. Det g�r bra att �ndra var som helst i
// koden i �vrigt, men mycket kan samlas h�r. Du kan utg� fr�n den
// globala listroten, gSpriteRoot, f�r att kontrollera alla sprites
// hastigheter och positioner, eller arbeta fr�n egna globaler.
// L�gg till din labbkod h�r. Det g�r bra att �ndra var som helst i
// koden i �vrigt, men mycket kan samlas h�r. Du kan utg� fr�n den
// globala listroten, gSpriteRoot, f�r att kontrollera alla sprites
// hastigheter och positioner, eller arbeta fr�n egna globaler.
	SpritePtr firstSp;
	SpritePtr secondSp;



	float distance;
	firstSp = gSpriteRoot;
	int outerCount = 0;
	//räkna ut average osv
	while (firstSp != NULL){
		secondSp = gSpriteRoot;
		FPoint totalPosition = {0.0, 0.0};
		FPoint avoidanceVector = {0.0,0.0};
		FPoint speedDiff = {0.0,0.0};

		float nSpritesNearby = 0.0;
		printf("OUTER sprite: %i\n", outerCount);
		float maxDistance = 0.0;
		while (secondSp != NULL){
			if(firstSp != secondSp){

				distance = calculateDistanceFPoint(firstSp->position, secondSp->position);
				if(distance > maxDistance)
				{
					maxDistance = distance;
				}
				//Todo: check if white sheep or not
				if(distance <= kCohesionRadius){
					totalPosition.h += secondSp->position.h;
					totalPosition.v += secondSp->position.v;

					speedDiff = add(speedDiff,subtract(secondSp->speed,firstSp->speed));

					nSpritesNearby += 1.0;
				}

				if(distance <= kAvoidanceRadius){
					FPoint posDiffVector = subtract(firstSp->position,secondSp->position);
					float distanceFactor = (1-sqrt(distance/kAvoidanceRadius));
					posDiffVector = mult(posDiffVector,distanceFactor);
					avoidanceVector = add(avoidanceVector, posDiffVector);
				}
			}
			secondSp=secondSp->next;
		}

		//calculate average position
		if(nSpritesNearby > 0){


			if(firstSp->face == blackFace){
				FPoint randVec = {(rand()/(float)RAND_MAX),(rand()/(float)RAND_MAX)};
				randVec = mult(randVec,kRandomWeight);
				firstSp->speed = add(firstSp->speed,randVec);
			}
			else
			{

				//COHESION
				FPoint avgPosition = {totalPosition.h/nSpritesNearby, totalPosition.v/nSpritesNearby};
				FPoint cohesionVector = subtract(avgPosition,firstSp->position);
				cohesionVector = mult(cohesionVector,kCohesionWeight);
				firstSp->speed = add(firstSp->speed,cohesionVector);

				//SEPARTAION
				avoidanceVector = divideByScalar(avoidanceVector,nSpritesNearby);
				avoidanceVector = mult(avoidanceVector,kAvoidanceWeight);
				printf("\tavoidanceVector: (%f,%f)\n",avoidanceVector.v,avoidanceVector.h );
				firstSp->speed = add(firstSp->speed,avoidanceVector);

				//ALIGNMENT
				speedDiff = divideByScalar(speedDiff,nSpritesNearby);
				speedDiff = mult(speedDiff,kAlignmentWeight);
				firstSp->speed = add(firstSp->speed,speedDiff);
			}


		}




		firstSp = firstSp->next;

		outerCount += 1;
	}



	//uppdatera
	// do{
	//
	// 	distance = calculateDistanceFPoint(sp->position, sp->next->position);
	// 	//Todo: check if white sheep or not
	// 	if(distance <= kCohesionRadius){
	// 		totalPosition.h += sp->position.h;
	// 		totalPosition.v += sp->position.v;
	// 		count += 1.0;
	// 	}
	//
	// 	sp = sp->next;
	// } while (sp != NULL);


}

// Drawing routine
void Display()
{
	SpritePtr sp;
	printf("=== FRAME NR %i ===\n",frameCounter);
	glClearColor(0, 0, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT+GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawBackground();

	SpriteBehavior(); // Din kod!

// Loop though all sprites. (Several loops in real engine.)
	sp = gSpriteRoot;
	do
	{
		HandleSprite(sp); // Callback in a real engine
		DrawSprite(sp);
		sp = sp->next;
	} while (sp != NULL);

	glutSwapBuffers();
	frameCounter += 1;
}

void Reshape(int h, int v)
{
	glViewport(0, 0, h, v);
	gWidth = h;
	gHeight = v;
}

void Timer(int value)
{
	glutTimerFunc(20, Timer, 0);
	glutPostRedisplay();
}

// Example of user controllable parameter
float someValue = 0.0;

void Key(unsigned char key,
         __attribute__((unused)) int x,
         __attribute__((unused)) int y)
{
  switch (key)
  {
    case '+':
    	someValue += 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case '-':
    	someValue -= 0.1;
    	printf("someValue = %f\n", someValue);
    	break;
    case 0x1b:
      exit(0);
  }
}

void Init()
{
	//TextureData *sheepFace, *blackFace, *dogFace, *foodFace;

	LoadTGATextureSimple("bilder/leaves.tga", &backgroundTexID); // Bakgrund

	sheepFace = GetFace("bilder/sheep.tga"); // Ett f�r
	blackFace = GetFace("bilder/blackie.tga"); // Ett svart f�r
	dogFace = GetFace("bilder/dog.tga"); // En hund
	foodFace = GetFace("bilder/mat.tga"); // Mat

	NewSprite(sheepFace, 100, 200, 1, 1);
	NewSprite(sheepFace, 200, 100, 1.5, -1);
	NewSprite(sheepFace, 250, 200, -1, 1.5);
	NewSprite(sheepFace, 300, 100, -1, 1.5);
	NewSprite(blackFace, 350, 200, -1, 1.5);
	NewSprite(foodFace, 600, 600, -1, 1.5);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitContextVersion(3, 2);
	glutCreateWindow("SpriteLight demo / Flocking");

	glutDisplayFunc(Display);
	glutTimerFunc(20, Timer, 0); // Should match the screen synch
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);

	InitSpriteLight();
	Init();

	glutMainLoop();
	return 0;
}
