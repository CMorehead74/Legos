// GLMain.cpp
// Program entry point and main control is here.
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "GLView.h"
#include <gl\wglew.h>


// Our instance of the OpenGL renderer
GLView	glRenderer;

//////////////////////////////////////////////////////////////////////////////
// Globals
// These may be shared across modules
HINSTANCE	ghInstance = NULL;		// Application instance
HWND		ghMainWnd = NULL;		// Main Window (window manager)
DEVMODE		gDevMode;

HDC	  g_hDC = NULL;		// GDI Device context
HGLRC g_hRC = NULL;	    // OpenGL Rendering context


#ifdef UNICODE
const wchar_t szGameName[] = TEXT("Project");
#else
const char szGameName[] = "Project";
#endif

//////////////////////////////////////////////////////////////////////////////
// Forward  and external Declarations
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////////////
// Program Entry Point
//////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
	{
	/////////////////////////////////////////////
	// Do any program wide Initialization here
	// Change display settings 
	


	/////////////////////////////////////////////
	// Create Main Window. 
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= (HCURSOR)LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("OpenGLWin32Window"); // Should be unique
	wcex.hIconSm		= NULL;

	if(RegisterClassEx(&wcex) == 0)
		return -1;

	// Select window styles
	UINT uiStyle,uiStyleX;
	uiStyle = WS_OVERLAPPEDWINDOW;
	uiStyleX = NULL;

	// Create the main 3D window
	ghMainWnd = CreateWindowEx(uiStyleX, wcex.lpszClassName, szGameName, uiStyle,
      0, 0, glRenderer.GetWidth(), glRenderer.GetHeight(), NULL, NULL, hInstance, NULL);

	// Get DC and set up Pixel Format Descriptor
	HDC hDC = GetDC(ghMainWnd);

	//Dummy Descriptor... Old school OpenGL initialization
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),		// Size of struct
		1,									// Version of struct
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA,	            // Always put this
		32,				            // Always put this, size of the color buffer
		0,0,0,0,0,0,0,0,0,0,0,0,0,  // You have no control over
		16,					        // Depth of the Z buffer
		8,				            // Stencil buffer
		0,0,0,0,0,0 };	            // Either obsolete, or we don't care about

	// Set Dummy format so we can get access WGL extensions
	SetPixelFormat(hDC, 1, &pfd);
	g_hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, g_hRC);

	// Checck for errors
	if(!g_hRC  || !hDC)
		return -1;

	// This is needed to initialize the wglChoosePixelFormatARB
    // function.
    glewInit();


	// Now that we have checked extensions delete window and create new one with correct pixel format
	wglMakeCurrent(NULL,NULL);
	wglDeleteContext(g_hRC);
	ReleaseDC(ghMainWnd,hDC);

	if(glRenderer.GetFullScreen() == GL_TRUE)
	{
		gDevMode.dmPelsHeight = glRenderer.GetHeight();
		gDevMode.dmPelsWidth = glRenderer.GetWidth();
		gDevMode.dmSize = sizeof(DEVMODE);
		gDevMode.dmBitsPerPel = 32;
		gDevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&gDevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, TEXT("Cannot change to selected desktop resolution."),
							  NULL, MB_OK | MB_ICONSTOP);
			return -1;
		}

		uiStyle = WS_POPUP;
		uiStyleX = WS_EX_APPWINDOW;

	}

	// Create New Window
	// Create the window again
    ghMainWnd = CreateWindowEx(	uiStyleX,     // Extended style
								wcex.lpszClassName, 
								szGameName, 
								uiStyle,// window stlye
								10,       // window position, x
								10,       // window position, y
								glRenderer.GetWidth(),  // width
								glRenderer.GetHeight(),   // height
								NULL,           // Parent window
								NULL,           // menu
								hInstance,    // instance
								NULL);          // pass this to WM_CREATE

	

	     
    g_hDC = GetDC(ghMainWnd);
    glRenderer.Resize(glRenderer.GetWidth(), glRenderer.GetHeight());

    int nPixCount = 0;
	int nPixelFormat = -1;

    // Specify the important attributes we care about
    int pixAttribs[] = { WGL_SUPPORT_OPENGL_ARB, 1, // Must support OGL rendering
                         WGL_DRAW_TO_WINDOW_ARB, 1, // pf that can run a window
                         //WGL_ACCELERATION_ARB,   1, // must be HW accelerated
                         WGL_COLOR_BITS_ARB,     24, // 8 bits of each R, G and B
                         WGL_DEPTH_BITS_ARB,     8, // 16 bits of depth precision for window
                         WGL_DOUBLE_BUFFER_ARB,	 GL_TRUE, // Double buffered context
						 WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // MSAA on
						 WGL_SAMPLES_ARB,        glRenderer.GetSamples(), // 8x MSAA 
                         WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB, // pf should be RGBA type
						 WGL_STENCIL_BITS_ARB, 8,
                         0}; // NULL termination

    // Ask OpenGL to find the most relevant format matching our attribs
    // Only get one format back.
    wglChoosePixelFormatARB(g_hDC, &pixAttribs[0], NULL, 1, &nPixelFormat, (UINT*)&nPixCount);

	if(nPixelFormat != -1)
	    {
		// Check for MSAA
        int attrib[] = { WGL_SAMPLES_ARB };
        int nResults = 0;
        wglGetPixelFormatAttribivARB(g_hDC, nPixelFormat, 0, 1, attrib, &nResults);
 
        // Got a format, now set it as the current one
        SetPixelFormat( g_hDC, nPixelFormat, &pfd );

        GLint attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
	                       WGL_CONTEXT_MINOR_VERSION_ARB,  0,
                           0 };
        
        g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs);
        if (g_hRC == NULL)
            {
            MessageBox(NULL, "Could not create an OpenGL 3.0 context.", NULL, MB_OK);
            return -1;
            }

        wglMakeCurrent( g_hDC, g_hRC );
	    }

	glRenderer.Initialize();

	if (!ghMainWnd)
		return -1;

	// Make sure window manager stays hidden
	ShowWindow(ghMainWnd, SW_SHOW);
	UpdateWindow(ghMainWnd);


	/////////////////////////////////////////////
	// Message Pump - Use the form that goes idle and waits for 
	// messages, not continually running.
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)) 
		{
		if(!TranslateMessage(&msg)) 
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}


	/////////////////////////////////////////////
	// Do any program wide shutdown here
	// Restore Display Settings
	if(glRenderer.GetFullScreen() == GL_TRUE)
		ChangeDisplaySettings(NULL, 0);

	// Return termination code
	return msg.wParam;
	}


