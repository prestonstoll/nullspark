#include <windows.h>

#include "GL/glew.h"
#define GLUT_DISABLE_ATEXIT_HACK
#if defined(__MACH__)
#include <GLUT/glut.h>
#else
#include "GL/gl.h"
#endif
#include <GL/glut.h>

#include <fbxsdk.h>
#include <fbxfilesdk/kfbxio/kfbxiosettings.h>

#include "../../Source/nullspark.h"
#include "../../Source/nsUtil.h"
#include <string>

#include <tchar.h>
#include <boost/thread/thread.hpp>

namespace
{
	void ** OutputBuffer;
	unsigned int WindowWidth;
	unsigned int WindowHeight;
	unsigned int OutputWidth;
	unsigned int OutputHeight;
	bool bResized;
	bool bBackBuffer;
	bool	bNewFrame;
}

bool SetupPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
				  PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int iFormat = ChoosePixelFormat( hdc, &pfd );
	SetPixelFormat( hdc, iFormat, &pfd );
	return true;
}

KFbxNode * FirstValidNode(KFbxNode * Node)
{
	int childCount;

	KFbxNodeAttribute * NodeAttribute = Node->GetNodeAttribute();
	if(NodeAttribute != NULL && NodeAttribute->GetAttributeType() == KFbxNodeAttribute::eMESH)
	{
		return Node;
	}

	childCount = Node->GetChildCount();

	for(int i = 0; i < childCount; i++)
	{
		KFbxNode * Return;
		Return = FirstValidNode(Node->GetChild(i));

		if(Return != NULL)
			return Return;
	}
	return NULL;
}

nsAABB ComputeAABB(nsTriangle * triangle)
{
	nsAABB ReturnAABB;
	float MinProjection[3], MaxProjection[3], CurrentProjection[3];
	vec3 CurrentPoint;
	vec3 Direction[3];// = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	Direction[0].x = 1.0f;
	Direction[1].y = 1.0f;
	Direction[2].z = 1.0f;
	int VerticesCount;

	MinProjection[0] = FLT_MAX;
	MinProjection[1] = FLT_MAX;
	MinProjection[2] = FLT_MAX;
	MaxProjection[0] = -FLT_MAX;
	MaxProjection[1] = -FLT_MAX;
	MaxProjection[2] = -FLT_MAX;

	VerticesCount = 3;
	

	// TODO: Clean this up
	ReturnAABB.min.x = FLT_MAX;
	ReturnAABB.min.y = FLT_MAX;
	ReturnAABB.min.z = FLT_MAX;
	ReturnAABB.max.x = -FLT_MAX;
	ReturnAABB.max.y = -FLT_MAX;
	ReturnAABB.max.z = -FLT_MAX;

	for(unsigned int v = 0; v < 3; v++)
	{
		CurrentPoint = triangle->vertices[v]->point;
		for(unsigned int a = 0; a < 3; a++)
		{
			CurrentProjection[a] = dot(CurrentPoint, Direction[a]);

			if(CurrentProjection[a] < MinProjection[a])
			{
				MinProjection[a] = CurrentProjection[a];
				ReturnAABB.min[a] = CurrentPoint[a];
			}

			if(CurrentProjection[a] > MaxProjection[a])
			{
				MaxProjection[a] = CurrentProjection[a];
				ReturnAABB.max[a] = CurrentPoint[a];
			}
		}
	}

	return ReturnAABB;
}

