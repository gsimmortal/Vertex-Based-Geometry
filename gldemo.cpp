
//
// Tutorial 04 - Solution
//

#include "stdafx.h"
#include <glew\glew.h>
#include <glew\wglew.h>
#include <freeglut\freeglut.h>
#include <CoreStructures\CoreStructures.h>
#include <CGImport\CGModel\CGModel.h>
#include <CGImport\Importers\CGImporters.h>
#include "helper_functions.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "CGPrincipleAxes.h"
#include "CGTexturedQuad.h"
#include <random>


using namespace std;
using namespace CoreStructures;


#pragma region Scene variables and resources

// Variables needed to track where the mouse pointer is so we can determine which direction it's moving in
int								mouse_x, mouse_y;
bool							mDown = false;

GUClock							*mainClock = nullptr;

// Main scene resources
GUPivotCamera					*mainCamera = nullptr;
CGPrincipleAxes					*principleAxes = nullptr;
CGTexturedQuad					*texturedQuad = nullptr;
CGModel							*exampleModel = nullptr;

GLuint							basicShader;



#pragma endregion

#pragma region grid variables and resources

//=================================================
//================= grid globals ==================
//=================================================

bool debuggingGrid = false;//used for outputting debug info if needed
bool wireFrame = false;//used for toggling wireframe mode for the grid

float							funkyMath0 = 0.0f;//offset func
float							funkyMath1 = 0.0f;//colour func
float							funkyMath2 = 0.0f;//period for timing (up to 360)
float							funkyMath3 = 1.0f;//OTHER RANDOM STUFF FOR FUNKY MATH STUFF -- SMOOTHNESS????

float							sendTime = 0.0f;//used to send time to shader as float
float							sendSize = 0.0f;//used to send the diameter of the grid
float							sendBMPSize = 0.0;//used for uv mapping

const int gridSize = 512;//used in loops to initialise grid arrays
const int gridSizeSquared = gridSize * gridSize;
const int gridSizeM1 = gridSize - 1;

float vertexArray[gridSizeSquared * 4];//32*32 points *4 floats per point (x,y,z,w)
float colourArray[gridSizeSquared * 4];//32*32 points *4 floats per colour (r,g,b,a)

//no of triangles = ((verticeDiameter - 1) * 2) * (verticeDiameter - 1)
//     1922       == ((32-1)*2)*(32-1)            (62 * 31 = 1922)

int indiceArray[((gridSizeM1*2) * gridSizeM1) * 3];//1922 triangles *3 ints per triangle
int warpedLines[(gridSize * gridSizeM1) * 2];//31 lines per row, 32 rows 2 values per line


//=================================================
//============== grid vbo resources ===============
//=================================================

GLuint							gridVertexBuffer;
GLuint							gridColourBuffer;
GLuint							gridIndexBuffer;

GLuint							gridVAO;

//=================================================
//============== texturemap resources =============
//=================================================

//image filepath
const char*						filePath = "Resources/4.bmp";

//image/texture id storage
GLuint							heightMap;

//file pointer that recieves the file pointer given by fopen_s (safe file open function)
FILE*							*pointerToSafeFile;

//values set from texture image
unsigned char					BMPheader[54];//BMP files have 54 byte headers
unsigned int					dataPosition;//position in file where data begins
unsigned int					hmWidth = 0, hmHeight = 0;
unsigned int					imageSize = hmWidth * hmHeight;
//RGB DATA
unsigned char*					BMPimageData;

#pragma endregion


#pragma region Function Prototypes

void init(void); // Main scene initialisation function
void update(void); // Main scene update function
void display(void); // Main scene render function

// Event handling functions
void mouseButtonDown(int button_id, int state, int x, int y);
void mouseMove(int x, int y);
void mouseWheel(int wheel, int direction, int x, int y);
void keyDown(unsigned char key, int x, int y);
void closeWindow(void);

//=================================================
//================ grid functions =================
//=================================================
void initGridVertices(void);
void initGridColours(void);
void initGridIndices(void);

void initWarpingLines(void);

void initGridVAOVBO(void);

void updateTheFunkyMath(void);

void drawSurface(void);

//=================================================
//============== heightmap functions ==============
//=================================================
GLuint customBMPFileLoad(const char * filePath);

#pragma endregion



