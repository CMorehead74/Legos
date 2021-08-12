// GLView.cpp
/* 
Copyright (c) 2009, Richard S. Wright Jr.
GLTools Open Source Library
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list 
of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used 
to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GLView.h"


////////////////////////////////////////////////////////////////////////////
// Do not put any OpenGL code here. General guidence on constructors in 
// general is to not put anything that can fail here either (opening files,
// allocating memory, etc.)
GLView::GLView(void) : screenWidth(800), screenHeight(600), bFullScreen(false), bAnimated(true), samples(1)
{



}

bool LoadBMPAsTexture( const char *szFileName, GLenum filter, GLenum wrapMode)
{
	//get color data 
	GLbyte* pBits;
	int nWidth, nHeight;
	pBits = gltReadBMPBits(szFileName, &nWidth, &nHeight);
	if (pBits == NULL)
	{
		MessageBox(NULL, "Error loading texture!", szFileName, MB_OK);
		return false;
	}

	//glTextImage2D
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, pBits);
	free(pBits);

	if (filter != GL_LINEAR && filter != GL_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	//TextPar

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	return true;
}

bool LoadTGAAsTexture( const char *szFileName, GLenum filter, GLenum wrapMode)
{
	//get color data 
	GLbyte* pBits;
	int nWidth, nHeight;
	int componets;
	GLenum format;

	//pBits = gltReadBMPBits(szFileName, &nWidth, &nHeight);
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &componets, &format);
	if (pBits == NULL)
	{
		MessageBox(NULL, "Error loading texture!", szFileName, MB_OK);
		return false;
	}

	//glTextImage2D
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, componets, nWidth, nHeight, 0, format, GL_UNSIGNED_BYTE, pBits);
	free(pBits);

	if (filter != GL_LINEAR && filter != GL_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	//TextPar

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	return true;
}

void GLView::LoadSkyBoxShader(void)
{
	gltMakeCube(cubeBatch, 40.0f);

	skyBoxShader = gltLoadShaderPairWithAttributes("skyBox.vp", "skyBox.fp", 1, GLT_ATTRIBUTE_VERTEX, "vVertex");

	locMVPSkyBox = glGetUniformLocation(skyBoxShader, "MVP");

	if (skyBoxShader == 0 || locMVPSkyBox < 0 )
		MessageBox(NULL, "Bad Shader, FOR skyBox, Fix yo shit!", NULL, MB_OK); 

	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
	LoadSkyBox();
}

///////////////////////////////////////////////////////////////////////////////
// OpenGL related startup code is safe to put here. Load textures, etc.
void GLView::Initialize(void)
{
	// Make sure OpenGL entry points are set
	glewInit();

	// Initialze Shader Manager
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);
	LoadSkyBoxShader();

	// Ugly plumb red picked on purpose... no a likely color to occur by accident.
	// So if you see this color, it means it's working ;-)
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);	//background
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

	//gltMakeTorus(torusBatch, 0.4f, 0.15f, 25, 25);		//this is the test the size of camera

	GLfloat verts[] = {	-8.4f,	8.4f,	0.0f,
		-8.4f,	-8.4f,	0.0f,
		8.4f,	-8.4f,	0.0f,
		8.4f,	8.4f,	0.0f };

	groundBatch.Begin(GL_TRIANGLE_FAN, 4);
	groundBatch.CopyVertexData3f(verts);
	groundBatch.End();

	gltMakeCylinder(legoCylinderBatch, 0.05f, 0.05f, 0.05f, 26, 13);
	gltMakeDisk(legoDiskBatch, 0.0f, 0.05f, 26, 13);

	GLfloat brickVerts[] = {	-0.60f, 0.15f, 0.0f,
		-0.60f, -0.15f, 0.0f,
		0.60f, -0.15f, 0.0f,
		0.60f, 0.15f, 0.0f };

	brickBatch4F.Begin(GL_TRIANGLE_FAN, 4);
	brickBatch4F.CopyVertexData3f(brickVerts);
	brickBatch4F.End();

	GLfloat brickVertsT[] = {	-0.60f, 0.3f, 0.0f,
		-0.60f, -0.3f, 0.0f,
		0.60f, -0.3f, 0.0f,
		0.60f, 0.3f, 0.0f };

	brickBatch4T.Begin(GL_TRIANGLE_FAN, 4);
	brickBatch4T.CopyVertexData3f(brickVertsT);
	brickBatch4T.End();

	GLfloat brickVertTopper[] = {	-0.60f, 0.025f, 0.0f,
		-0.60f, -0.025f, 0.0f,
		0.60f, -0.025f, 0.0f,
		0.60f, 0.025f, 0.0f };

	brickBatchTopper.Begin(GL_TRIANGLE_FAN, 4);
	brickBatchTopper.CopyVertexData3f(brickVertTopper);
	brickBatchTopper.End();

	GLfloat brickVertSides[] = {	-0.3f, 0.15f, 0.0f,
		-0.3f, -0.15f, 0.0f,
		0.3f, -0.15f, 0.0f,
		0.3f, 0.15f, 0.0f };

	brickBatch2.Begin(GL_TRIANGLE_FAN, 4);
	brickBatch2.CopyVertexData3f(brickVertSides);
	brickBatch2.End();



	//FERRIS WHEEL---------------------------------------
	//make vertex half side of the bricks
	GLfloat ferrisWheelVert[] = {	-0.60f, 0.075f, 0.0f,
		-0.60f, -0.075f, 0.0f,
		0.60f, -0.075f, 0.0f,
		0.60f, 0.075f, 0.0f };

	ferrisWheelBrick.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheelBrick.CopyVertexData3f(ferrisWheelVert);
	ferrisWheelBrick.End();

	GLfloat FWVT[] = {					-0.60f, 0.15f, 0.0f,
		-0.60f, -0.15f, 0.0f,
		0.60f, -0.15f, 0.0f,
		0.60f, 0.15f, 0.0f };

	ferrisWheelBrickTop.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheelBrickTop.CopyVertexData3f(FWVT);
	ferrisWheelBrickTop.End();

	GLfloat FWVS[] = {					-0.075f, 0.15f, 0.0f,
		-0.075f, -0.15f, 0.0f,
		0.075f, -0.15f, 0.0f,
		0.075f, 0.15f, 0.0f };

	ferrisWheelBrickSide.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheelBrickSide.CopyVertexData3f(FWVS);
	ferrisWheelBrickSide.End();

	GLfloat FWV4x4[] = {				-0.3f, 0.15f, 0.0f,
		-0.3f, -0.15f, 0.0f,
		0.3f, -0.15f, 0.0f,
		0.3f, 0.15f, 0.0f };

	ferrisWheel4x4.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheel4x4.CopyVertexData3f(FWV4x4);
	ferrisWheel4x4.End();

	GLfloat FWV4x4S[] = {				-0.15f, 0.3f, 0.0f,
		-0.15f, -0.3f, 0.0f,
		0.15f, -0.3f, 0.0f,
		0.15f, 0.3f, 0.0f };

	ferrisWheel4x4Side.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheel4x4Side.CopyVertexData3f(FWV4x4S);
	ferrisWheel4x4Side.End();

	GLfloat FWS4x4T[] = {				-0.3f, 0.3f, 0.0f,
		-0.3f, -0.3f, 0.0f,
		0.3f, -0.3f, 0.0f,
		0.3f, 0.3f, 0.0f };

	ferrisWheel4x4Top.Begin(GL_TRIANGLE_FAN, 4);
	ferrisWheel4x4Top.CopyVertexData3f(FWS4x4T);
	ferrisWheel4x4Top.End();


	gltMakeCylinder(ferrisWheelRebar, 0.03f, 0.03f, 0.9f, 26, 13);
	gltMakeCylinder(ferrisWheelSupport, 0.075f, 0.075f, 1.3f, 26, 13);
	gltMakeDisk(ferrisWheelDiskSupport, 0.075f, 0.15f, 26, 13);
	gltMakeCylinder(ferrisWheelSupport2, 0.10f, 0.10f, 0.45f, 26, 13);


	//ferrisWheelSupportSide

	GLfloat FW4x4ST[] = {
		-0.30f, 0.085f, 0.0f,
		-0.30f, -0.085f, 0.0f,
		0.30f, -0.085f, 0.0f,
		0.30f, 0.085f, 0.0f };

		ferrisWheelSupportSide.Begin(GL_TRIANGLE_FAN, 4);
		ferrisWheelSupportSide.CopyVertexData3f(FW4x4ST);
		ferrisWheelSupportSide.End();

		GLfloat FW4x4ST2[] =
		{	-0.15f, 0.085f, 0.0f,
		-0.15f, -0.085f, 0.0f,
		0.15f, -0.085f, 0.0f,
		0.15f, 0.085f, 0.0f };

		ferrisWheelSupportSide2.Begin(GL_TRIANGLE_FAN, 4);
		ferrisWheelSupportSide2.CopyVertexData3f(FW4x4ST2);
		ferrisWheelSupportSide2.End();

		GLfloat FW4x4Top[] = {	
			-0.3f, 0.15f, 0.0f,
			-0.3f, -0.15f, 0.0f,
			0.3f, -0.15f, 0.0f,
			0.3f, 0.15f, 0.0f };

			ferrisWheelSupportTop.Begin(GL_TRIANGLE_FAN, 4);
			ferrisWheelSupportTop.CopyVertexData3f(FW4x4Top);
			ferrisWheelSupportTop.End();

			GLfloat vCar[] =
			{		
				-0.15f, 0.075f, 0.0f,
				-0.15f, -0.075f, 0.0f,
				0.15f, -0.075f, 0.0f,
				0.15f, 0.075f, 0.0f
			};

			carBrick.Begin(GL_TRIANGLE_FAN, 4);
			carBrick.CopyVertexData3f(vCar);
			carBrick.End();

			gltMakeCylinder(carWheel, 0.075f, 0.075f, 0.05f, 26, 13);
			gltMakeDisk(carWheelDisk, 0.0, 0.075f, 26, 13);

			GLfloat windowverts[] =
			{	/*0.09f, -0.09f, 0.0f,
				0.0f, 0.09, 0.0f,
				-0.09f, -0.09f, 0.0f*/
				0.075f, -0.15, 0.0f,
				0.075f, 0.0f, 0.0f,
				-0.075f, -0.15, 0.0f
			};

			carWindshieldTriangle.Begin(GL_TRIANGLES, 3);
			carWindshieldTriangle.CopyVertexData3f(windowverts);
			carWindshieldTriangle.End();

			gltMakeCylinder(carHeadLights, 0.025f, 0.025f, 0.025f, 26, 13);

			LoadBorder();


	glBindTexture(GL_TEXTURE_2D, tHandles[1]);
	LoadTGAAsTexture("scratched.tga", GL_LINEAR, GL_REPEAT);


	glBindTexture(GL_TEXTURE_2D, tHandles[2]);
	LoadTGAAsTexture("legoMan03.tga", GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT);

	treeShader = gltLoadShaderPairWithAttributes("lego.vp", "lego.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertexPos", GLT_ATTRIBUTE_TEXTURE0, "vTextureCoordAttribute");
	TREEUniformMVP = glGetUniformLocation(treeShader, "mMVP");

	M3DVector3f boxVerts[4] =
	{ 
		-1.0f, 1.0f, 0.0f, 
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};

	M3DVector2f vTexCoord[4] = 
	{
		1.0f, 1.0f,
		1.0f, 0.0f, 
		0.0f, 0.0f, 
		0.0f, 1.0f
	};

	treeBatch.Begin(GL_TRIANGLE_FAN, 4, 2);
	treeBatch.CopyTexCoordData2f(vTexCoord, 0);
	treeBatch.CopyVertexData3f(boxVerts);
	treeBatch.End();

	GLuint hUniformDetail = glGetUniformLocation(treeShader, "detailMap");
	glUseProgram(treeShader);
	glUniform1i(hUniformDetail,1);
	

	


			//GLfloat vCarSide[] =
			//{		
			//	-0.3f, 0.075f, 0.0f,
			//	-0.3f, -0.075f, 0.0f,
			//	0.3f, -0.075f, 0.0f,
			//	0.3f, 0.075f, 0.0f
			//};

			//	carBrick.Begin(GL_TRIANGLE_FAN, 4);
			//	carBrick.CopyVertexData3f(vCarSide);
			//	carBrick.End();

			//TOON SHADING..................................................		
			//shader = gltLoadShaderPairWithAttributes("toon.vp", "toon.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNorm");

			//vLight = glGetUniformLocation(shader, "vLightPos");
			//MVP = glGetUniformLocation(shader, "mvp");
			//MV = glGetUniformLocation(shader, "mv");
			//normal = glGetUniformLocation(shader, "normalMat");
			//color = glGetUniformLocation(shader, "colorMap");

			//	if (shader == 0 || vLight < 0 || MVP < 0 || MV < 0 || normal < 0 || color < 0)
			//	MessageBox(NULL, "Bad Shader, Fix yo shit!", NULL, MB_OK); 

			//	glGenTextures(5, text);
			//	glBindTexture(GL_TEXTURE_1D, text[0]);

			//	GLubyte toonRed[4][3] = { { 64, 0, 0 },
			//                             { 128, 0, 0, },
			//                             { 192, 0, 0, },
			//                             { 255, 0, 0, }};

			//	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, toonRed);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

			//	glBindTexture(GL_TEXTURE_1D, text[1]);

			//	GLubyte toonBlue[4][3] = {	{ 0, 0, 115 },
			//								{ 0, 0, 160, },
			//								{ 0, 0, 200, },
			//								{ 0, 0, 255, }};

			//	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, toonBlue);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