std::string openfilename(char *filter = "All Files (*.*)\0*.*\0", HWND owner = NULL)
{
	OPENFILENAME ofn;
	char fileName[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "";

	std::string fileNameStr;

	if ( GetOpenFileName(&ofn) )
		fileNameStr = fileName;

	return fileNameStr;
}

int ImportUserSelectFBX( nsTriangle *** pTriangleData, unsigned int  * pTriangleCount )
{
	std::string fileName = openfilename("FBX Files (*.fbx)\0*.fbx\0");

	KFbxSdkManager* lSdkManager;
	KFbxIOSettings * ios;
	KFbxImporter* lImporter;
	KFbxScene* lScene;
	KFbxNode*	FBXNode;

	if(strcmp(fileName.c_str(), "") == 0)
		return -1;

	// Create the FBX SDK manager
	lSdkManager = KFbxSdkManager::Create();

	lScene = KFbxScene::Create(lSdkManager,"fbxScene");

	// Create an IOSettings object.
	ios = KFbxIOSettings::Create(lSdkManager, IOSROOT );
	lSdkManager->SetIOSettings(ios);

	// ... Configure the KFbxIOSettings object ...

	// Create an importer.
	lImporter = KFbxImporter::Create(lSdkManager, "");

	// Initialize the importer.
	bool lImportStatus = lImporter->Initialize(fileName.c_str(), -1, lSdkManager->GetIOSettings());

	if(lImportStatus)
	{
		lImporter->Import(lScene);

		FBXNode = lScene->GetRootNode();

		FBXNode = FirstValidNode(FBXNode);

		if(FBXNode)
		{
			KFbxNodeAttribute * NodeAttribute = FBXNode->GetNodeAttribute();

			if(NodeAttribute != NULL && NodeAttribute->GetAttributeType() == KFbxNodeAttribute::eMESH)
			{
				KFbxMesh * TargetMesh = (KFbxMesh *)NodeAttribute;
				if (!TargetMesh->IsTriangleMesh())
				{
					KFbxGeometryConverter meshConverter ( lSdkManager );
					TargetMesh = meshConverter.TriangulateMesh(TargetMesh);
				}
				// Mesh is final
				// Count
				unsigned int vertex_count = (unsigned int)TargetMesh->GetControlPointsCount();
				unsigned int indices_count = (unsigned int)TargetMesh->GetPolygonVertexCount();

				//mesh->pVertexCount = (unsigned int) vertex_count;

				// Size
				//mesh->pIndicesCount = indices_count;
				unsigned int triangle_count = indices_count / 3;
				*pTriangleCount = triangle_count;
				*pTriangleData = (nsTriangle **) malloc(sizeof(nsTriangle*) * triangle_count);

				// UV extration
				KFbxLayerElementUV* uvElement = TargetMesh->GetLayer(0)->GetUVs();
				KFbxLayerElementNormal* normalElement = TargetMesh->GetLayer(0)->GetNormals();
				KFbxVector2 uvs;
				KFbxVector4 normal;

				for(unsigned int i = 0; i < triangle_count; i++)
				{
					nsTriangle triangle;

					triangle.vertices = (nsVertex **) malloc(sizeof(nsVertex*) * 3);

					for(unsigned int v = 0; v < 3; v++)
					{
						triangle.vertices[v] = (nsVertex *) malloc(sizeof(nsVertex));
						unsigned int vertIndex = TargetMesh->GetPolygonVertices()[i * 3 + v];
						triangle.vertices[v]->point.x = static_cast<float>(TargetMesh->GetControlPointAt( vertIndex ).GetAt(0));
						triangle.vertices[v]->point.y = static_cast<float>(TargetMesh->GetControlPointAt( vertIndex ).GetAt(1));
						triangle.vertices[v]->point.z = static_cast<float>(TargetMesh->GetControlPointAt( vertIndex ).GetAt(2));

						normal = normalElement->GetDirectArray().GetAt(vertIndex);

						triangle.vertices[v]->normal.x = static_cast<float>(normal.GetAt(0));
						triangle.vertices[v]->normal.y = static_cast<float>(normal.GetAt(1));
						triangle.vertices[v]->normal.z = static_cast<float>(normal.GetAt(2));

						if(uvElement != NULL)
						{
							uvs = uvElement->GetDirectArray().GetAt(vertIndex);

							triangle.vertices[v]->u = static_cast<float>(uvs.GetAt(0));
							triangle.vertices[v]->v = static_cast<float>(uvs.GetAt(1));
						}
						else
						{
							triangle.vertices[v]->u = static_cast<float>(0.5f);
							triangle.vertices[v]->v = static_cast<float>(0.5f);
						}
					}
					vec3 ab, ac;
					ab = triangle.vertices[1]->point - triangle.vertices[0]->point;
					ac = triangle.vertices[2]->point - triangle.vertices[0]->point;
					triangle.normal = (vec3 *) malloc(sizeof(vec3));
					*triangle.normal = cross(ab, ac);
					*triangle.normal = normalize(*triangle.normal);

					triangle.pAABB = (nsAABB *) malloc(sizeof(nsAABB));
					*triangle.pAABB = ComputeAABB(&triangle);

					(*pTriangleData)[i] = (nsTriangle *) malloc(sizeof(nsTriangle));
					*((*pTriangleData)[i]) = triangle;
				}

				lImporter->Destroy();
				return 0; // SUCCESS
			}
			lImporter->Destroy();
			return 1; // STEP FAIL
		}
	}
	return -1;	// ERROR
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/*	Switch message, condition that is met will execute*/
	switch(message)
	{
		/*	Window is being created*/
		case WM_CREATE: 
			//VoxelConvertInterface::Inst()->InitWindow(hwnd);
			return 0;
			break;
		/*	Window is closing*/
		case WM_SYSCOMMAND:                     // Intercept System Commands
			{
				switch (wParam)                     // Check System Calls
				{
				case SC_SCREENSAVE:             // Screensaver Trying To Start?
				case SC_MONITORPOWER:               // Monitor Trying To Enter Powersave?
					return 0;                   // Prevent From Happening
				}
				break;                          // Exit
			}
		case WM_SIZE:                           // Resize The OpenGL Window
			{

					WindowWidth = LOWORD(lParam);
					WindowHeight = HIWORD(lParam);
					bResized = true;
					return 0;
			}

		case WM_CLOSE: 
			PostQuitMessage(0);
			return 0;
			break;
		default:
			break;
	}
	return (DefWindowProc(hwnd,message,wParam,lParam));
}

int NSRENDER(HDC hdc)
{
		HGLRC hglrc;

	if(!SetupPixelFormat(hdc))
	{
		MessageBox(0, "Error setting up pixel format", "Error", MB_OK);
		//ReleaseDC(hwnd, hdc);
		return false;
	}

	if(!(hglrc = wglCreateContext(hdc)))
	{
		MessageBox(0, "Error: wglCreateContext failed.","",0);
	}
	if(!wglMakeCurrent(hdc, hglrc))
	{
		MessageBox(0, "Error: wglMakeCurrent failed.","",0);
	}
	glewInit();

	glDisable( GL_DEPTH_TEST );
	glViewport(0,0,WindowWidth,WindowHeight);

	bBackBuffer = true;
	while(true)
	{
		if(bResized)
		{
			bResized = false;

			OutputWidth = WindowWidth;
			OutputHeight = WindowHeight;

			if(OutputBuffer[0] != NULL)
			{
				delete OutputBuffer[0];
				OutputBuffer[0] = new int[OutputWidth * OutputHeight];
			}
			if(OutputBuffer[1] != NULL)
			{
				delete OutputBuffer[1];
				OutputBuffer[1] = new int[OutputWidth * OutputHeight];
			}
		}
		// No? Let's put a task up
		nsSetOutputBuffer(OutputBuffer[bBackBuffer]);
		nsSetOutputFormat(NS_PIXEL_RGBA_8, OutputWidth, OutputHeight);
		nsRender( );

		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		glClear( GL_COLOR_BUFFER_BIT );

		glDrawPixels(OutputWidth, OutputHeight, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, (void *)OutputBuffer[bBackBuffer]);
		bBackBuffer = !bBackBuffer;
		SwapBuffers( hdc );
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{
	// Windows
	WNDCLASSEX  windowClass;		//window class
	HWND		hwnd;				//window handle
	MSG			msg;				//message
	bool		done;				//flag saying when app is complete

	bResized = false;
	bNewFrame = false;
	WindowWidth = 640;
	WindowHeight = 480;

	/*	Fill out the window class structure*/
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "NullSparkWindow";
	windowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	/*	Register window class*/
	if (!RegisterClassEx(&windowClass))
	{
		return 0;
	}
	/*	Class registerd, so now create window*/
	hwnd = CreateWindowEx(NULL,		//extended style
		"NullSparkWindow",			//class name
		"NullSpark Example",		//app name
		WS_OVERLAPPEDWINDOW |		//window style
		WS_VISIBLE |
		WS_SYSMENU,
		100,100,			//x/y coords
		WindowWidth,WindowHeight,			//width,height
		NULL,				//handle to parent
		NULL,				//handle to menu
		hInstance,			//application instance
		NULL);				//no extra parameter's
	/*	Check if window creation failed*/
	if (!hwnd)
		return 0;

	// OpenGL
	HDC hdc = GetDC(hwnd);
	HGLRC hglrc;

	if(!SetupPixelFormat(hdc))
	{
		MessageBox(0, "Error setting up pixel format", "Error", MB_OK);
		ReleaseDC(hwnd, hdc);
		return false;
	}

	if(!(hglrc = wglCreateContext(hdc)))
	{
		MessageBox(0, "Error: wglCreateContext failed.","",0);
	}
	if(!wglMakeCurrent(hdc, hglrc))
	{
		MessageBox(0, "Error: wglMakeCurrent failed.","",0);
	}
	glewInit();

	// CVR
	nsTriangle ** triangleBatch = NULL;
	unsigned int triCount = 0;



	int rockmanFullMeshID = -1;

	//std::string textureFileName = ImportUserSelectTexture();

	//if(textureFileName != "")
	//{
	//	rockmanTriangles.pTextureData = LoadTexture(textureFileName.c_str());
	//}
	

	if(ImportUserSelectFBX(&triangleBatch, &triCount) != -1)
	{
		rockmanFullMeshID = nsLoadTriangleData(triangleBatch, triCount);
	}
	/*else
	{
		cvrGridID = ImportUserSelectCVROCTREE();
		if(cvrGridID != -1)
			rockmanFullMeshID = cvrGridID;
	}*/

	

	if(rockmanFullMeshID == -1)
		return 0;

	
	unsigned int OutputWidth = WindowWidth;
	unsigned int OutputHeight = WindowHeight;
	bBackBuffer = false;
	bool bNewBuffer = true;

	OutputBuffer = (void **)malloc(sizeof(void *) * 2);
	OutputBuffer[0] = new int[OutputWidth * OutputHeight];
	OutputBuffer[1] = new int[OutputWidth * OutputHeight];
	NS_PIXEL_LAYOUT pixelLayout = NS_PIXEL_RGBA_8;

	float Transform;

	nsInit( );

	nsDrawMesh( rockmanFullMeshID, &Transform );

	done = false;
	boost::thread * nullSparkThread = new boost::thread(NSRENDER, hdc);

	while(!done)
	{
		PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
		if (msg.message == WM_QUIT) //check for a quit message
		{
			done = true; //if found, quit app
		}
		else
		{
			/*	Translate and dispatch to event queue*/
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}

	nullSparkThread->interrupt();
	delete[] OutputBuffer;
	nsShutdown( );
	return 0;
}