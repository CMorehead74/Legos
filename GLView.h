// GLView.h
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

#ifndef __GLVIEW__
#define __GLVIEW__

#include <gltools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <stdio.h>

class GLView
	{
	public:
		GLView();
		virtual ~GLView() {};

		void Initialize(void);							// Called after context is created
		void Shutdown(void);							// Called before context is destroyed
		void Resize(GLsizei nWidth, GLsizei nHeight);	// Called when window is resized, at least once when window is created
		void Render(void);								// Called to update OpenGL view
		
		// These methods are used by the calling framework. Set the appropriate internal
		// protected variables to tell the parent framework your desired configuration
		inline GLuint GetWidth(void) { return screenWidth; }
		inline GLuint GetHeight(void) { return screenHeight; }
		inline GLboolean GetFullScreen(void) { return bFullScreen; }
		inline GLboolean GetAnimated(void) { return bAnimated; }
		inline GLuint GetSamples(void) { return samples; }
			
	protected:
		GLsizei	 screenWidth;			// Desired window or desktop width
		GLsizei  screenHeight;			// Desired window or desktop height
		
		GLboolean bFullScreen;			// Request to run full screen
		GLboolean bAnimated;			// Request for continual updates

		GLuint    samples;

		GLShaderManager		shaderManager;			// Shader Manager
		GLMatrixStack		modelViewMatrix;		// Modelview Matrix
		GLMatrixStack		projectionMatrix;		// Projection Matrix
		GLFrustum			viewFrustum;			// View Frustum
		GLGeometryTransform	transformPipeline;		// Geometry Transform Pipeline
		GLFrame				cameraFrame;			// Camera frame

		void DrawOutline();
		void DrawToon(int Color, M3DVector4f Light);

		void MoveCamera();
		void DrawFloor();
		void DrawBricksSouth(/*M3DVector4f vLightPosWorld*/);
		void DrawBricksEast();
		void DrawBricksNorth();
		void DrawBricksWest();
		void DrawFerrisWheel();
		void DrawFerrisWheelRotate();
		GLBatch				groundBatch;
		GLTriangleBatch		legoCylinderBatch;
		GLTriangleBatch		legoDiskBatch;
		GLBatch				brickBatch4F;
		GLBatch				brickBatch4T;
		GLBatch				brickBatch2;
		GLBatch				brickBatchTopper;
		GLTriangleBatch		sphereBatch;


		//test for 0,0-------------------
		//GLTriangleBatch		torusBatch;
		//-------------------------------

		GLTriangleBatch		ferrisWheelBars;
		GLBatch				ferrisWheelBrick;
		GLBatch				ferrisWheelBrickTop;
		GLBatch				ferrisWheelBrickSide;
		GLBatch				ferrisWheel4x4;
		GLBatch				ferrisWheel4x4Side;
		GLBatch				ferrisWheel4x4Top;
		GLTriangleBatch		ferrisWheelRebar;
		GLTriangleBatch		ferrisWheelSupport;
		GLTriangleBatch		ferrisWheelSupport2;
		GLTriangleBatch		ferrisWheelDiskSupport;
		GLBatch				ferrisWheelSupportSide;
		GLBatch				ferrisWheelSupportSide2;
		GLBatch				ferrisWheelSupportTop;

		void LoadSkyBoxShader(void);
		void LoadSkyBox(void);

		GLBatch			cubeBatch;
		
		GLuint              cubeTexture;
		GLint               skyBoxShader;
		GLint				locMVPSkyBox;

		const char *		szCubeFaces[6];
		GLenum				cube[6];

		//Car-------------------------------		//idea phong just the car
		void DrawCar(void);
		GLBatch				carBrick;
		GLBatch				carBrickSide;
		GLBatch				carBrickTop;
		GLTriangleBatch		carWheel;
		GLTriangleBatch		carWheelDisk;
		GLBatch				carWindshieldTriangle;
		GLTriangleBatch		carHeadLights;

		GLFrustum			fOrtho;
		GLuint				tHandles[3];
		GLBatch				HUDBatch;

		void LoadBorder(void);
		void DrawBorder(void);

		GLBatch			treeBatch;
		GLuint			treeShader;
		GLint			TREEUniformMVP;

		void DrawLegoDude();

		//TOON SHADING...................
		//GLuint				text[5];
		//GLuint				shader;
		//GLint				vLight;
		//GLint				MVP;
		//GLint				MV;
		//GLint				normal;
		//GLint				color;


	};
		
#endif // __GLVIEW__
