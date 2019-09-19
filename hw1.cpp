/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: aswaroop
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

//Global variables
BasicPipelineProgram objPipelineProg; 
OpenGLMatrix openGLMatrix;
GLfloat heightmapHt;
GLint program_ID, h_modelViewMatrix, h_ProjectionMatrix;
float imgwidth, imgheight, m[16], p[16];
string display_mode = "pointMode";

GLuint pointmode_vao, pointmode_vbo;
GLuint wiremode_vao, wiremode_vbo;
GLuint solidmode_vao, solidmode_vbo;
GLuint wsmode_vao, wsmode_vbo;
GLuint indexbuffer;

std::vector<GLfloat> vec_pVertices, vec_pColors;
std::vector<GLfloat> vec_lVertices, vec_lColors;
std::vector<GLfloat> vec_tVertices, vec_tColors;
std::vector<GLfloat> vec_wsVertices, vec_wsColors;
std::vector<GLuint> indices;

int counter = 0, index=0;
const char* screenshotloc = "animation/";
string screenshotname, ss;
char s[3];

float scaleFactor = 0.025f;

#pragma region point mode 

void calcPointHeightField()
{
	//positioning the image at center by extending by half towards the negative axis
	for (int i = -imgwidth / 2; i < imgwidth / 2; i++) {
		for (int j = -imgheight / 2; j < imgheight / 2; j++) {

			//populate the 'z' vertex
			heightmapHt = scaleFactor * heightmapImage->getPixel(i + imgwidth / 2, j + imgheight / 2, 0);

			//initialise point vertices
			GLfloat vertices[] = { (float)i , heightmapHt , -(float)j };
			vec_pVertices.insert(vec_pVertices.end(), vertices, vertices + 3);
			
			//initialising the indexes for drawing using glDrawElements
			indices.insert(indices.end(), index);
			index++;

			//initialise color for point vertices
			GLfloat pointClr[] = { 0.0f, 1.0f, 1.0f, 1.0f };
			vec_pColors.insert(vec_pColors.end(), pointClr, pointClr + 4);

		}
	}
}

void initPointBuffers()
{
	glGenVertexArrays(1, &pointmode_vao);
	glBindVertexArray(pointmode_vao);

	glGenBuffers(1, &pointmode_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pointmode_vbo);
	glBufferData(GL_ARRAY_BUFFER, (vec_pVertices.size() + vec_pColors.size()) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vec_pVertices.size() * sizeof(GLfloat), &vec_pVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vec_pVertices.size() * sizeof(GLfloat), vec_pColors.size() * sizeof(GLfloat), &vec_pColors[0]);

	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
}

#pragma endregion point mode

#pragma region wireframe mode

