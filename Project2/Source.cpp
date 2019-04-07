
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <time.h>
#include <stdio.h>      
#include <stdlib.h> 
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>


using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

// predeclares
void draw();
void redraw(int value);
void subDraw();
void init();
void subInit();
void writeBitmapString3f(float x, float y, float z, string s);
class Cell;
class InputBox;
class Button;
class ColorBox;

//IDs of windows
GLint mainID, subID;

//subWindow variables
int subWIDTH = 1920 / 2, subHEIGHT = 1080 / 2;

//variables for camera positioning
float eyex = 0, eyey = 0, eyez = 3300, r = 50, rOld = 0;
float deltaAnglex = 0.0f, deltaAngley = 0.0f, anglex = 0.0f, angley = 0.0f;
int xOrigin = -1, yOrigin = -1;

//variables for the size of the main cube and its children and hashmap to keep track of children
int side, siz = 10;
unordered_map<int, Cell> cells;

//subwindow variables
int orthoX1 = -100;
int orthoX2 = 100;
int orthoY1 = -100;
int orthoY2 = 100;
int orthoNear = -100;
int orthoFar = 100;

//controlling variables
int currentEdit = -1; int currentClick = -1, layer = 0;
bool start = false, preview = false;

//Lightning variables
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 50.0 };
GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

//simple data structure to keep track of cell origin in 3D (bottom left corner of cube) and alive or not
class Cell {
public:
	bool alive; int x, y, z;

	Cell(int X, int Y, int Z) {
		x = X;
		y = Y;
		z = Z;
		alive = false;
		//index = i;
	}

	Cell() {
		x = 0;
		y = 0;
		z = 0;
		alive = false;
	}

};

//Create and draw a button object in a 3D world given its % from left and % from top of viewport as origin. Notes: Z will be 0
class Button {
private:
	float percentFromLeft, percentFromTop, wordSize, width; string str;
public:
	Button() {
		percentFromLeft = 0;
		percentFromTop = 0;
		wordSize = 0;
		width = 0;
		str = "";
	}

	Button(float percentLeft, float percentTop, string s) {
		percentFromLeft = percentLeft;
		percentFromTop = percentTop;
		str = s;
		wordSize = 11.5;
		width = (float)str.length();
	}

	void draw() {
		float startButtonX = orthoX1 + abs(orthoX2 - orthoX1) * percentFromLeft / 100; // 2
		float startButtonY = orthoY2 - abs(orthoY2 - orthoY1) * percentFromTop / 100; //5


		glBegin(GL_LINE_LOOP);
		glVertex3f(startButtonX, startButtonY, 0.0);
		glVertex3f(startButtonX + wordSize * width, startButtonY, 0.0);
		glVertex3f(startButtonX + wordSize * width, startButtonY - 15.0, 0.0);
		glVertex3f(startButtonX, startButtonY - 15.0, 0.0);
		glEnd();

		glRasterPos3f(startButtonX + 5, startButtonY - 9, 0);
		for (int i = 0; i < width; i++) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, str[i]);
		}
		glLoadIdentity();
	}

	vector<float> getBoundaries() {
		float oriX = floor(percentFromLeft / 100 * subWIDTH * 20 / 100);
		float oriY = percentFromTop / 100 * subHEIGHT;
		vector<float> re = { oriX,oriY,oriX + wordSize * width,oriY + wordSize * width };
		return re;
	}
};

//This will create and draw an input box object in a 3D world given its % from left and % from top of viewport as origin. Notes: Z will be 0
class InputBox {
private:
	float percentFromLeft, percentFromTop, w, wordSize; string str;
public:
	InputBox() {
		percentFromLeft = 0;
		percentFromTop = 0;
		w = 0;
		wordSize = 0;
	}

	InputBox(float percentLeft, float percentTop, float width) {
		percentFromLeft = percentLeft;
		percentFromTop = percentTop;
		w = width;
		wordSize = 11.5;
		str = "0";
	}

	void draw() {
		float startButtonX = orthoX1 + abs(orthoX2 - orthoX1) * percentFromLeft / 100; // 2
		float startButtonY = orthoY2 - abs(orthoY2 - orthoY1) * percentFromTop / 100; //5
		glBegin(GL_LINE_LOOP);
		glVertex3f(startButtonX, startButtonY, 0.0);
		glVertex3f(startButtonX + wordSize * w, startButtonY, 0.0);
		glVertex3f(startButtonX + wordSize * w, startButtonY - 10.0, 0.0);
		glVertex3f(startButtonX, startButtonY - 10.0, 0.0);
		glEnd();

		if (str.length() != 0) {
			writeBitmapString3f(startButtonX + 5, startButtonY - 7, 0, str);
		}
		glLoadIdentity();
	}