int _tmain(int argc, char* argv[]) {

	// Initialise GL Utility Toolkit and initialise COM so we can use the Windows Imaging Component (WIC) library
	glutInit(&argc, argv);
	CoInitialize(NULL);

	// Setup the OpenGL environment and initialise scene variables and resources
	init();

	// Setup and start the main clock
	mainClock = new GUClock();

	// Main event / render loop
	glutMainLoop();

	// Stop clock and report final timing data
	if (mainClock) {

		mainClock->stop();
		mainClock->reportTimingData();
		
		mainClock->release();
	}
	
	// Shut down COM and exit
	CoUninitialize();
	return 0;
}


#pragma region Initialisation, Update and Render

void init(void) {

	// Request an OpenGL 4.3 context with the Compatibility profile
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	// Setup OpenGL Display mode - include MSAA x4
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 16);

	// Setup window
	int windowWidth = 900;	//== changed ==
	int windowHeight = 600;	//== changed ==
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(64, 64);
	glutCreateWindow("Real-Time Rendering Techniques - CW1");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Register callback functions
	glutIdleFunc(update); // Main scene update function
	glutDisplayFunc(display); // Main render function
	glutKeyboardFunc(keyDown); // Key down handler
	glutMouseFunc(mouseButtonDown); // Mouse button handler
	glutMotionFunc(mouseMove); // Mouse move handler
	glutMouseWheelFunc(mouseWheel); // Mouse wheel event handler
	glutCloseFunc(closeWindow); // Main resource cleanup handler

	// Initialise GLEW library
	GLenum err = glewInit();

	// Ensure GLEW was initialised successfully before proceeding
	if (err == GLEW_OK) 
	{
		cout << "GLEW initialised okay\n";
	} 
	else
	{
		cout << "GLEW could not be initialised\n";
		throw;
	}
	
	// Example query OpenGL state (get version number)
	reportExtensions();
	reportContextVersion();


	// ===== Initiaise scene resources and variables =====
	// Initialise OpenGL
	wglSwapIntervalEXT(0);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//=================================================
	//======== grid changes to OpenGL setup ===========
	//=================================================

	//OPENGL DEFAULTS TO CALCULATING FACES COUNTER CLOCKWISE
	//I WORK OUT MY INDICES CLOCKWISE SO CHANGE FRONT FACE CALCULATION TO CLOCKWISE
	glFrontFace(GL_CW);

	// Setup main camera
	float viewportAspect = (float)windowWidth / (float)windowHeight;
	//							   theta , phi ,radius, fov  ,     aspect    ,nearplane
	mainCamera = new GUPivotCamera(315.0f, 0.0f, 32.0f, 55.0f, viewportAspect, 0.1f);
	principleAxes = new CGPrincipleAxes();

	//=================================================
	//============ grid initialization ================
	//=================================================
	initGridVertices();
	initGridColours();
	initGridIndices();

	initWarpingLines();

	initGridVAOVBO();

	// Load example shaders
	err = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\basic_shader.vs"),
		string("Resources\\Shaders\\basic_shader.fs"),
		&basicShader);
}


// Main scene update function (called by FreeGLUT's main event loop every frame) 
void update(void) {

	// Update clock
	mainClock->tick();

	// Update offsetScale
	//static float phase = 0.0f;
	//phase += mainClock->gameTimeDelta() * gu_pi; // Want a rate of 1/2 oscillation every second
	//offsetScale = sinf(phase);


	//=================================================
	//========== update grid funkyMath :) =============
	//=================================================
	if (funkyMath0 == 3.0)
	{
		funkyMath2 += 0.001f;
		if (funkyMath2 >= 360) funkyMath2 = 0.0f;
	}
	else 
	{
		funkyMath2 += 0.005f;
		if (funkyMath2 >= 360) funkyMath2 = 0.0f;
	}

	//=================================================
	//========= update shader time value :) ===========
	//=================================================
	sendTime += mainClock->gameTimeDelta() * gu_pi;
	

	// Redraw the screen
	display();

	// Update the window title to show current frames-per-second and seconds-per-frame data
	char timingString[256];
	sprintf_s(timingString, 256, "Real-Time Rendering Demo. Average fps: %.0f; Average spf: %f", mainClock->averageFPS(), mainClock->averageSPF() / 1000.0f);
	glutSetWindowTitle(timingString);
}


