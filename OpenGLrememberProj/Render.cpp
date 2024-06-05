#include "Render.h"
#include <windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"
#include "MyOGL.h"
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include "MyShaders.h"
#include "ObjLoader.h"
#include "GUItextRectangle.h"
#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;
bool oko = true;
bool shine = true;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile* model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 10);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

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



}  camera;   //создаем объект камеры

//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(-15, 15, 15);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void DrawLightGhismo()
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);

		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		/*Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();*/

		if (OpenGL::isKeyPressed('G') && oko)
		{
			PUSH;
			glRotated(76,0,0,1);
			glBegin(GL_QUADS);
			glColor4f(1, 0, 0, 0.3); 
			glVertex3d(0.1, 0, 10.4);//A
			glVertex3d(-0.1, 0, 10.4);//B
			glVertex3d(-0.1, 0, 10.7);//C
			glVertex3d(0.1, 0, 10.7);//D
			glEnd();
			POP;

			glBegin(GL_QUADS);
			glColor4f(1, 0,0, 0.3);
			glVertex3d(0.1, 0, 10.4);//A
			glVertex3d(-0.1, 0, 10.4);//B
			glVertex3d(pos.X() - 3, pos.Y() + 3, 0);//B1
			glVertex3d(pos.X() + 3, pos.Y() + 3, 0);//A1
			glEnd();

			glBegin(GL_QUADS);
			glColor4f(1, 0, 0, 0.3);
			glVertex3d(0.1, 0, 10.4);//A
			glVertex3d(pos.X() + 3, pos.Y() + 3, 0);//A1
			glVertex3d(pos.X() + 3, pos.Y() - 3, 0);//D1
			glVertex3d(0.1, 0, 10.7);//D
			glEnd();

			glBegin(GL_QUADS);
			glColor4f(1, 0, 0, 0.3);
			glVertex3d(pos.X() - 3, pos.Y() - 3, 0);//C1
			glVertex3d(pos.X() + 3, pos.Y() - 3, 0);//D1
			glVertex3d(0.1, 0, 10.7);//D
			glVertex3d(-0.1, 0, 10.7);//C
			glEnd();

			glBegin(GL_QUADS);
			glColor4f(1, 0, 0, 0.3);
			glVertex3d(pos.X() - 3, pos.Y() + 3, 0);//B1
			glVertex3d(pos.X() - 3, pos.Y() - 3, 0);//C1
			glVertex3d(-0.1, 0, 10.7);//C
			glVertex3d(-0.1, 0, 10.4);//B
			glEnd();

			glBegin(GL_QUADS);
			glColor4f(1, 0, 0, 0.3);
			glVertex3d(pos.X()+3, pos.Y()+3, 0);//A1
			glVertex3d(pos.X()-3, pos.Y()+3, 0);//B1
			glVertex3d(pos.X()-3, pos.Y()-3, 0);//C1
			glVertex3d(pos.X()+3, pos.Y()-3, 0);//D1
			glEnd();

			glBegin(GL_LINE_LOOP);
			glColor3f(1, 1, 1);
			glVertex3d(pos.X() + 3, pos.Y() + 3, 0);//A1
			glVertex3d(pos.X() - 3, pos.Y() + 3, 0);//B1
			glVertex3d(pos.X() - 3, pos.Y() - 3, 0);//C1
			glVertex3d(pos.X() + 3, pos.Y() - 3, 0);//D1
			glEnd();
		}
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1.};

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света


//старые координаты мыши
int mouseX = 0, mouseY = 0;

float offsetX = 0, offsetY = 0;
float zoom = 1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0 * dx / ogl->getWidth() / zoom;
		offsetY += 1.0 * dy / ogl->getHeight() / zoom;
	}

	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y, 60, ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);

		
	}


	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL* ogl, int delta)
{
	float _tmpZ = delta * 0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2 * zoom * _tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;
}

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL* ogl, int key)
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

	if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
	{
		Time = 0;
	}

	if (key == 'E')
	{
		oko = !oko;
	}

	if (key == 'W')
	{
		shine = !shine;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel, objModel1, objModel2, objModel3;

Texture objModelTex, objModelTex1, objModelTex2, objModelTex3, objModelTex4;

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{

	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);




	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем


	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем



	//так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	// создающимися во время компиляции, я переименовал модели в *.obj_m
	loadModel("models\\untitled1.obj_m", &objModel);
	loadModel("models\\untitled3.obj_m", &objModel2);
	loadModel("models\\untitled4.obj_m", &objModel3);


	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\untitled2.obj_m", &objModel1);
	objModelTex.loadTextureFromFile("textures//2.bmp");
	objModelTex.bindTexture();
	objModelTex1.loadTextureFromFile("textures//1.bmp");
	objModelTex1.bindTexture();
	objModelTex2.loadTextureFromFile("textures//3.bmp");
	objModelTex2.bindTexture();
	objModelTex3.loadTextureFromFile("textures//4.bmp");
	objModelTex3.bindTexture();
	objModelTex4.loadTextureFromFile("textures//5.bmp");
	objModelTex4.bindTexture();

	tick_n = GetTickCount();
	tick_o = tick_n;

	//rec.setSize(300, 100);
	//rec.setPosition(10, ogl->getHeight() - 100 - 10);
	//rec.setText("T - вкл/выкл текстур\nL - вкл/выкл освещение\nF - Свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали", 0, 0, 0);


}