void GLView::DrawLegoDude(void)
{
	M3DVector4f vWhite  = { 1.0f, 1.0f, 1.0f, 1.0f };

	modelViewMatrix.PushMatrix();
	{
		glUseProgram(treeShader);
		glUniformMatrix4fv(TREEUniformMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glActiveTexture(GL_TEXTURE1);		
		glBindTexture(GL_TEXTURE_2D, tHandles[1]);
		glActiveTexture(GL_TEXTURE0);		
		glBindTexture(GL_TEXTURE_2D, tHandles[2]);
		treeBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		glUseProgram(treeShader);
		glUniformMatrix4fv(TREEUniformMVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
		glActiveTexture(GL_TEXTURE1);	
		glBindTexture(GL_TEXTURE_2D, tHandles[1]);
		glActiveTexture(GL_TEXTURE0);		
		glBindTexture(GL_TEXTURE_2D, tHandles[2]);
		treeBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
}

void GLView::LoadSkyBox(void)
{
	GLbyte *pBytes;
	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;
	int i;

	// Cull backs of polygons
	szCubeFaces[0] = "pos_x.tga"; //toy-story-3-andys-room
	szCubeFaces[1] = "neg_x.tga";
	szCubeFaces[2] = "pos_y.tga";
	szCubeFaces[3] = "neg_y.tga";
	szCubeFaces[4] = "pos_z.tga";
	szCubeFaces[5] = "neg_z.tga";

	cube[0] = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	cube[1] = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	cube[2] = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	cube[3] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
	cube[4] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
	cube[5] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;

	// Set up texture maps        
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);       
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load Cube Map images
	for(i = 0; i < 6; i++)
	{        
		// Load this texture map
		pBytes = gltReadTGABits(szCubeFaces[i], &iWidth, &iHeight, &iComponents, &eFormat);
		glTexImage2D(cube[i], 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
		free(pBytes);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

///////////////////////////////////////////////////////////////////////////////
// Do your cleanup here. Free textures, display lists, buffer objects, etc.
void GLView::Shutdown(void)
{
	glDeleteTextures(1, tHandles);
}

///////////////////////////////////////////////////////////////////////////////
// This is called at least once and before any rendering occurs. If the screen
// is a resizeable window, then this will also get called whenever the window
// is resized.
void GLView::Resize(GLsizei nWidth, GLsizei nHeight)
{
	screenWidth = nWidth;
	screenHeight = nHeight;
	glViewport(0, 0, nWidth, nHeight);
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

	viewFrustum.SetPerspective(35.0f, float(nWidth)/float(nHeight), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelViewMatrix.LoadIdentity();
}

void GLView::MoveCamera()
{
	static CStopWatch timer;
	float deltaT = timer.GetElapsedSeconds();
	timer.Reset();
	if (GetAsyncKeyState(VK_UP))
		cameraFrame.MoveForward(3.0f * deltaT);
	if (GetAsyncKeyState(VK_DOWN))
		cameraFrame.MoveForward(-3.0f * deltaT);
	if (GetAsyncKeyState(VK_LEFT))
		cameraFrame.RotateWorld(deltaT * m3dDegToRad(60.0f), 0.0f, 1.0f, 0.0f);
	if (GetAsyncKeyState(VK_RIGHT))
		cameraFrame.RotateWorld(deltaT * m3dDegToRad(-60.0f), 0.0f, 1.0f, 0.0f);

}

void GLView::DrawFloor()
{
	//FLOOR
	GLfloat vGreen[] = { 0.14f, 0.55f, 0.13f, 1.0f};
	GLfloat vOffGreen[] = { 0.11f, 0.5f, 0.1f, 1.0f };
	GLfloat vCyan[] = { 0.0f, 1.0f, 1.0f, 1.0f };
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		//shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		groundBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//LEGO DOTS-------------------------------------------------------------
	GLfloat floorRadius = 8.4f;	//<----- adjust size of FLOOR here, and above on verts[]
	GLfloat gridSpace = 0.3f;
	GLfloat zAxis = 0.3f;

	for (float z = -floorRadius; z < floorRadius; z +=zAxis)
	{
		for (float x = -floorRadius; x < floorRadius; x +=gridSpace)
		{
			modelViewMatrix.PushMatrix();
			{
				modelViewMatrix.Translate(x + 0.1f, 0.05f, z);
				modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
				//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vOffGreen);
				shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vOffGreen);
				legoCylinderBatch.Draw();
			}
			modelViewMatrix.PopMatrix();
			modelViewMatrix.PushMatrix();
			{
				modelViewMatrix.Translate(x +0.1f, 0.05f, z);
				modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
				//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vOffGreen);
				shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vOffGreen);
				legoDiskBatch.Draw();
			}
			modelViewMatrix.PopMatrix();
		}
	}
}

void GLView::DrawBricksSouth(/*M3DVector4f vLightPosWorld*/)
{
	GLfloat vCyan[] = { 0.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vPink[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat floorRadius = 8.4f;	//<----- adjust size of FLOOR here, and above on verts[]
	GLfloat xAxis = 1.2f;

	//	modelViewMatrix.PushMatrix();
	//	{
	//		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-0.65f);
	//		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
	//		brickBatch4F.Draw();
	//	}
	//	modelViewMatrix.PopMatrix();

	//for (float z = -floorRadius; z < floorRadius; z +=xAxis)
	//{
	//	modelViewMatrix.PushMatrix();
	//	{
	//		modelViewMatrix.Translate(floorRadius += z, 0.15f, floorRadius-0.65f);
	//		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
	//		brickBatch4F.Draw();
	//	}
	//	modelViewMatrix.PopMatrix();
	//}
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

		brickBatch4F.Draw();

	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25, 0.15f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//SECOND ROW------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.25f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-2.45f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.65f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.85f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.05f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.25f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-8.45f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.65f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.85f, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.05, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.25, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-14.45, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.65, 0.45f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//THIRD ROW---------------------------------------------------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25, 0.75f, floorRadius-0.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//TOP ROW--------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X		
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25, 0.925f, floorRadius-0.75f);
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();

	//TOPPERS--------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X		
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25, 0.95f, floorRadius-0.45f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();

	//LEGO DOTS---------------------------------------------------

	GLfloat gridSpace = 0.3f;

	for (float x = -floorRadius; x < floorRadius; x +=gridSpace)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(x + 0.1f, 1.0f, floorRadius-0.30f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(x + 0.1f, 1.0f, floorRadius-0.60f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			//shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
		//LEGO DOT DISKS
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(x + 0.1f, 1.0f, floorRadius-0.30f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(x + 0.1f, 1.0f, floorRadius-0.60f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
	}
}

void GLView::DrawBricksEast()
{
	GLfloat vCyan[]		= { 0.0f, 1.0f, 1.0f, 1.0f };	//lego dots
	GLfloat vPink[]		= { 1.0f, 0.0f, 1.0f, 1.0f };	//test color
	GLfloat vBlue[]		= { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[]		= { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[]	= { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[]	= { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[]	= { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat floorRadius = 8.4f;	//<----- adjust size of FLOOR here, and above on verts[]

	//FIRST ROW-----------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-6.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//SECOND ROW ----------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-1.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-3.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-4.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-5.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-6.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-7.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-9.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-10.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-11.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-12.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-13.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-15.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.45f, floorRadius-16.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//THIRD ROW --------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-6.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//TOP ROW--------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-6.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.35f, 0.95f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();

	// TOPPERs----------------------------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-6.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.925f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
		brickBatchTopper.Draw();
	}
	modelViewMatrix.PopMatrix();

	//LEGO DOTS -------------------------------------------------------------------------------

	GLfloat gridSpace = (floorRadius - 16.5f);

	for (float x = 0; x <= 51; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(floorRadius - 0.2, 1.0f, gridSpace += 0.3f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
	}

	gridSpace = (floorRadius - 16.5f);

	for (float x = 0; x <= 51; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(floorRadius - 0.5, 1.0f, gridSpace += 0.3f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
	}
}

void GLView::DrawBricksNorth()
{
	GLfloat vCyan[]		= { 0.0f, 1.0f, 1.0f, 1.0f };	//lego dots
	GLfloat vPink[]		= { 1.0f, 0.0f, 1.0f, 1.0f };	//test color
	GLfloat vBlue[]		= { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[]		= { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[]	= { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[]	= { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[]	= { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat floorRadius = 8.4f;	//<----- adjust size of FLOOR here, and above on verts[]

	//FIRST ROW-----------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//SECOND ROW------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.25f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-2.45f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.65f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.85f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.05f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.25f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-8.45f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.65f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.85f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.05f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.25f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-14.45f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.65f, 0.45f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//THIRD ROW--------------------------------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-0.65f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-1.85f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-3.05, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-4.25f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-5.45f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-6.65f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-7.85f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-9.05f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-10.25f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-11.45f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-12.65f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-13.85f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-15.05f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-16.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//Top ROW ------------------------------------------------------------

	float xAxis = floorRadius-17.45f;

	for (float x = 0; x < 14; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(xAxis += 1.2, 0.95f, floorRadius-16.65f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
			brickBatch4T.Draw();
		}
		modelViewMatrix.PopMatrix();
	}
	//TOPPERS-------------------------------------------------------------------

	xAxis = floorRadius-17.45f;

	for (float x = 0; x < 14; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(xAxis += 1.2, 0.925f, floorRadius-16.35f);
			shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vCyan);
			brickBatchTopper.Draw();
		}
		modelViewMatrix.PopMatrix();
	}

	//LEGO DOTS-------------------------------------------------------------------

	xAxis = floorRadius-17.0f;

	for (float x = 0; x <= 55; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(xAxis += 0.3, 1.0f, floorRadius-16.5f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
	}

	xAxis = floorRadius-17.0f;
	for (float x = 0; x <= 55; x++)
	{
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(xAxis += 0.3, 1.0f, floorRadius-16.8f);
			modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
			shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vCyan);
			legoCylinderBatch.Draw();
			legoDiskBatch.Draw();
		}
		modelViewMatrix.PopMatrix();
	}
}

void GLView::DrawBricksWest()
{
	GLfloat vCyan[]		= { 0.0f, 1.0f, 1.0f, 1.0f };	//lego dots
	GLfloat vPink[]		= { 1.0f, 0.0f, 1.0f, 1.0f };	//test color
	GLfloat vBlue[]		= { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[]		= { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[]	= { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[]	= { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[]	= { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat floorRadius = 8.4f;	//<----- adjust size of FLOOR here, and above on verts[]

	//FIRST ROW-----------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-6.15f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.15f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//SECOND ROW ---------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-1.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-3.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-4.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-5.55f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-6.75f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-7.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-9.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-10.35f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-11.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-12.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-13.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-15.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.45f, floorRadius-16.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//THIRD ROW----------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-4.95f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-6.15f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-7.35f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-9.75f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-10.95f);
	//	modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
	//	brickBatch4F.Draw();
	//}
	//modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.25f, 0.75f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4F.Draw();
	}
	modelViewMatrix.PopMatrix();

	//SIDES-----------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.45f, floorRadius-4.95f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.15f, floorRadius-5.55f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.75f, floorRadius-4.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.15f, floorRadius-6.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.45f, floorRadius-7.35f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.75f, floorRadius-7.95f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.75f, floorRadius-9.15);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.45f, floorRadius-9.75f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.45f, floorRadius-10.95f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.75f, floorRadius-11.55f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch2.Draw();
	}
	modelViewMatrix.PopMatrix();

	//TOP VIEW-----------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-15.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-14.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-13.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-12.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.60f, floorRadius-11.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.30f, floorRadius-10.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.30f, floorRadius-9.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.60f, floorRadius-9.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-8.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.60f, floorRadius-7.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.30f, floorRadius-7.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.30f, floorRadius-4.95f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.60f, floorRadius-4.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-3.75f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-2.55f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.55f, 0.90f, floorRadius-1.35f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		brickBatch4T.Draw();
	}
	modelViewMatrix.PopMatrix();

	//LEGO DOTS-----------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-16.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-16.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-15.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-15.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-15.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-15.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-15.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-15.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//YELLOW
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-15.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-15.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-14.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-14.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-14.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-14.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-14.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-14.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//red
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-13.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-13.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-13.5);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-13.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-13.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-13.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-12.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-12.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//green
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-12.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-12.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-12.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-12.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-12.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-12.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-11.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-11.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//red
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-11.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-11.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-11.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-11.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//red
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-10.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-10.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-10.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-10.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//white
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-10.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-10.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-9.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-9.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//blue
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-9.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-9.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-9.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-9.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//blue
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-9.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-9.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-8.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-8.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-8.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-8.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-8.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-8.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//yellow
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-7.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-7.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-7.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-7.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//yellow
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-7.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-7.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-6.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-6.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//yellow
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-5.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-5.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.35f, floorRadius-5.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.35f, floorRadius-5.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vYellow);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//green
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-4.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-4.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.65f, floorRadius-4.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.65f, floorRadius-4.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//blue
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-4.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-4.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-3.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-3.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-3.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-3.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-3.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-3.3f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//red
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-3.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-3.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-2.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-2.7f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-2.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-2.4f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-2.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-2.1f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//white
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-1.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-1.8f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-1.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-1.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-1.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-1.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.4f, 0.95f, floorRadius-0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(floorRadius-16.7f, 0.95f, floorRadius-0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
		legoDiskBatch.Draw();
	}
	modelViewMatrix.PopMatrix();

}

void GLView::DrawFerrisWheel(void)
{
	GLfloat vCyan[] = { 0.0f, 1.0f, 1.0f, 1.0f };	
	GLfloat vPink[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	//floorlength = 8.4, full brick is 0.6, 0.15, 0.0

	//flat base-south----------------------------------------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.075f, 0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheelBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.075f, 1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheelBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	//top of base
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.15f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheelBrickTop.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.15f, 0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheelBrickTop.Draw();
	}
	modelViewMatrix.PopMatrix();
	//side base
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.55f, 0.075f, 0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.65f, 0.075f, 0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.55f, 0.075f, 0.9f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.65f, 0.075f, 0.9f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	//LEGO DOTS
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.5f, 0.20f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.5f, 0.20f, 0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.4f, 0.20f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.4f, 0.20f, 0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.5f, 0.20f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.5f, 0.20f,-0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.4f, 0.20f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.4f, 0.20f, -0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//--4x4 LEGO DOTS---------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.1f, 1.10f, 0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.2f, 1.10f, 0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.1f, 1.10f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.2f, 1.10f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vGreen);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();


	//flat base-north----------------------------------------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.075f, -0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheelBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.075f, -1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheelBrick.Draw();
	}
	modelViewMatrix.PopMatrix();

	//top of base
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.15f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheelBrickTop.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.15f, -0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheelBrickTop.Draw();
	}
	modelViewMatrix.PopMatrix();
	//side base
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.55f, 0.075f, -0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.65f, 0.075f, -0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.55f, 0.075f, -0.9f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.65f, 0.075f, -0.9f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);

		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheelBrickSide.Draw();
	}
	modelViewMatrix.PopMatrix();

	//4x4block----------------------------------------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.3f, 0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.3f, 1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//box 3
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.9f, 1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.9f, 0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//side 4x4
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.3f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.9f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.3f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.9f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();

	//second block4x4 - side
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.6f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.6f, 0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.6f, 0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.6f, 1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vWhite);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//Top 4x4

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.05f, 0.75f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
		ferrisWheel4x4Top.Draw();
	}
	modelViewMatrix.PopMatrix();

	//North 4x4-----------------------------------------
	// -0.45f); -1.05fz
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.3f, -0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.3f, -1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//box 3
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.9f, -1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.9f, -0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//side 4x4
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.3f, -0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.9f, -0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.3f, -0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.9f,-0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 0.6f, -0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 0.6f, -0.75f);
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);

		ferrisWheel4x4Side.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.6f, -0.45f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 0.6f, -1.05f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);
		ferrisWheel4x4.Draw();
	}
	modelViewMatrix.PopMatrix();
	//Top 4x4

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.05f, -0.75f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
		ferrisWheel4x4Top.Draw();
	}
	modelViewMatrix.PopMatrix();
	//--4x4 LEGO DOTS---------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.1f, 1.10f, -0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.2f, 1.10f, -0.9f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vRed);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();



	//----------------------------------------------------------------------------ABOVE BASE-------------------------------------------------------------------
	//-----------------MAIN SUPPORT0---------------------------------------------------------------------------------------------------------------------------

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.14f, -0.65f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		ferrisWheelSupport.Draw();
	}
	modelViewMatrix.PopMatrix();

	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.135f, -0.75f);

		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.135f, -0.45f);

		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 1.135f, -0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f); 
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 1.135f, -0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f); 
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.22f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);	
		ferrisWheelSupportTop.Draw();
	}
	modelViewMatrix.PopMatrix();

	//Lego DOTS
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.1f, 1.275f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.2f, 1.275f, -0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.135f, 0.75f);

		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.135f, 0.45f);

		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.35f, 1.135f, 0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f); 
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.25f, 1.135f, 0.6f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f); 
		//modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);		
		ferrisWheelSupportSide2.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.22f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f); //rotate on the X (top)
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlue);	
		ferrisWheelSupportTop.Draw();
	}
	modelViewMatrix.PopMatrix();

	//Lego DOTS
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(0.1f, 1.275f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.2f, 1.275f, 0.6f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vBlue);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();

	//--------------------------------------------DISK------------------------------------------------------------------------------------- 

}