	void update(char s) {
		str += s;
	}

	void backSpace() {
		if (str.length() > 0)
			str = str.substr(0, str.length() - 1);
	}

	string getStr() {
		return str;
	}

	vector<float> getBoundaries() {
		float oriX = floor(percentFromLeft / 100 * subWIDTH * 20 / 100);
		float oriY = percentFromTop / 100 * subHEIGHT;
		vector<float> re = { oriX,oriY,oriX + 122,oriY + 25 };
		return re;
	}
};

//This will create and draw a color-filled square object in a 3D world given its % from left and % from top of viewport as origin. Notes: Z will be 0
class ColorBox {
public:
	float percentFromTop, percentFromLeft, r, g, b, s;
	ColorBox() {
		percentFromLeft = 0;
		percentFromTop = 0;
		r = 0;
		g = 0;
		b = 0;
		s = 0;
	}

	ColorBox(float percentLeft, float percentTop, float red, float green, float blue, float size) {
		percentFromLeft = percentLeft;
		percentFromTop = percentTop;
		r = red;
		g = green;
		b = blue;
		s = size;
	}

	void draw() {
		float startX = orthoX1 + abs(orthoX2 - orthoX1) * percentFromLeft / 100;
		float startY = orthoY2 - abs(orthoY2 - orthoY1) * percentFromTop / 100;
		glColor3f(r, g, b);
		glBegin(GL_QUADS);
		glVertex3f(startX, startY, 0);
		glVertex3f(startX + (s / 2.84), startY, 0);
		glVertex3f(startX + (s / 2.84), startY + s, 0);
		glVertex3f(startX, startY + s, 0);
		glEnd();
	}

	void setColor(float blue) {
		b = blue;
	}

	float getColor() {
		return b;
	}
	vector<float> getBoundaries() {
		float oriX = floor(percentFromLeft / 100 * (subWIDTH - subWIDTH * 20 / 100) + subWIDTH * 20 / 100);
		float oriY = subHEIGHT / 2 + percentFromTop / 100 * subHEIGHT / 2;
		vector<float> re = { oriX,oriY,oriX + s + 10,oriY - s - 10 };
		return re;
	}
};

//subWindow objects
vector<InputBox> boxes; unordered_map<int, ColorBox> colorBoxes;
Button b1, b2, b3;

//drawing routine for a string given its xyz in a 3D world
void writeBitmapString3f(float x, float y, float z, string s) {
	glLoadIdentity();
	glRasterPos3f(x, y, z);
	for (int i = 0; i < s.length(); i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, s[i]);
	}
}

//This will setup the main window to be ready to draw by clearing the window and set up a 3D scene and world. Camera is included.
void setup() {
	glutSetWindow(mainID);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glFrustum(-5, 5, -5, 5, 1, 1000000);
	//gluPerspective(20, 16 / 9, 1, 100000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(eyex, eyey, eyez, 0, 0, 0, 0, 100, 0); // camera
	
	
}

//This will setup the main window to be ready to draw by clearing the window and set up a 3D scene and world. Camera is included.
void subSetup() {
	glutSetWindow(subID);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, orthoNear, orthoFar);
	//glFrustum(-5, 5, -5, 5, 1, 1000000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(1.0);
}

//camera control with mouse in main window
void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON && glutGetWindow() != mainID)
		glutSetWindow(mainID);
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			anglex += deltaAnglex;
			angley += deltaAngley;
			xOrigin = -1;
			yOrigin = -1;
		}
		else {// state = GLUT_DOWN
			xOrigin = x;
			yOrigin = y;
		}
	}
	if (button == 4) {
		rOld = r;
		r++;
		eyey = r / rOld * eyey;
		eyex = r / rOld * eyex;
		eyez = r / rOld * eyez;
		setup();
		draw();
	}
	else if (button == 3 && r > 1) {
		rOld = r;
		r--;
		eyey = r / rOld * eyey;
		eyex = r / rOld * eyex;
		eyez = r / rOld * eyez;
		setup();
		draw();
	}
}