// Main scene rendering function
void display(void) {

	// Clear the screen
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set viewport to the client area of the current window
	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		
	// Get the camera view and projection matrix transforms
	GUMatrix4 viewMatrix = mainCamera->viewTransform();
	GUMatrix4 projMatrix = mainCamera->projectionTransform();

	// Render principle axes
	if (principleAxes) principleAxes->render(projMatrix * viewMatrix);
	// Use basic shader for rendering
	glUseProgram(basicShader);

	// Get the locations of the viewMatrix, projectionMatrix and offsetScale uniform variables in the shader
	static GLint viewMatrixLocation = glGetUniformLocation(basicShader, "viewMatrix");
	static GLint projMatrixLocation = glGetUniformLocation(basicShader, "projectionMatrix");
	//********************************** static GLint offsetScaleLocation = glGetUniformLocation(basicShader, "offsetScale");

	// Set the matrices in the shader with the camera view and projection matrices obtained above
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, (GLfloat*)&viewMatrix);
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, (GLfloat*)&projMatrix);
	//********************************** glUniform1f(offsetScaleLocation, offsetScale);
	
	drawSurface();

	glutSwapBuffers();

}

#pragma endregion



#pragma region Event handling functions

void mouseButtonDown(int button_id, int state, int x, int y) {

	if (button_id == GLUT_LEFT_BUTTON) {

		if (state == GLUT_DOWN) {

			mouse_x = x;
			mouse_y = y;

			mDown = true;

		}
		else if (state == GLUT_UP) {

			mDown = false;
		}
	}
}

void mouseMove(int x, int y) {

	int dx = x - mouse_x;
	int dy = y - mouse_y;

	if (mainCamera)
		mainCamera->transformCamera((float)-dy, (float)-dx, 0.0f);
		
	mouse_x = x;
	mouse_y = y;
}

void mouseWheel(int wheel, int direction, int x, int y) {

	if (mainCamera) {

		if (direction<0)
			mainCamera->scaleCameraRadius(1.1f);
		else if (direction>0)
			mainCamera->scaleCameraRadius(0.9f);
	}
}

void keyDown(unsigned char key, int x, int y) {

	// Toggle fullscreen (This does not adjust the display mode however!)
	if (key == 'f' || key == 'F') glutFullScreenToggle();


	//=================================================
	//============== grid inputs setup ================
	//=================================================
	if (key == 'w' || key == 'W')
	{
		if (wireFrame) wireFrame = false;
		else wireFrame = true;
	}

	if (key == 'v' || key == 'V')
	{
		funkyMath0 += 1.0f;
		if (funkyMath0 > 6.0) funkyMath0 = 0.0f;
		cout << "Cycling 'Y' offset in Vertex Shader: " << funkyMath0 << endl;
	}

	if (key == 'c' || key == 'C')
	{
		funkyMath1 += 0.1f;
		if (funkyMath1 >= 0.6) funkyMath1 = 0.0f;
		cout << "Cycling colour profile in Fragment Shader: " << funkyMath1 << endl;
	}

	if (funkyMath0 == 3.0)
	{
		if (key == '=' || key == '+')
		{
			funkyMath3 -= 0.1;
			if (funkyMath3 <= 0.1)funkyMath3 = 0.1f;
			cout << "Increasing smoothness, length between subdivisions: " << funkyMath3 << endl;
		}
		if (key == '-' || key == '_')
		{
			funkyMath3 += 0.1;
			if (funkyMath3 >= 4.0)funkyMath3 = 4.0f;
			cout << "Decreasing smoothness, length between subdivisions: " << funkyMath3 << endl;
		}
	}

}

void closeWindow(void) {

	// Clean-up scene resources

	if (mainCamera)
		mainCamera->release();

	if (principleAxes)
		principleAxes->release();

	if (texturedQuad)
		texturedQuad->release();

	if (exampleModel)
		exampleModel->release();
}

#pragma endregion


#pragma region gridFunctions