void GLView::DrawFerrisWheelRotate()
{
	static CStopWatch timer;
	float yRot = timer.GetElapsedSeconds() * 60.0f;
	GLfloat vCyan[] = { 0.0f, 1.0f, 1.0f, 1.0f };	
	GLfloat vPink[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.225f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel
		modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelSupport2.Draw();
	}
	modelViewMatrix.PopMatrix();
	////================================================================================================

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(15, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel
		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-15, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(45, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-45, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(75, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-75, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(105, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-105, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(135, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-135, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(165, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, -0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-165, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	//-------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(15, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-15, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(45, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-45, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(75, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-75, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(105, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-105, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(135, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-135, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(165, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite);
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-0.05f, 1.145f, 0.2f);
		modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

		modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
		modelViewMatrix.Rotate(-165, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vWhite); //findend
		ferrisWheelRebar.Draw();
	}
	modelViewMatrix.PopMatrix();

	//chairs----------------------------------------------------------
	//modelViewMatrix.PushMatrix();
	//{
	//	modelViewMatrix.Translate(-0.05f, 1.145f, 0.0f);
	//	//modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);	//rotates the ferris wheel

	//	modelViewMatrix.Rotate(-90, 1.0f, 0.0f, 0.0f);
	//	modelViewMatrix.Rotate(90, 0.0f, 0.0f, 1.0f);
	//	shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vPink); //findend
	//	ferrisWheelRebar.Draw();
	//}
	//modelViewMatrix.PopMatrix();

}

void GLView::DrawCar(void)
{
	static CStopWatch timer;
	float yRot = timer.GetElapsedSeconds() * 60.0f;
	GLfloat vCyan[] = { 0.0f, 1.0f, 1.0f, 1.0f };	
	GLfloat vPink[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat vRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat vYellow[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat vBlack[] = {0.0f, 0.0f, 0.0f, 1.0f };

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.0f, 0.125f, -0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.0f, 0.125f, -0.6f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.85f, 0.125f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.85f, 0.125f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.15f, 0.125f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.15f, 0.125f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vPink);
		carBrick.Draw();
	}
	modelViewMatrix.PopMatrix();

	//Wheel------------------
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.85f, 0.075f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheel.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.8f, 0.075f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheelDisk.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.85f, 0.075f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheel.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.8f, 0.075f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheelDisk.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.2f, 0.075f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheel.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.2f, 0.075f, -0.45f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheelDisk.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.2f, 0.075f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheel.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.2f, 0.075f, -0.15f);
		modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
		carWheelDisk.Draw();
	}
	modelViewMatrix.PopMatrix();
	//HeadLights
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.1f, 0.09f, 0.0f);
		//modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		carHeadLights.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-4.9f, 0.09f, 0.0f);
		//modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vYellow);
		carHeadLights.Draw();
	}
	modelViewMatrix.PopMatrix();
	//LEGO DOTS

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.0f, 0.25f, -0.2f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vPink);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.Translate(-5.0f, 0.25f, -0.5f);
		modelViewMatrix.Rotate(90, 1.0f, 0.0f, 0.0f);
		shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(),transformPipeline.GetProjectionMatrix(), vPink);
		legoCylinderBatch.Draw();
	}
	modelViewMatrix.PopMatrix();


}