//GUI control with mouse for sub window
void subMouseButton(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && glutGetWindow() != subID) {
		glutSetWindow(subID);
	}
	if (button == GLUT_LEFT_BUTTON) {
		for (int t = 0; t < boxes.size(); t++) {
			if (state == GLUT_UP && x >= boxes[t].getBoundaries()[0] && x <= boxes[t].getBoundaries()[2] && y >= boxes[t].getBoundaries()[1] && y <= boxes[t].getBoundaries()[3])
			{
				currentEdit = t;
			}
		}
		if (state == GLUT_UP && x >= b1.getBoundaries()[0] && x <= b1.getBoundaries()[2] && y >= b1.getBoundaries()[1] && y <= b1.getBoundaries()[3])
		{
			glutSetWindow(mainID);
			start = true;
			redraw(0);
			currentEdit = -1;
		}
		else if (state == GLUT_UP && x >= b2.getBoundaries()[0] && x <= b2.getBoundaries()[2] && y >= b2.getBoundaries()[1] && y <= b2.getBoundaries()[3])
		{
			glutSetWindow(subID);
			init();
			subInit();
			start = false;
			currentEdit = -1;
		}
		else if (state == GLUT_UP && x >= b3.getBoundaries()[0] && x <= b3.getBoundaries()[2] && y >= b3.getBoundaries()[1] && y <= b3.getBoundaries()[3]) {

			if (preview == true) {
				preview = false;
				subDraw();
			}
			else {
				preview = true;
				subDraw();
			}
			currentEdit = -1;
		}
		else {
			for (auto box : colorBoxes) {
				if (state == GLUT_DOWN && x >= box.second.getBoundaries()[0] && x <= box.second.getBoundaries()[2] && y <= box.second.getBoundaries()[1] && y >= box.second.getBoundaries()[3]) {
					currentClick = box.first;
				}
			}
			if (currentClick >= 0 && colorBoxes[currentClick].b == 1.0) {
				colorBoxes[currentClick].b = 0.0;
				cells[currentClick + layer * pow(side, 2)].alive = false;
			}
			else if (currentClick >= 0 && colorBoxes[currentClick].b == 0.0) {
				colorBoxes[currentClick].b = 1.0;
				cells[currentClick + layer * pow(side, 2)].alive = true;
			}
			cout << currentClick << endl;
			currentClick = -1;
			subDraw();
		}
	}


}

//moving the camera as the mouse move around (0,0,0)
void mouseMove(int x, int y) {

	// this will only be true when the left button is down
	if (xOrigin >= 0) {
		// update deltaAngle
		deltaAnglex = (x - xOrigin) * 0.001f;
		deltaAngley = (y - yOrigin) * 0.001f;
		// update camera's direction
		eyex = r * cos(anglex + deltaAnglex) * sin(angley + deltaAngley);
		eyez = r * sin(anglex + deltaAnglex) * sin(angley + deltaAngley);
		eyey = r * cos(angley + deltaAngley);

		setup();
		draw();
	}

}

//Keyboard interactions with input box
void subKeyFunc(unsigned char key, int x, int y)
{
	if (key == 'w' && layer < side - 1) {
		layer++;

	}
	if (key == 's' && layer > 0) {
		layer--;

	}
	if (key == 8 && currentEdit >= 0)
		boxes[currentEdit].backSpace();
	else if (currentEdit >= 0 && isdigit(key))
		boxes[currentEdit].update((char)key);
	if (isdigit(key) && boxes[0].getStr() != "") {
		init();
		subInit();

	}
	subDraw();


}

//to reshape the image as window change
void reshape(int width, int height) {

	/* define the viewport transformation */
	glViewport(0, 0, width, height);

}

void drawBorder() {
	if (preview == true || start == true) {
		int l = side - 1;

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz, -side * siz / 2 + l * 10 + siz);
		glVertex3i(-side * siz / 2 + l * 10 + siz, -side * siz / 2 + 0 * 10, -side * siz / 2 + l * 10 + siz);
		glEnd();
	}
}