/////////////////////////////////////////////////////////////////////
// Main window message procedure. This is the window manager for
// the application
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, 
								WPARAM wParam, LPARAM lParam)
	{

	switch (message) 
		{
		// Window is created, the application is starting
		case WM_CREATE:
			{
	
			}
			break;

		case WM_PAINT:
			{
            if(g_hDC == NULL) {
                ValidateRect(hWnd, NULL);
                break;
                }

			glRenderer.Render();
			SwapBuffers(g_hDC);
			if(!glRenderer.GetAnimated())
				ValidateRect(hWnd, NULL);
			}
			break;

		case WM_SIZE:
            if(g_hDC != NULL)
    			glRenderer.Resize(LOWORD(lParam), HIWORD(lParam));
			break;

		// Window is destroyed, it's the end of the application
		case WM_DESTROY:
			// Cleanup
            if(g_hDC != NULL)
    			glRenderer.Shutdown();
	
     		wglMakeCurrent(g_hDC, NULL);
			wglDeleteContext(g_hRC);
			ReleaseDC(hWnd, g_hDC);
			PostQuitMessage(0);
			break;

		// Window is either full screen, or not visible
		case WM_ACTIVATE:
			{
			// Only handled when full screen mode
			if(glRenderer.GetFullScreen())
				{
				WINDOWPLACEMENT wndPlacement;
				wndPlacement.length = sizeof(WINDOWPLACEMENT);
				wndPlacement.flags = WPF_RESTORETOMAXIMIZED;
				wndPlacement.ptMaxPosition.x = 0;
				wndPlacement.ptMaxPosition.y = 0;
				wndPlacement.ptMinPosition.x = 0;
				wndPlacement.ptMinPosition.y = 0;
				wndPlacement.rcNormalPosition.bottom = gDevMode.dmPelsHeight;
				wndPlacement.rcNormalPosition.left = 0;
				wndPlacement.rcNormalPosition.top = 0;
				wndPlacement.rcNormalPosition.right = gDevMode.dmPelsWidth;

				if(LOWORD(wParam) == WA_INACTIVE)
					{
					wndPlacement.showCmd = SW_SHOWMINNOACTIVE;
					SetWindowPlacement(hWnd, &wndPlacement);
					ShowCursor(TRUE);
					}
				else
					{
					wndPlacement.showCmd = SW_RESTORE;
					SetWindowPlacement(hWnd, &wndPlacement);
					ShowCursor(FALSE);
					}
				}
			}
			break;

	
		// Handle keyboard input
		case WM_CHAR:
			// Close program on ESC key press
			if(wParam == 27)
				DestroyWindow(hWnd);
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	return 0;
	}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