void GLView::DrawOutline()
{
	/*	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(9.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	float vBlack[] = {0.0f, 0.0f, 0.0f, 1.0f};
	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);*/

}

void GLView::DrawToon(int Color, M3DVector4f Light)
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glCullFace(GL_BACK);

	//glUseProgram(shader);
	//glUniform3fv(vLight, 1, Light);
	//glUniformMatrix4fv(MVP, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	//glUniformMatrix4fv(MV, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	//glUniformMatrix3fv(normal, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
	//glUniform1i(Color,0);
	//switch (Color)
	//{
	//case 1:
	//	glBindTexture(GL_TEXTURE_1D, text[0]);
	//	break;
	//case 2:
	//	glBindTexture(GL_TEXTURE_1D, text[1]);
	//	break;
	//}

}

void GLView::LoadBorder(void)
{
	fOrtho.SetOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	M3DVector3f HUD[] = {	-1.0f,	1.0f,	0.0f,
		-1.0f, -1.0f,	0.0f,
		1.0f,	-1.0f,	0.0f,
		1.0f,	1.0f,	0.0f };

	M3DVector2f vTexCoords[] = {	0.0f, 1.0f,
		0.0f, 0.0f, 
		1.0f, 0.0f,
		1.0f, 1.0f };

	HUDBatch.Begin(GL_TRIANGLE_FAN, 4, 1);
	HUDBatch.CopyTexCoordData2f(vTexCoords, 0);
	HUDBatch.CopyVertexData3f(HUD);
	HUDBatch.End();

	glGenTextures(3, tHandles);
	glBindTexture(GL_TEXTURE_2D, tHandles[0]);
	LoadTGAAsTexture("legoBorder.tga", GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void GLView::DrawBorder(void)
{
	M3DVector4f vWhite =  { 1.0f, 1.0f, 1.0f, 0.5f };
	modelViewMatrix.PushMatrix();
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, tHandles[0]);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, fOrtho.GetProjectionMatrix(), 0);

		HUDBatch.Draw();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	modelViewMatrix.PopMatrix();
}
//place new function here