//drawing routine for the whole GOL world
void drawGrid() {
	glColor3f(0, 0, 1);
	for (auto cell : cells) {
		Cell c = cell.second;
		int style = GL_QUADS;
		if (c.alive == true) {
			// GL_QUADS
		   //bottom
			glBegin(style);
			glVertex3i(c.x, c.y, c.z);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3i(c.x + siz, c.y, c.z);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3i(c.x + siz, c.y, c.z + siz);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3i(c.x, c.y, c.z + siz);
			glNormal3f(0.0, -1.0, 0.0);
			glEnd();

			//front
			glBegin(style);
			glVertex3i(c.x, c.y, c.z + siz);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3i(c.x + siz, c.y, c.z + siz);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3i(c.x + siz, c.y + siz, c.z + siz);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3i(c.x, c.y + siz, c.z + siz);
			glNormal3f(0.0, 0.0, 1.0);
			glEnd();

			//back
			glBegin(style);
			glVertex3i(c.x, c.y, c.z);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3i(c.x + siz, c.y, c.z);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3i(c.x + siz, c.y + siz, c.z);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3i(c.x, c.y + siz, c.z);
			glNormal3f(0.0, 0.0, -1.0);
			glEnd();

			//top
			glBegin(style);
			glVertex3i(c.x, c.y + siz, c.z);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3i(c.x + siz, c.y + siz, c.z);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3i(c.x + siz, c.y + siz, c.z + siz);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3i(c.x, c.y + siz, c.z + siz);
			glNormal3f(0.0, 1.0, 0.0);
			glEnd();

			//left
			glBegin(style);
			glVertex3i(c.x, c.y, c.z);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3i(c.x, c.y, c.z + siz);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3i(c.x, c.y + siz, c.z + siz);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3i(c.x, c.y + siz, c.z);
			glNormal3f(-1.0, 0.0, 0.0);
			glEnd();

			//right
			glBegin(style);
			glVertex3i(c.x + siz, c.y, c.z);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3i(c.x + siz, c.y + siz, c.z);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3i(c.x + siz, c.y + siz, c.z + siz);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3i(c.x + siz, c.y, c.z + siz);
			glNormal3f(1.0, 0.0, 0.0);
			glEnd();

		}
	}
}

//drawing routine for the menu in sub window
void menuPanel() {
	glViewport(0, 0, subWIDTH * 20 / 100, subHEIGHT);
	//glLoadIdentity();
	glColor3f(0., 0., 0.);
	glBegin(GL_LINES);
	glVertex3i(100, -100, 0);
	glVertex3i(100, 100, 0);
	glEnd();

	b1 = Button(1.0, 5.0, "START");
	b2 = Button(32.5, 5.0, "STOP");
	b3 = Button(58.0, 5.0, "PREVIEW");

	b1.draw();
	b2.draw();
	b3.draw();
	writeBitmapString3f(-50.0, 65.0, 0.0, "GOL size: ");
	boxes[0].draw();
	writeBitmapString3f(-90.0, 44.0, 0.0, "Dead if neighbors <");
	boxes[1].draw();
	writeBitmapString3f(-90.0, 23.0, 0.0, "Dead if neighbors >");
	boxes[2].draw();
	writeBitmapString3f(-90.0, 2.0, 0.0, "Alive if neighbors =");
	boxes[3].draw();
	writeBitmapString3f(-95.0, -19.0, 0.0, "Layer(x from back):" + to_string(layer));
	writeBitmapString3f(-40.0, -30.0, 0.0, "Notes:");
	writeBitmapString3f(-90.0, -41.0, 0.0, "GOL area = size^3");

}

//Drawing routine for alive cells of GOL in subwindow
void drawPreview() {
	glLoadIdentity();
	glViewport(subWIDTH * 20 / 100, subHEIGHT / 2, subWIDTH - subWIDTH * 20 / 100, subHEIGHT / 2);
	if (preview == true) {
		gluLookAt(-side * siz / 2 + (side - 1) * 10 + 40, -side * siz / 2 + (side - 1) * 10 + 15, -side * siz / 2 + (side - 1) * 10 + 20, 0, 0, 0, 0, 1000, 0);
		glEnable(GL_SCISSOR_TEST);
		glScissor(subWIDTH * 20 / 100, subHEIGHT / 2, subWIDTH - subWIDTH * 20 / 100, subHEIGHT / 2);
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		drawBorder();
		drawGrid();
	}
	else
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(subWIDTH * 20 / 100, subHEIGHT / 2, subWIDTH - subWIDTH * 20 / 100, subHEIGHT / 2);
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);

	}
}

//Drawing routine for the clicking cells
void drawClickingArea() {
	glEnable(GL_SCISSOR_TEST);
	glScissor(subWIDTH * 20 / 100, 0, subWIDTH - subWIDTH * 20 / 100, subHEIGHT / 2);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glLoadIdentity();
	glViewport(subWIDTH * 20 / 100, 0, subWIDTH - subWIDTH * 20 / 100, subHEIGHT / 2);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(-100, 99, 0);
	glVertex3f(100, 99, 0);
	glEnd();

	for (auto box : colorBoxes) {
		box.second.draw();
	}

}