void Chas(float h) {
	glBegin(GL_QUADS);
	glColor3d(0.6, 0.4,0);
	glVertex3f(1, 0, h + 15);
	glVertex3f(1, 0, h + 16);
	glVertex3f(0, 0, h + 16);
	glVertex3f(0, 0, h + 15);
	glEnd();
}

double h = 0.1, h1 = h;


void Render(OpenGL* ogl)
{

	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);
	
	if (shine) {
		PUSH;
		glDisable(GL_LIGHTING);
		glRotatef(54, 0, 0, 1);
		glTranslated(0.4, 0.4, 2);
		glScaled(0.3, 0.3, 0.3);
		Chas(h1);
		POP;

		PUSH;
		glDisable(GL_LIGHTING);
		glRotatef(45, 0, 0, 1);
		glTranslated(0.4, -0.4, 2);
		glScaled(0.3, 0.3, 0.3);
		Chas(h1 + 3);
		POP;

		PUSH;
		glDisable(GL_LIGHTING);
		glRotatef(75, 0, 0, 1);
		glTranslated(-0.4, -0.4, 2);
		glScaled(0.3, 0.3, 0.3);
		Chas(h1 - 2);
		POP;

		PUSH;
		glDisable(GL_LIGHTING);
		glRotatef(23, 0, 0, 1);
		glTranslated(-0.4, 0.4, 2);
		glScaled(0.3, 0.3, 0.3);
		Chas(h1 + 2);
		POP;
	}

	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.X(), light.pos.Y(), light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	s[0].UseShader();
	int l = glGetUniformLocationARB(s[0].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	objModelTex.bindTexture();
	objModel.DrawObj();
	glPopMatrix();

	s[0].UseShader();
	l = glGetUniformLocationARB(s[0].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	objModelTex1.bindTexture();
	objModel1.DrawObj();
	glPopMatrix();

	

	Shader::DontUseShaders();
	glDisable(GL_LIGHTING);
	s[1].UseShader();
	l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glScaled(0.6,0.6,0.6);
	objModelTex2.bindTexture();
	objModel2.DrawObj();
	glPopMatrix();

	{
		Shader::DontUseShaders();
		glDisable(GL_LIGHTING);
		s[1].UseShader();
		l = glGetUniformLocationARB(s[1].program, "tex");
		glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		glScaled(0.01, 0.01, 0.01);
		objModelTex3.bindTexture();
		objModel2.DrawObj();
		glPopMatrix();
	}

	Shader::DontUseShaders();
	s[1].UseShader();
	l = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	glPushMatrix();
	glBegin(GL_QUADS);
	//glColor3d(1,1,1);
	objModelTex3.bindTexture();
	glTexCoord2d(1, 1);
	glVertex3d(60, 60, 0);
	glTexCoord2d(1, 0);
	glVertex3d(60, -60, 0);
	glTexCoord2d(0, 0);
	glVertex3d(-60, -60, 0);
	glTexCoord2d(0, 1);
	glVertex3d(-60, 60, 0);
	glEnd();
	glPopMatrix();

	if (oko) {
		s[1].UseShader();
		l = glGetUniformLocationARB(s[1].program, "tex");
		glUniform1iARB(l, 0);     //так как когда мы загружали текстуру грузили на GL_TEXTURE0
		glPushMatrix();
		glRotated(90, 1, 0, 0);
		objModelTex4.bindTexture();
		objModel3.DrawObj();
		glPopMatrix();
	}

	
	h1 += h;
	if (h1 > 15)
		h1 = 0;
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL* ogl)
{

	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);


	glActiveTexture(GL_TEXTURE0);
	rec.Draw();



	Shader::DontUseShaders();




}

void resizeEvent(OpenGL* ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