void initGridVertices(void)
{
	//=================================================
	//============= grid vertice setup ================
	//=================================================

	for (int x = 0; x < (gridSizeSquared*4); x++)	//4096= 32*32 points * 4 floats per point
	{
		vertexArray[x] = ((floor(x / 4)) - (floor((x / 4) / gridSize)) * gridSize) - (gridSize/2);	//works out x co-ord depending on what float its on
		x++;
		vertexArray[x] = 0.0f;		//set y = 0.0f
		x++;
		vertexArray[x] = (floor((x / 4) / gridSize)) - (gridSize/2);	//works out z co-ord depending on what float its on
		x++;
		vertexArray[x] = 1.0f;		//set 'weight' to 1.0f
	}

	//debug output to see if its setting up the vertices correctly
	cout << endl << "vertice steup done origin at: " << vertexArray[0] << ", " << vertexArray[1] << ", " << vertexArray[2] << endl;
}

void initGridColours(void)
{
	//=================================================
	//============== grid colour setup ================
	//=================================================
	for (int x = 0; x < gridSizeSquared*4; x++)
	{
		colourArray[x] = 1.0f;
	}

	//debug output to see if its setting up the colours correctly
	cout << endl << "colour steup done origin col: " << colourArray[0] << ", " << colourArray[1] << ", " << colourArray[2] << endl;
}

void initGridIndices(void)
{
	//=================================================
	//============== grid indice setup ================
	//=================================================
	//works out indices in a clockwise direction.

	int indiceCalc = 0;//holds the number of the vertice it needs to link to
	int onInt = 0;//holds the int its on

	for (int x = 0; x < gridSizeM1; x++){
		for (int y = 0; y < gridSizeM1; y++)
		{
			//maths for working out two points (1 square)
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc += 1;//point to next vertice
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc += gridSizeM1;//point to 1 row down 1 back vertice +(size-1)
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc -= gridSizeM1;
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc += gridSize;//point to next vertice
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc -= 1;//point to 1 row up vertice -size
			indiceArray[onInt] = indiceCalc;
			onInt++;
			indiceCalc -= gridSizeM1;
		}
		indiceCalc += 1;
	}

	//debug output to see if its setting up the indices correctly
	cout << endl << "indice steup done origin tri: " << indiceArray[0] << ", " << indiceArray[1] << ", " << indiceArray[2] << " .. " << indiceArray[3] << ", " << indiceArray[4] << ", " << indiceArray[5] << " .. " << endl;
	cout << "tri cont. : " << indiceArray[6] << ", " << indiceArray[7] << ", " << indiceArray[8] << " .. " << indiceArray[9] << ", " << indiceArray[10] << ", " << indiceArray[11] << " .. " << indiceArray[12] << ", " << indiceArray[13] << ", " << indiceArray[14] << ".... " << indiceArray[1919] << indiceArray[1920] << indiceArray[1921] << endl;
}

void initGridVAOVBO(void)
{
	//=================================================
	//============== grid vao/vbo setup ===============
	//=================================================

	//setup grid object
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);
	// Setup VBO for vertex position data
	glGenBuffers(1, &gridVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0); // attribute 0 gets data from bound VBO (so assign vertex position buffer to attribute 0)

	glGenBuffers(1, &gridColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colourArray), colourArray, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, (const GLvoid*)0); // attribute 1 gets colour data

	//NOT NEEDED IF I DONT HAVE INDEX ARRAY
	glGenBuffers(1, &gridIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indiceArray), indiceArray, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//=================================================
	//======== HERE BE TEXTURE LOADING DRAGONS ========
	//=================================================

	//SET UP BMP IMAGE LOAD FOR HEIGHTMAP AND BIND TO TEXTURE
	//LOAD IMAGE:
	// MAKE SURE FORMAT IS POWER OF TWO TEXTURES.bmp (128*128, 256*256 etc)
	heightMap = customBMPFileLoad(filePath);
	//enable textures
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &heightMap);
	
	//Bind the newly setup texture
	glBindTexture(GL_TEXTURE_2D, heightMap);

	//give the image to openGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, hmWidth, hmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, BMPimageData);

	//======= what do these lines do? =======
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	//reset vertex array before going back to code
	glBindVertexArray(0);

}

