#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

class CustomCamera : public Camera
{
public:
	double camDist;
	double fi1, fi2;

	
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	void SetUpCamera()
	{
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;

struct Vector {
	float x;
	float y;
	float z;
};

Vector FindNormal(const double(&p1)[3], const double(&p2)[3], const double(&p3)[3])
{
	Vector v1, v2, normal;

	v1.x = p2[0] - p1[0];
	v1.y = p2[1] - p1[1];
	v1.z = p2[2] - p1[2];

	v2.x = p3[0] - p1[0];
	v2.y = p3[1] - p1[1];
	v2.z = p3[2] - p1[2];

	normal.x = v1.y * v2.z - v1.z * v2.y;
	normal.y = -(v1.x * v2.z - v1.z * v2.x);
	normal.z = v1.x * v2.y - v1.y * v2.x;

	float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= length;
	normal.y /= length;
	normal.z /= length;
	glNormal3d(normal.x, normal.y, normal.z);
	return normal;
}

class CustomLight : public Light
{
public:
	CustomLight()
	{
		pos = Vector3(1, 1, 3);
	}

	
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		glLightfv(GL_LIGHT0, GL_POSITION, position);

		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);

		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;



int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;


	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;


void initRender(OpenGL *ogl)
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	

	RGBTRIPLE *texarray;

	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);


	free(texCharArray);
	free(texarray);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	ogl->mainCamera = &camera;
	ogl->mainLight = &light;


	glEnable(GL_NORMALIZE);


	glEnable(GL_LINE_SMOOTH); 

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	glMaterialf(GL_FRONT, GL_SHININESS, sh);


	glShadeModel(GL_SMOOTH);
	int j = 0;
	for (int i = 0; i <= 1; i++)
	{
		double zero[] = { 0, 0, j };
		double bA[] = { -1, 7, j };
		double bB[] = { 2, 2, j };
		double bC[] = { 8, 3, j };
		double bD[] = { 6, -1, j };
		double bE[] = { 2, -1, j };
		double bF[] = { 0, -6, j };
		double bI[] = { -5, -3, j };

		glBegin(GL_TRIANGLE_FAN);
		glColor3d(0.2, 0.4, 0.7);
		glVertex3dv(zero);
		glVertex3dv(bA);
		glVertex3dv(bB);
		glVertex3dv(bC);
		glVertex3dv(bD);
		glVertex3dv(bE);
		glVertex3dv(bF);
		glVertex3dv(bI);
		glEnd();
		j = 3;
	}
	double zero[] = { 0, 0, 0 };
	double n1[] = { 0, 0, j };
	double n2[] = { -1, 7, j };
	double n3[] = { -1, 7, 0 };
	FindNormal(n1, n2, n3);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.1);
	glVertex3dv(zero);
	glVertex3dv(n1);
	glVertex3dv(n2);
	glVertex3dv(n3);
	glEnd();

	double n4[] = { 2, 2, 0 };
	double n5[] = { 2, 2, j };
	FindNormal(n2, n3, n4);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.2);
	glVertex3dv(n2);
	glVertex3dv(n3);
	glVertex3dv(n4);
	glVertex3dv(n5);
	glEnd();

	double n6[] = { 8, 3, 0 };
	double n7[] = { 8, 3, j };
	FindNormal(n4, n5, n7);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.1);
	glVertex3dv(n4);
	glVertex3dv(n5);
	glVertex3dv(n7);
	glVertex3dv(n6);
	glEnd();

	double n8[] = { 6, -1, 0 };
	double n9[] = { 6, -1, j };
	FindNormal(n7, n6, n8);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.2);
	glVertex3dv(n7);
	glVertex3dv(n6);
	glVertex3dv(n8);
	glVertex3dv(n9);
	glEnd();

	double n10[] = { 2, -1, 0 };
	double n11[] = { 2, -1, j };
	FindNormal(n8, n9, n11);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.1);
	glVertex3dv(n8);
	glVertex3dv(n9);
	glVertex3dv(n11);
	glVertex3dv(n10);
	glEnd();

	double n12[] = { 0, -6, 0 };
	double n13[] = { 0, -6, j };
	FindNormal(n11, n10, n12);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.2);
	glVertex3dv(n11);
	glVertex3dv(n10);
	glVertex3dv(n12);
	glVertex3dv(n13);
	glEnd();

	double n14[] = { -5, -3, 0 };
	double n15[] = { -5, -3, j };
	FindNormal(n12, n13, n15);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.1);
	glVertex3dv(n12);
	glVertex3dv(n13);
	glVertex3dv(n15);
	glVertex3dv(n14);
	glEnd();

	FindNormal(n1, n15, n14);
	glBegin(GL_QUADS);
	glColor3d(0.4, 0.1, 0.2);
	glVertex3dv(zero);
	glVertex3dv(n1);
	glVertex3dv(n15);
	glVertex3dv(n14);
	glEnd();


	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();			    
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}