//cumulative drawing routine for 3D axes and the GOL world for main window
void draw(void) {
	if (start == false)
	{
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(0.0, 0.0, 1.0);
		drawBorder();
		//glFlush();
		glutSwapBuffers();
	}
	else {
		//glMatrixMode(GL_MODELVIEW);
			/*
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINES);
			glVertex3f(100000.0, 0.0, 0.0);
			glVertex3f(-100000.0, 0.0, 0.0);
			glEnd();
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINES);
			glVertex3f(0.0, 100000.0, 0.0);
			glVertex3f(0.0, -100000.0, 0.0);
			glEnd();
			glColor3f(0.0, 0.0, 1.0);
			glBegin(GL_LINES);
			glVertex3f(0.0, 0.0, -100000.0);
			glVertex3f(0.0, 0.0, 100000.0);
			glEnd();
			*/
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		drawBorder();
		drawGrid();


		//glFlush();
		glutSwapBuffers();
	}
}

//cumulative drawing routines for sub window
void subDraw() {
	subSetup();
	menuPanel();
	drawPreview();
	drawClickingArea();
	glFlush();
}

//initialize the program with the size of the GOL world (cube) and assigning indexes to each cell
void init() {
	//srand(time(NULL));
	int count = 0;
	side = stoi(boxes[0].getStr());
	cells.clear();
	for (int i = 0; i < side; i += 1) {
		for (int j = 0; j < side; j += 1)
			for (int k = 0; k < side; k += 1) {
				cells.insert({ count, Cell(-side * siz / 2 + k * 10, -side * siz / 2 + j * 10, -side * siz / 2 + i * 10) });
				count++;
			}
	}

}

//initialize sub window with necessary objects
void subInit() {
	if (boxes.size() == 0) {
		boxes.push_back(InputBox(15.0, 20.0, 11.0));
		boxes.push_back(InputBox(15.0, 30.0, 11.0));
		boxes.push_back(InputBox(15.0, 40.0, 11.0));
		boxes.push_back(InputBox(15.0, 50.0, 11.0));
	}
	int index = 0;
	float range = subHEIGHT / 2 - subHEIGHT / 2 * 0.1;
	float size = (range / subHEIGHT) * 200 / side;
	colorBoxes.clear();
	for (int i = 0; i < side; i += 1)
		for (int j = 0; j < side; j += 1) {
			colorBoxes.insert({ index, ColorBox((subWIDTH - range / 2) / subWIDTH / 2 * 100 + j * (size / 2.84) - 7,95 - i * size,0,0,0,size) });
			index++;
		}
}

//function to find the index of the cell in the hashmap base of the Cell origin
int getIndex(int x, int y, int z) {
	for (unordered_map<int, Cell>::const_iterator it = cells.begin(); it != cells.end(); ++it) {
		if (it->second.x == x && it->second.y == y && it->second.z == z)
			return it->first;
	}
	return -1;
}

//count the # of alive neighbor of a specified cell and its index
int checkNeighbors(int idx, Cell c) {
	int count = 0;
	for (int i = -1; i < 2; i += 1) {
		for (int j = -1; j < 2; j += 1) {
			for (int k = -1; k < 2; k += 1) {
				int index = getIndex(c.x + i * siz, c.y + j * siz, c.z + k * siz);
				if (index == -1)
					continue;
				if (index == idx)
					continue;
				if (cells[index].alive == true)
					count++;
			}
		}
	}
	return count;
}

//rules of the GOL and drawing after calculation
void GOL() {
	for (auto &c : cells) {

		int neighBors = checkNeighbors(c.first, c.second);
		if (c.second.alive == true && (neighBors < stoi(boxes[1].getStr()) || neighBors > stoi(boxes[2].getStr())))
			c.second.alive = false;

		else if (c.second.alive == false && neighBors == stoi(boxes[3].getStr()))
			c.second.alive = true;

	}
	setup();
	draw();

}

// timer to redraw
void redraw(int value) {
	if (start == true) {
		GOL();
		glutTimerFunc(500, redraw, 0);
	}
}

//gl calls to set up sub window and its tools
void subWindow(void) {
	glutInitWindowSize(subWIDTH, subHEIGHT);           // set a window size
	glutInitWindowPosition(50, 50);
	subID = glutCreateWindow("Menu");
	subInit();
	glutSetWindow(subID);
	glutMouseFunc(subMouseButton);
	glutKeyboardFunc(subKeyFunc);
	glutDisplayFunc(subDraw);
}

// gl calls to set up main window and its tools
void mainWindow(void) {
	glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	mainID = glutCreateWindow("Game Of Life 3D");
	init();
	glShadeModel(GL_FLAT);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutTimerFunc(500, redraw, 0);
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	
	subWindow();
	mainWindow();
	glewExperimental = GL_TRUE;
	glewInit();
	glutMainLoop();
}