///////////////////////////////////////////////////////////////////////////////
// Render a frame. The owning framework is responsible for buffer swaps,
// flushes, etc.
void GLView::Render(void)
{
	static CStopWatch timer;
	float yRot = timer.GetElapsedSeconds() * 60.0f;
	M3DVector4f vLightPosWorld = { 0.0f, 1.0f, 0.0f, 1.0f };
	M3DVector4f vLightPosEye;

	M3DMatrix44f mCamera;

	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightPosEye, vLightPosWorld, mCamera);
	cameraFrame.GetCameraMatrix(mCamera);
	MoveCamera();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	M3DVector4f vWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.MultMatrix(mCamera);
		modelViewMatrix.Translate(0,-0.5f,-5.0f);	//this places the objects back -20 not in your face.
		
		modelViewMatrix.PushMatrix();
		{
			//modelViewMatrix.Translate(-0.05f, 1.145f, -0.075f);
			//modelViewMatrix.Rotate(yRot, 0.0f, 0.0f, 1.0f);
			DrawFerrisWheelRotate();
		}
		modelViewMatrix.PopMatrix();
		DrawFerrisWheel();

		// Draw stuff relative to the camera
		modelViewMatrix.PushMatrix();
		{
			//shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vWhite, 0);
			DrawFloor();
		}
		modelViewMatrix.PopMatrix();
		DrawBricksSouth();
		DrawBricksEast();
		DrawBricksNorth();
		DrawBricksWest();
		//this doesnt work.... 
		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);	//drives in a circle
			DrawCar();
		}
		modelViewMatrix.PopMatrix();


		//SKYBOX------------------------------------------------
		modelViewMatrix.PushMatrix();	
		{
			modelViewMatrix.Translate(0,15.0f,0);	//this places the objects back -20 not in your face.

			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			//glDisable(GL_DEPTH_TEST);
			glUseProgram(skyBoxShader);
			glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
			cubeBatch.Draw();
			glFrontFace(GL_CW);
			glCullFace(GL_FRONT);
			//glEnable(GL_DEPTH_TEST);
		}
		modelViewMatrix.PopMatrix();

		//WINDSHIELD
		GLfloat vClear[] = { 1.0f, 1.0f, 1.0f, 0.35f };

		modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);	//drives in a circle

			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_FRONT);
				modelViewMatrix.Translate(-5.0f, 0.25f, -0.15f);
				modelViewMatrix.Rotate(-45, 1.0f, 0.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carBrick.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();
			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_BACK);
				modelViewMatrix.Translate(-5.0f, 0.25f, -0.15f);
				modelViewMatrix.Rotate(-45, 1.0f, 0.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carBrick.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();
			//HEADLIGHTS
			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_FRONT);
				modelViewMatrix.Translate(-5.14f, 0.3f, -0.15f);
				modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carWindshieldTriangle.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();
			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_BACK);
				modelViewMatrix.Translate(-5.14f, 0.3f, -0.15f);
				modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carWindshieldTriangle.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();

			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_FRONT);
				modelViewMatrix.Translate(-4.87f, 0.3f, -0.15f);
				modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carWindshieldTriangle.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();
			modelViewMatrix.PushMatrix();
			{
				glEnable (GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glCullFace(GL_BACK);
				modelViewMatrix.Translate(-4.87f, 0.3f, -0.15f);
				modelViewMatrix.Rotate(90, 0.0f, 1.0f, 0.0f);
				shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPosEye, vClear);
				carWindshieldTriangle.Draw();
				glDisable(GL_BLEND);
			}
			modelViewMatrix.PopMatrix();
		}
		modelViewMatrix.PopMatrix();

				modelViewMatrix.PushMatrix();
		{
			modelViewMatrix.Translate(2.0f, 0.0f, 2.0f);
			modelViewMatrix.Scale(0.5f, 0.5f, 0.5f);
			DrawLegoDude();
		}
		modelViewMatrix.PopMatrix();
		DrawBorder();	
	}
	modelViewMatrix.PopMatrix();

}