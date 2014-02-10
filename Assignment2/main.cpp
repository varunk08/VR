/*
*
* Demonstrates how to load and display an Wavefront OBJ file.
* Using triangles and normals as static object. No texture mapping.
*
* OBJ files must be triangulated!!!
* Non triangulated objects wont work!
* You can use Blender to triangulate
*
*/
#include <GL/freeglut.h>
#include <windows.h> // remove this for mac

#include <GL/gl.h> // These are in a different spot for mac and glut is already installed
#include <GL/glu.h>

#include <gmtl/gmtl.h> // This is a header-only library in the project folders

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "plyReader.h"

#include <sixense.h>
#include <sixense_math.hpp>
#ifdef WIN32
#include <sixense_utils/mouse_pointer.hpp>
#endif
#include <sixense_utils/derivatives.hpp>
#include <sixense_utils/button_states.hpp>
#include <sixense_utils/event_triggers.hpp>
#include <sixense_utils/controller_manager/controller_manager.hpp>

#define KEY_ESCAPE 27
#define MAX_PARTICLES 300

using namespace std;

/************************************************************************
Window
************************************************************************/
#define ANAGLYPH  1 
#define STEREO  2
#define ANAGLYPH_ASYMMETRIC 3
typedef struct {
	int width;
	int height;
	char* title;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;





/***************************************************************************
* Program code
***************************************************************************/

Model_PLY ply;
float g_rotation;
glutWindow win;
bool renderPaused = false;
float prevTime;
float cubeRotation = 0.0f;
float cubeAmplitude = 0.2f;
gmtl::Vec3f _camPos = gmtl::Vec3f(0.0, 0.0, 0.0);
gmtl::Vec3f _target = gmtl::Vec3f(0.0, 0.0, 0.0);
float _viewAngle = 3.14f;
float _forward = 0.0f;
float _strafe = 0.0f;
int _ProgramMode;

float xDisplacement = 0.0;
float yDisplacement = 0.0;
static bool controller_manager_screen_visible = true;
std::string controller_manager_text_string;
void DrawSphere(float size, gmtl::Vec3f translation, gmtl::Vec3f color, char rotationAxis){
	glPushMatrix();
	float *_translation = translation.getData();
	glTranslatef(_translation[0], _translation[1], _translation[2]);
	switch (rotationAxis){
	case 'x':
		glTranslatef(0.0, cubeAmplitude * sin(cubeRotation), cubeAmplitude * cos(cubeRotation));
		break;
	case 'y':
		glTranslatef(cubeAmplitude * sin(cubeRotation), 0.0f, cubeAmplitude * cos(cubeRotation));
		break;
	case 'z':
		glTranslatef(cubeAmplitude * sin(cubeRotation), cubeAmplitude * cos(cubeRotation), 0.0f);
		break;
	}

	float *c = color.getData();
	glColor3f(c[0], c[1], c[2]);
	glutSolidSphere(size, 25, 25);
	glPopMatrix();
}
void DrawTunnel(){
	
	float xLeft = -1.3333 * 0.5;
	float xRight = 1.333 * 0.5;
	float top = 0.5;
	float bottom = -0.5;
	float zNear = -1.0f;
	float zFar = -10.0f;
	
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(1.0f);
	glBegin(GL_LINES); 
	//Floor, Roof lines
	for (GLfloat x = xLeft; x <= xRight; x += 0.15){
		glVertex3f(x, bottom, zNear); glVertex3f(x, bottom, zFar);
		glVertex3f(x, top, zNear); glVertex3f(x, top, zFar);
	}
	for (GLfloat z = zNear; z >= zFar; z -= 0.5){
		glVertex3f(xLeft, bottom, z); glVertex3f(xRight, bottom, z); //floor
		glVertex3f(xLeft, top, z); glVertex3f(xLeft, bottom, z); //Left wall vertical lines
		glVertex3f(xRight, top, z); glVertex3f(xRight, bottom, z); //Right vertical lines
		glVertex3f(xLeft, top, z); glVertex3f(xRight, top, z); //Roof vertical lines
	}
	
	//Right,  Left wall
	for (GLfloat y = top; y >= bottom; y -= 0.1){
		glVertex3f(xLeft, y, zNear); glVertex3f(xLeft, y, zFar);
		glVertex3f(xRight, y, zNear); glVertex3f(xRight, y, zFar);

	}
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}
void DrawBunny(float x, float y, float z, float scale = 1.0f, float r = 0.5f, float g = 0.5f, float b = 1.0f){
	
	glPushMatrix(); // gluLookat puts a transform in model view, save it with pushmatrix
	glTranslatef(x, y ,z);
	glRotatef(g_rotation, 0, 1, 0); // post multiply the current model view with a rotate
	glRotatef(90, 0, 1, 0);
	glScalef(scale,scale,scale);
	glColor3f(r, g, b);
	ply.Draw(); // calls the ply code to render the triangles stored in the object
	glPopMatrix(); // get rid of the rotations
	//g_rotation++; // a global variable that is incremented each frame and used in the rotation transform
}
void DrawScene(){
	
	for (GLfloat z = -1.5; z >= -5; z--){
		DrawBunny(0, -0.3, z, 2.0f);
	}

	//Particle fountain
	glPushMatrix();
	glTranslatef(0, 0, 0);
	//DrawObjects();
	glPopMatrix();

	DrawSphere(0.1, gmtl::Vec3f(0, 0, -2), gmtl::Vec3f(1, 1, 0), 'z');
	DrawSphere(0.1, gmtl::Vec3f(0, 0, -3), gmtl::Vec3f(1, 0.5, 0), 'x');
	//wire cube
	glPushMatrix();
	glColor3f(0.0f, 1.0f, 1.0f);
	glLineWidth(10.0);
	glTranslatef(0.0, 0.0, -1.0);
	glRotatef(cubeRotation, 1, 1, 1);
	//glutWireIcosahedron();// (1.0);
	glPopMatrix();


	DrawTunnel();

}

float _eyeOffset = 0.002;
void display()
{

	
	glClearColor(0.2, 0.2, 0.2, 1.0);

	if (_ProgramMode == STEREO) {
		glDrawBuffer(GL_BACK_LEFT);

	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the color and the depth buffer
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLfloat aspect = (GLfloat)win.width / win.height;
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	

	//cout << glutGet(GLUT_ELAPSED_TIME) << endl;
	if (glutGet(GLUT_ELAPSED_TIME) - prevTime >= (1000 / 24.0f)){


		prevTime = glutGet(GLUT_ELAPSED_TIME);
		if (!renderPaused){
			cubeRotation += 0.05;
			g_rotation++;
		}
	}

	//TODO: changing angle and fix forward motion beyond target
	gmtl::Vec3f viewVector = _camPos - _target;
	gmtl::Vec3f viewDir = viewVector;
	gmtl::normalize(viewDir);
	_camPos = _camPos + _forward * viewDir;
	/*gmtl::AxisAnglef myAxisAngle;
	myAxisAngle.setAngle(_viewAngle);
	myAxisAngle.setAxis(gmtl::Vec3f(0.0, 1.0, 0.0));
	gmtl::Matrix44f myMat = gmtl::make< gmtl::Matrix44f >(myAxisAngle);
	gmtl::xform(viewVector, myMat, viewVector);
	*/
	gmtl::Vec3f perpVec;
	gmtl::cross(perpVec, viewDir, gmtl::Vec3f(0, 1, 0));
	gmtl::normalize(perpVec);
	gmtl::Vec3f newCamPos = _camPos + _strafe * perpVec;
	gmtl::Vec3f goalVec = _target + _strafe * perpVec;
	float *goal = goalVec.getData();
	float *eye = newCamPos.getData();

	//Left eye
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // Resets the Model view matrix
	gmtl::Vec3f leftCamPos = newCamPos + _eyeOffset * perpVec;
	eye = leftCamPos.getData();
	 gluLookAt(eye[0], eye[1], eye[2], goal[0], goal[1], goal[2], 0, 1, 0); // eye xyz goal xyz up vec xyz
	
	if (_ProgramMode != STEREO) glColorMask(true, false, false, false);
	DrawScene();


	//Right Eye
	if (_ProgramMode == STEREO) {
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the color and the depth buffer
	}
	else glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // Resets the Model view matrix
	gmtl::Vec3f rightCamPos = newCamPos - _eyeOffset * perpVec;
	eye = rightCamPos.getData();
	gluLookAt(eye[0], eye[1], eye[2], goal[0], goal[1], goal[2], 0, 1, 0); // eye xyz goal xyz up vec xyz
	if (_ProgramMode != STEREO) glColorMask(false, true, true, false);
	DrawScene();

	_forward = 0.0f;
	//_strafe = 0.0f; //_viewAngle = f;
	glColorMask(true, true, true, true);
	glutSwapBuffers(); // finished drawing this frame

	//cout << "Mode: " << _ProgramMode << endl;
	
}

void animate(){
	if (glutGet(GLUT_ELAPSED_TIME) - prevTime >= (1000 / 24.0f)){


		prevTime = glutGet(GLUT_ELAPSED_TIME);
		if (!renderPaused){
			cubeRotation += 0.05;
			g_rotation++;
		}
	}

}
gmtl::Vec3f _camDir;
void moveCamera(){
	//move camera position and target asymmetric
	//xDisplacement += _strafe;
	cout << "Ydisp: " << yDisplacement << endl;
	//_camPos[0] -= _strafe;
	_camPos[0] = -xDisplacement;
	_camPos[1] = yDisplacement;
	_camPos[2] += _forward;
	_camDir.set(0.0, 0.0, -1.0);
	_target = _camPos + _camDir;
	_forward = 0;
	_strafe = 0;
}
float left_mouse_pos[2] = { 0, 0};
void GetXDisplacement(){
	int cont, base;
	sixenseAllControllerData acd;
	for (base = 0; base<sixenseGetMaxBases(); base++) {
		sixenseSetActiveBase(base);
		sixenseGetAllNewestData(&acd);

		
		cont = 0; //LEFT controller
			if (sixenseIsControllerEnabled(cont)) {

		//		printf("base: %d controller: %d   pos: %f %f %f \n\n", base, cont, acd.controllers[cont].pos[0], acd.controllers[cont].pos[1], acd.controllers[cont].pos[2]);
				xDisplacement = (-acd.controllers[cont].pos[0] )/ 400.0;
				yDisplacement = (acd.controllers[cont].pos[1] - 350) / 250.0 ;
		}
	}

	
}
void UpdateSixenseControl(){
	sixenseSetActiveBase(0);
	sixenseAllControllerData acd;
	sixenseGetAllNewestData(&acd);
	sixenseUtils::getTheControllerManager()->update(&acd);

	//check_for_button_presses(&acd);


	GetXDisplacement();


	// Either draw the controller manager instruction screen, or display the controller information
	if (controller_manager_screen_visible) {
		//draw_controller_manager_screen();
	}
	else {
		//draw_controller_info();
		//GetXDisplacement();
	}
}

// This is run once to setup lights, render properties and the camera matrix
void displayAsymmetric(){
		UpdateSixenseControl();

	animate();
	moveCamera();
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	float aspectRatio = (float) win.width / (float) win.height;
	float radians = (3.14159 / 180)  * win.field_of_view_angle / 2;
	float wd2 = win.z_near * tan(radians);
	float testHeight = 1.0f;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float left = -aspectRatio * wd2;
	float right = aspectRatio * wd2;
	float top = wd2;
	float bottom = -wd2;
	/*glFrustum(left, right, bottom, top, win.z_near, win.z_far);

	glMatrixMode(GL_MODELVIEW);
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(_camPos[0], _camPos[1], _camPos[2],_target[0], _target[1], _target[2],0, 1.0, 0);
	DrawScene();
	glutSwapBuffers();

	*/

	
	//RIGHT EYE
	left = -aspectRatio * wd2 - _eyeOffset;
	right = aspectRatio * wd2 - _eyeOffset;
	//cout << "EyeOffset: " << _eyeOffset << endl;
	left = -aspectRatio * 0.5f + xDisplacement;
	right = aspectRatio * 0.5f + xDisplacement;
	top = 0.5f - yDisplacement;
	bottom = -0.5f - yDisplacement;
	glFrustum(left, right, bottom, top, 1, win.z_far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluLookAt(_camPos[0], _camPos[1], _camPos[2], _target[0]  , _target[1], _target[2], 0, 1.0, 0);
	/*gluLookAt(camera.pos.x + right.x, camera.pos.y + right.y, camera.pos.z + right.z,
		camera.pos.x + right.x + camera.dir.x,
		camera.pos.y + right.y + camera.dir.y,
		camera.pos.z + right.z + camera.dir.z,
		camera.up.x, camera.up.y, camera.up.z);*/
	//glColorMask(true, false, false, false);
	DrawScene();
	//cout << "Left: \n" << "left: " << left << "\nright: " << right << "\ntop: " << top << "\nbottom: " << bottom << endl;
	//cout << "CAM: " << _camPos<<endl;
	// Left eye
	/*glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	top = wd2;
	bottom = -wd2;
	left = -aspectRatio * wd2 + _eyeOffset;
	right = aspectRatio * wd2 + _eyeOffset;

	left = -aspectRatio * (testHeight / 2) + _eyeOffset;
	right = aspectRatio * (testHeight / 2) + _eyeOffset;
	top = testHeight / 2;
	bottom = -testHeight / 2;

	glFrustum(left, right, bottom, top, win.z_near, win.z_far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(_camPos[0], _camPos[1], _camPos[2], _target[0], _target[1], _target[2], 0, 1.0, 0);*/
	/*gluLookAt(camera.pos.x - right.x, camera.pos.y - right.y, camera.pos.z - right.z,
		camera.pos.x - right.x + camera.dir.x,
		camera.pos.y - right.y + camera.dir.y,
		camera.pos.z - right.z + camera.dir.z,
		camera.up.x, camera.up.y, camera.up.z);*/
	/*glColorMask(false, true, true, false);
	DrawScene();
	cout << "Right: \n" << "left: " << left << "\nright: " << right << "\n top: " << top << "\n bottom: " << bottom << endl;
	*/
	//cout << "\n" << endl;
	//glColorMask(true, true, true, true);


	glutSwapBuffers();

}
void initialize()
{
	GLfloat aspect = (GLfloat)win.width / win.height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.1f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
	GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
}
void reshape(int width, int height){
	glViewport(0, 0, width, height);
	GLfloat aspect = (GLfloat)width / (GLfloat)height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	//glMatrixMode(GL_MODELVIEW);

}
void ChangeProjection(){
	GLfloat aspect = (GLfloat)win.width / win.height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);
	glMatrixMode(GL_MODELVIEW);
}
// This checks for key presses. Make sure you use break properly in the switch statement.
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case KEY_ESCAPE:
		//3glutLeaveFullScreen(); 
		exit(0);
		break;
	case 111: //o
		_eyeOffset -= 0.0001;
		break;
	case 112: //p
		_eyeOffset += 0.0001;
		break;
	case 32://SPACE
		//pause animations
		renderPaused = !renderPaused;
		break;

	case 119:
	case 87: //w
	{
				 _forward -= 0.1;
				 break;
	}
	case 83:
	case 115: //s
	{
				  _forward += 0.1;
				  break;
	}
	case 65:
	case 97: //a
	{
				 _strafe += 0.01;
				 break;
	}
	case 68:
	case 100: //d
	{
				  _strafe -= 0.01;
				  break;
	}
	case 81:
	case 113: //q
	{
				  _viewAngle += 0.1;
				  cout << "Decrementing view angle: " << _viewAngle << endl;
				  break;
	}
	case 69:
	case 101: //e
	{
				  _viewAngle -= 0.1;
				  cout << "Incrementing view angle: " << _viewAngle << endl;
				  break;
	}

	case 78: // N
		//decrement near plane
	{
				 win.z_near -= 0.01;
				 cout << "Decrementing near plane: " << win.z_near << endl;
				 ChangeProjection();
				 break;
	}
	case 110: // n
		//increment near plane
	{
				  win.z_near += 0.01;
				  cout << "Incrementing near plane: " << win.z_near << endl;
				  ChangeProjection();
				  break;
	}
	case 70: // F
		//decrement farr plane
	{
				 win.z_far -= 0.5;
				 cout << "Decrementing far plane: " << win.z_far << endl;
				 ChangeProjection();
				 break;
	}
	case 102: // f
		//increment far plane
	{
				  win.z_far += 0.01;
				  cout << "Incrementing far plane: " << win.z_far << endl;
				  ChangeProjection();
				  break;
	}
	case 86: // V
		//decrement fov
	{
				 win.field_of_view_angle--;
				 cout << "Decrementing fov: " << win.field_of_view_angle << endl;
				 ChangeProjection();
				 break;
	}
	case 118: // v
		//increment fov
	{
				  win.field_of_view_angle++;
				  cout << "Incrementing fov: " << win.field_of_view_angle << endl;
				  ChangeProjection();
				  break;
	}
	default:
		break;
	}
}