void initWarpingLines(void)
{
	int lineCalc = 0;//holds the number of the vertice it needs to link to
	int onInt = 0;//holds the int its on

	//for the sphere triangles dont really work too well so here i set up lines 
	//joining each point on each row.

	for (int x = 0; x < gridSize; x++){
		for (int y = 0; y < gridSizeM1; y++)
		{
			//maths for working out two points (1 square)
			warpedLines[onInt] = lineCalc;
			onInt++;
			lineCalc ++;//point to next point
			warpedLines[onInt] = lineCalc;
			onInt++;
			lineCalc ++;//point to 1 point ahead
		
		}
		lineCalc += 1;
	}

	//debug output to see if its setting up the lines correctly
	cout << endl << "setup steup done origin at: " << warpedLines[0] << ", " << warpedLines[1] << endl;
}

//=== custom bmp file loading function ===
GLuint customBMPFileLoad(const char * filePath)
{
	FILE *file;
	errno_t errorCode = fopen_s(&file, filePath, "rb");
	if (!file)//if loading the file failed
	{ 
		cout << "Image load failed\n" << endl;
		return 0; 
	}

	if (fread(BMPheader, 1, 54, file) != 54)// If the header isnt 54 bytes theres an error
	{ 
		cout << "Header size problem: Not a correct BMP file\n" << endl;
		return 0;
	}

	if (BMPheader[0] != 'B' || BMPheader[1] != 'M')// checks to see if the header starts with the correct characters for a bmp file
	{
		cout << "Not a correct BMP file\n" << endl;;
		return 0;
	}

	//read integers from the byte array
	dataPosition	= *(int*)&(BMPheader[0x0A]);
	imageSize		= *(int*)&(BMPheader[0x22]);
	hmWidth			= *(int*)&(BMPheader[0x12]);
	hmHeight		= *(int*)&(BMPheader[0x16]);

	//if any data is missing create it!
	if (imageSize == 0) imageSize = hmWidth * hmHeight * 3; //one byte for r, g & b
	if (dataPosition == 0) dataPosition = 54;//BMP header file just needs this

	//buffer
	BMPimageData = new unsigned char[imageSize];
	//read data to buffer
	fread(BMPimageData, 1, imageSize, file);
	//close file as weve loaded it into the buffer we just created
	fclose(file);
}
//========= very proud of this :) ========


void drawSurface(void)
{
	//=================================================
	//============= get/set funkyMath :) ==============
	//=================================================

	//create the funkymath holder
	GUVector4 funkyHolder = GUVector4(funkyMath0, funkyMath1, funkyMath2, funkyMath3);
	//get the funkymath location in the shader
	static GLint funkyMathLocation = glGetUniformLocation(basicShader, "funkyMathValues");
	//set funkymath in the shader
	glUniform4fv(funkyMathLocation, 1, (GLfloat*)&funkyHolder);

	//=================================================
	//============= get/set timeValue :) ==============
	//=================================================

	//We already have the time float set up
	//get the time value location in the shader
	static GLfloat timingLocation = glGetUniformLocation(basicShader, "inTime");
	//set time in the shader
	glUniform1f(timingLocation, sendTime);

	//=================================================
	//============= get/set sizeValue :) ==============
	//=================================================

	sendSize = float(gridSize);

	//We already have the size float set up
	//get the time value location in the shader
	static GLfloat sizeLocation = glGetUniformLocation(basicShader, "inSize");
	//set size in the shader
	glUniform1f(sizeLocation, sendSize);


	//=================================================
	//=============== grid draw func's ================
	//=================================================

	glEnable;

	//bind grid
	glBindVertexArray(gridVAO);

	//bind texture

	//if wireframe mode set polygon mode to gl_line and drawPoints
	if (wireFrame)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPointSize(2.0f);
		glDrawArrays(GL_POINTS, 0, gridSizeSquared);
	}

	if (funkyMath0 >= 5.0 - 0.1 && funkyMath0 <= 5.0 + 0.1)
	{
		glDrawArrays(GL_LINES, 0, (gridSize * gridSizeM1) * 2);
	}
	else
	{
		//draw vertexs with triangles
		glDrawElements(GL_TRIANGLES, ((gridSizeM1 * 2) * gridSizeM1) * 3, GL_UNSIGNED_INT, (const GLvoid*)0);
	}

	//if wireframe mode reset the polygonmode before returning
	if (wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//unbind the grid
	glBindVertexArray(0);

	//debug outputs
	if (funkyMath0 == 2.0)cout << funkyMath2 << endl;

}

#pragma endregion