void calcLineHeightField()
{
	GLfloat curr_color[4];
	GLfloat abv_color[4];
	GLfloat diag_color[4];
	GLfloat next_color[4];
	//color for wireframe + solid mode
	GLfloat wsColor[] = { 0.901 , 0.29 , 0.0, 1.0 };

	for (int i = -imgheight / 2; i < imgheight / 2; i++) {
		for (int j = -imgwidth / 2; j < imgwidth / 2; j++) {

			//populate the current pixel
			GLfloat pixel = (float)heightmapImage->getPixel(i + imgheight / 2, j + imgwidth / 2, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat curr[] = { (float)i, heightmapHt, (float)-j };
			
			//dividing the pixel value by 255 to get color value
			GLfloat color = pixel / 255.0f;
			if (color > 0.5)
			{
				curr_color[0] = 0.301; curr_color[1] = 0.53; curr_color[2] = 1.0; curr_color[3] = 1.0;
			}
			else
			{
				curr_color[0] = 0.0; curr_color[1] = 0.0; curr_color[2] = 1.0; curr_color[3] = 1.0;
			}

			//populate the pixel diagonal to current pixel
			pixel = heightmapImage->getPixel(i + imgwidth / 2 + 1, j + imgheight / 2 + 1, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat diagcurr[] = { (float)i + 1, heightmapHt, (float)-(j + 1) };
			color = pixel / 255.0f;
			if (color > 0.5)
			{
				diag_color[0] = 0.301; diag_color[1] = 0.53; diag_color[2] = 1.0; diag_color[3] = 1.0;
			}
			else
			{
				diag_color[0] = 0.0; diag_color[1] = 0.0; diag_color[2] = 1.0; diag_color[3] = 1.0;
			}

			//populate the pixel next to current pixel
			pixel = heightmapImage->getPixel(i + imgwidth / 2, j + imgheight / 2 + 1, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat nextcurr[] = { (float)i, heightmapHt, (float)-(j + 1) };
			color = pixel / 255.0f;
			if (color > 0.5)
			{
				color = 1.0;
				next_color[0] = 0.301; next_color[1] = 0.53; next_color[2] = 1.0; next_color[3] = 1.0;
			}
			else
			{
				next_color[0] = 0.0; next_color[1] = 0.0; next_color[2] = 1.0; next_color[3] = 1.0;
			}

			//if it is the corner most pixel draw the border line
			// 
			// |
			if (j == -imgwidth / 2)
			{
				//populate the pixel above current pixel to draw border line
				pixel = heightmapImage->getPixel(i + imgheight / 2 + 1, j + imgwidth / 2, 0);
				heightmapHt = scaleFactor * pixel;
				GLfloat abvcurr[] = { (float)i + 1, heightmapHt, (float)-j };
				color = pixel / 255.0f;
				if (color > 0.5)
				{
					color = 1.0;
					abv_color[0] = 0.301; abv_color[1] = 0.53; abv_color[2] = 1.0; abv_color[3] = 1.0;
				}
				else
				{
					abv_color[0] = 0.0; abv_color[1] = 0.0; abv_color[2] = 1.0; abv_color[3] = 1.0;
				}

				//insert the line vertices in the vector
				vec_lVertices.insert(vec_lVertices.end(), abvcurr, abvcurr + 3);
				vec_lVertices.insert(vec_lVertices.end(), curr, curr + 3);

				//the same will be used for wireframe + solid mode
				vec_wsVertices.insert(vec_wsVertices.end(), abvcurr, abvcurr + 3);
				vec_wsVertices.insert(vec_wsVertices.end(), curr, curr + 3);

				//initialise color for both
				vec_lColors.insert(vec_lColors.end(), abv_color, abv_color + 4);
				vec_lColors.insert(vec_lColors.end(), curr_color, curr_color + 4);

				vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
				vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
			}

			//draw the bottom and side edge
			// 
			// | __| __| __|
			vec_lVertices.insert(vec_lVertices.end(), curr, curr + 3);
			vec_lVertices.insert(vec_lVertices.end(), nextcurr, nextcurr + 3);
			vec_lVertices.insert(vec_lVertices.end(), nextcurr, nextcurr + 3);
			vec_lVertices.insert(vec_lVertices.end(), diagcurr, diagcurr + 3);

			//initialise the same for wireframe + solid mode
			vec_wsVertices.insert(vec_wsVertices.end(), curr, curr + 3);
			vec_wsVertices.insert(vec_wsVertices.end(), nextcurr, nextcurr + 3);
			vec_wsVertices.insert(vec_wsVertices.end(), curr, curr + 3);
			vec_wsVertices.insert(vec_wsVertices.end(), diagcurr, diagcurr + 3);

			//initialse the color
			vec_lColors.insert(vec_lColors.end(), curr_color, curr_color + 4);
			vec_lColors.insert(vec_lColors.end(), next_color, next_color + 4);
			vec_lColors.insert(vec_lColors.end(), next_color, next_color + 4);
			vec_lColors.insert(vec_lColors.end(), diag_color, diag_color + 4);

			vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
			vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
			vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
			vec_wsColors.insert(vec_wsColors.end(), wsColor, wsColor + 4);
		}
	}
}

void initLineBuffers()
{
	glGenVertexArrays(1, &wiremode_vao);
	glBindVertexArray(wiremode_vao);

	glGenBuffers(1, &wiremode_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, wiremode_vbo);
	glBufferData(GL_ARRAY_BUFFER, (vec_lVertices.size() + vec_lColors.size()) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vec_lVertices.size() * sizeof(GLfloat), &vec_lVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vec_lVertices.size() * sizeof(GLfloat), vec_lColors.size() * sizeof(GLfloat), &vec_lColors[0]);
}

#pragma endregion wireframe mode

#pragma region solid mode

void calcTriangleHeightfield()
{
	for (int i = -imgheight / 2; i < imgheight / 2 - 1; i++) {
		for (int j = -imgwidth / 2; j < imgwidth / 2 - 1; j++) {

			//populate current pixel and surrounding pixels
			GLfloat pixel = (float)heightmapImage->getPixel(i + imgheight / 2, j + imgwidth / 2, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat curr[] = { (float)i, heightmapHt, (float)-j };
			GLfloat color = pixel / 255.0f;
			GLfloat curr_color[] = {color , color, color, 1.0};
			
			pixel = heightmapImage->getPixel(i + imgheight / 2 + 1, j + imgwidth / 2, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat abvcurr[] = { (float)i + 1, heightmapHt, (float)-j };
			color = pixel / 255.0f;
			GLfloat abv_color[] = { color , color, color, 1.0 };

			pixel = heightmapImage->getPixel(i + imgwidth / 2 + 1, j + imgheight / 2 + 1, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat diagcurr[] = { (float)i + 1, heightmapHt, (float)-(j + 1) };
			color = pixel / 255.0f;
			GLfloat diag_color[] = { color , color, color, 1.0 };

			pixel = heightmapImage->getPixel(i + imgwidth / 2, j + imgheight / 2 + 1, 0);
			heightmapHt = scaleFactor * pixel;
			GLfloat nextcurr[] = { (float)i, heightmapHt, (float)-(j + 1) };
			color = pixel / 255.0f;
			GLfloat next_color[] = { color , color, color, 1.0 };

			// triangles will be drawn by dividing a square into two half triangles
			//     _
			//  |\\ |
			//  |_\\|

			vec_tVertices.insert(vec_tVertices.end(), curr, curr + 3);
			vec_tVertices.insert(vec_tVertices.end(), nextcurr, nextcurr + 3);
			vec_tVertices.insert(vec_tVertices.end(), abvcurr, abvcurr + 3);
			vec_tVertices.insert(vec_tVertices.end(), abvcurr, abvcurr + 3);
			vec_tVertices.insert(vec_tVertices.end(), nextcurr, nextcurr + 3);
			vec_tVertices.insert(vec_tVertices.end(), diagcurr, diagcurr + 3);
					
			vec_tColors.insert(vec_tColors.end(), curr_color, curr_color + 4);
			vec_tColors.insert(vec_tColors.end(), next_color, next_color + 4);
			vec_tColors.insert(vec_tColors.end(), abv_color, abv_color + 4);
			vec_tColors.insert(vec_tColors.end(), abv_color, abv_color + 4);
			vec_tColors.insert(vec_tColors.end(), next_color, next_color + 4);
			vec_tColors.insert(vec_tColors.end(), diag_color, diag_color + 4); 
						
		}
	}

}

void initTriangleBuffers()
{
	glGenVertexArrays(1, &solidmode_vao);
	glBindVertexArray(solidmode_vao);

	glGenBuffers(1, &solidmode_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, solidmode_vbo);
	glBufferData(GL_ARRAY_BUFFER, (vec_tVertices.size() + vec_tColors.size()) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vec_tVertices.size() * sizeof(GLfloat), &vec_tVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vec_tVertices.size() * sizeof(GLfloat), vec_tColors.size() * sizeof(GLfloat), &vec_tColors[0]);
}

#pragma endregion solid mode

#pragma region wireframe + solid mode

void initWSBuffers()
{
	glGenVertexArrays(1, &wsmode_vao);
	glBindVertexArray(wsmode_vao);

	glGenBuffers(1, &wsmode_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, wsmode_vbo);
	glBufferData(GL_ARRAY_BUFFER, (vec_wsVertices.size() + vec_wsColors.size()) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vec_wsVertices.size() * sizeof(GLfloat), &vec_wsVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vec_wsVertices.size() * sizeof(GLfloat), vec_wsColors.size() * sizeof(GLfloat), &vec_wsColors[0]);
}

#pragma endregion wireframe + solid mode


// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	objPipelineProg.Bind();

	//model view
	openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
	openGLMatrix.LoadIdentity();
	openGLMatrix.LookAt(200,200,256, 0, 0, 0, 0, 1, 0);
	
	//enable transformations on the rednering
	openGLMatrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	openGLMatrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	openGLMatrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	openGLMatrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	openGLMatrix.Scale(landScale[0], landScale[1], landScale[2]);
	
	openGLMatrix.GetMatrix(m);
	objPipelineProg.SetModelViewMatrix(m);

	//projection
	openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
	openGLMatrix.LoadIdentity();
	openGLMatrix.Perspective(60, 1.5, 0.01, 1000.0);
	openGLMatrix.GetMatrix(p);
	objPipelineProg.SetProjectionMatrix(p);

	openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);

	if (display_mode == "pointMode")
	{
		//calcPointHeightField();
		//initPointBuffers();
		glBindBuffer(GL_ARRAY_BUFFER, pointmode_vbo);
		GLuint loc = glGetAttribLocation(program_ID, "position");
		glEnableVertexAttribArray(loc);
		const void * offset = (const void*)0;
		GLsizei stride = 0;
		GLboolean normalized = GL_FALSE;
		glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

		loc = glGetAttribLocation(program_ID, "color");
		glEnableVertexAttribArray(loc);
		offset = (const void*)(vec_pVertices.size() * sizeof(GL_FLOAT));
		glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);

		int count = vec_pVertices.size() / 3;
		//glDrawArrays(GL_POINTS, 0, count);
		//using glDrawElements
		glDrawElements(GL_POINTS, count, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}

	if (display_mode == "wireframeMode")
	{
		//calcLineHeightField();
		//initLineBuffers();
		glBindBuffer(GL_ARRAY_BUFFER, wiremode_vbo);
		GLuint loc = glGetAttribLocation(program_ID, "position");
		glEnableVertexAttribArray(loc);
		const void * offset = (const void*)0;
		GLsizei stride = 0;
		GLboolean normalized = GL_FALSE;
		glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

		loc = glGetAttribLocation(program_ID, "color");
		glEnableVertexAttribArray(loc);
		offset = (const void*)(vec_lVertices.size() * sizeof(GL_FLOAT));
		glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

		int count = vec_lVertices.size() / 3;
		glDrawArrays(GL_LINES, 0, count);

		glBindVertexArray(0);
	}

	if (display_mode == "solidMode")
	{
		glBindBuffer(GL_ARRAY_BUFFER, solidmode_vbo);
		GLuint loc = glGetAttribLocation(program_ID, "position");
		glEnableVertexAttribArray(loc);
		const void * offset = (const void*)0;
		GLsizei stride = 0;
		GLboolean normalized = GL_FALSE;
		glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

		loc = glGetAttribLocation(program_ID, "color");
		glEnableVertexAttribArray(loc);
		offset = (const void*)(vec_tVertices.size() * sizeof(GL_FLOAT));
		glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

		int count = vec_tVertices.size() / 3;
		glDrawArrays(GL_TRIANGLES, 0, count);

		glBindVertexArray(0);
	}

	if (display_mode == "combined")
	{
		//render the wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1.0f, 1.0f);

		glBindBuffer(GL_ARRAY_BUFFER, wsmode_vbo);
		GLuint loc = glGetAttribLocation(program_ID, "position");
		glEnableVertexAttribArray(loc);
		const void * offset = (const void*)0;
		GLsizei stride = 0;
		GLboolean normalized = GL_FALSE;
		glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

		loc = glGetAttribLocation(program_ID, "color");
		glEnableVertexAttribArray(loc);
		offset = (const void*)(vec_wsVertices.size() * sizeof(GL_FLOAT));
		glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

		int count = vec_wsVertices.size() / 3;
		glDrawArrays(GL_LINES, 0, count);

		//reset the offset and render the solid mode
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		glBindBuffer(GL_ARRAY_BUFFER, solidmode_vbo);
		loc = glGetAttribLocation(program_ID, "position");
		glEnableVertexAttribArray(loc);
		offset = (const void*)0;
		glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);

		loc = glGetAttribLocation(program_ID, "color");
		glEnableVertexAttribArray(loc);
		offset = (const void*)(vec_tVertices.size() * sizeof(GL_FLOAT));
		glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

		count = vec_tVertices.size() / 3;
		glDrawArrays(GL_TRIANGLES, 0, count);

		glBindVertexArray(0);

	}
	//glBindVertexArray(0);
	glutSwapBuffers();

}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)
	
  // make the screen update 
	if(counter < 301)
	{
		//rotate for first 50 frames
		if (counter <= 50)
		{
			landRotate[0] += 1.0 * 2.0;
			landRotate[1] += 1.0 * 2.0;
		}
		//change the mode and rotate again
		else if (counter > 50 && counter <= 100)
		{
			display_mode = "solidMode";
			landRotate[0] += 1.0 * 2.0;
			landRotate[1] += 1.0 * 2.0;
		}
		//zoom in and zoom out in the wireframe mode for 100 frames
		else if (counter > 100 && counter <= 150)
		{
			display_mode = "wireframeMode";
			landScale[0] += 0.1f;
			landScale[1] += 0.1f;
			landScale[2] += 0.1f;
		}
		else if (counter > 150 && counter <= 200)
		{
			landScale[0] -= 0.1f;
			landScale[1] -= 0.1f;
			landScale[2] -= 0.1f;
		}
		//rotate in point mode
		else if (counter > 200 && counter <= 250)
		{
			display_mode = "pointMode";
			landRotate[0] -= 1.0 * 2.0;
			landRotate[1] -= 1.0 * 2.0;
		}
		else if (counter > 250 && counter <= 300)
		{
			landRotate[0] -= 1.0 * 2.0;
			landRotate[1] -= 1.0 * 2.0;
		}
	//save screenshot
	sprintf(s, "%03d", counter);
	ss = s;
	screenshotname = screenshotloc + ss + ".jpg";
	saveScreenshot(screenshotname.c_str());
	counter++;
	}

  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // setup perspective matrix...
  openGLMatrix.SetMatrixMode(OpenGLMatrix::Projection);
  openGLMatrix.LoadIdentity();
  openGLMatrix.Perspective(60, 1.5, 0.01, 1000.0);
  openGLMatrix.SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.1f;
        landTranslate[1] -= mousePosDelta[1] * 0.1f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.1f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.1f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.1f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.1f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;

	case 'p':
		cout << "point mode" << endl;
		display_mode = "pointMode";
		break;
	case 'w':
		cout << "wireframe mode" << endl;
		display_mode = "wireframeMode";
		break;
	case 's':
		cout << "solid mode" << endl;
		display_mode = "solidMode";
		break;
	case 'c':
		cout << "wireframe and solid mode combined" << endl;
		display_mode = "combined";
		break;
  }
}

void initScene(int argc, char *argv[])
{
  // load the image from a jpeg disk file to main memory
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }

  scaleFactor *= (float)heightmapImage->getHeight() / 100;

  imgwidth = (float)heightmapImage->getWidth();
  imgheight = (float)heightmapImage->getHeight();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);

  // do additional initialization here...

  objPipelineProg.Init("../openGLHelper-starterCode");
  objPipelineProg.Bind();

  program_ID = objPipelineProg.GetProgramHandle();
  glUseProgram(program_ID);

 // h_modelViewMatrix = glGetUniformLocation(program_ID, "modelViewMatrix");
 // h_ProjectionMatrix = glGetUniformLocation(program_ID, "projectionMatrix");

  calcPointHeightField();
  initPointBuffers();
  calcLineHeightField();
  initLineBuffers();
  calcTriangleHeightfield();
  initTriangleBuffers();
  //calcWSHeightField();
  initWSBuffers();

}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