// flags that the controller manager system can set to tell the graphics system to draw the instructions
// for the player

void controller_manager_setup_callback(sixenseUtils::ControllerManager::setup_step step) {

	if (sixenseUtils::getTheControllerManager()->isMenuVisible()) {

		// Turn on the flag that tells the graphics system to draw the instruction screen instead of the controller information. The game
		// should be paused at this time.
		controller_manager_screen_visible = true;

		// Ask the controller manager what the next instruction string should be.
		controller_manager_text_string = sixenseUtils::getTheControllerManager()->getStepString();

		// We could also load the supplied controllermanager textures using the filename: sixenseUtils::getTheControllerManager()->getTextureFileName();

	}
	else {

		// We're done with the setup, so hide the instruction screen.
		controller_manager_screen_visible = false;

	}

}
int main(int argc, char **argv)
{

	cout << "\n Which mode do you want to run in? \n ANAGLYPH (or) ACTIVE STEREO?\n Press 1 for ANAGLYPH or \nPress 2 for ACTIVE STEREO or \n\n\nPress 3 for ASSIGNMENT 3: Head-Tracked VR using Razer Hydra: ";
	int option;
	cin >> option;

	// set window values
	win.width = 640;
	win.height = 480;
	win.title = "OpenGL/GLUT OBJ Loader.";
	win.field_of_view_angle = 45;
	win.z_near = 1.0f;
	win.z_far = 50.0f;

	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	void(*displayfunc)() = NULL;
	switch (option){
	case 1:
		_ProgramMode = 1;
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);  // Display Mode
		 displayfunc = &display;
		cout << "Mode = " << _ProgramMode << endl;
		break;
	case 2:
		_ProgramMode = 2;
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);  // Display Mode
		displayfunc = &display;
		cout << "Mode = " << _ProgramMode << endl;
		break;
	case 3:
		_ProgramMode = 3;
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		//glutFullScreen(); //set glut to fullscreen using the 
		displayfunc = &displayAsymmetric;	 // register Display Function
		cout << "Mode = " << _ProgramMode << endl;
		break;
	default:
		cout << "\nError. Invalid option selected\n";
	}
	glutInitWindowSize(win.width, win.height);	 // set window size
	glutCreateWindow(win.title);	 // create Window

	glutDisplayFunc(displayfunc);	 // register Display Function
	glutReshapeFunc(reshape);
	glutIdleFunc(displayfunc);	 // register Idle Function
	glutKeyboardFunc(keyboard);	 // register Keyboard Handler
	initialize();
	ply.Load("BAPbunny.ply");
	// Init sixense
	sixenseInit();

	// Init the controller manager. This makes sure the controllers are present, assigned to left and right hands, and that
	// the hemisphere calibration is complete.
	sixenseUtils::getTheControllerManager()->setGameType(sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER);
	sixenseUtils::getTheControllerManager()->registerSetupCallback(controller_manager_setup_callback);

	glutMainLoop();

	sixenseExit();
	return 0;
}