#include "app_window.h"
#include "webcore.h"
#include "cef_control.h"

#include <osg/Camera>
#include <osg/NodeCallback>
#include <osg/CoordinateSystemNode>
#include <osg/Timer>
#include <osgViewer/Viewer>
#include <osgViewer/api/Win32/GraphicsWindowWin32>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgEarth/Map>
#include <osgEarth/MapOptions>
#include <osgEarth/Registry>
#include <osgEarth/MapNode>
#include <osgEarth/URI>
#include <osgEarth/TerrainEngineNode>
#include <osgEarthUtil/Controls>
#include "scheme_handler.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <direct.h>
#include <sstream>
#include <string>
#include <process.h>

#include "include/cef_runnable.h"
#include "include/cef_app.h"
#include "include/cef_origin_whitelist.h"

#include "cefclient/client_app.h"

namespace app {
    struct AppParams {
        osg::ref_ptr<osgViewer::Viewer> viewer;
        HINSTANCE hinst;
        app::WebCore webcore;
        bool finished;
    };
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

static const char* s_map_host = "http://www.google.com";

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static bool initialized = false;
    LONG ret = 1;

    app::AppParams* params = reinterpret_cast<app::AppParams*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch ( message )
    {
    case WM_CREATE:
        {
            if(params == 0L) {
                params = reinterpret_cast<app::AppParams*>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
            }
            
          	params->viewer = new osgViewer::Viewer();

		    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

            RECT rect;
		    ::GetWindowRect( hwnd, &rect );
		    osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData( hwnd );
		    traits->width = min(rect.right - rect.left,2048);
		    traits->height = min(rect.bottom - rect.top,2048);

            traits->inheritedWindowData = windata;
		    traits->doubleBuffer = true;
		    traits->windowDecoration = false;
	
		    traits->x = 0;
		    traits->y = 0;
		
		    if (traits->width == 0) traits->width = 1;
		    if (traits->height == 0) traits->height = 1;

		    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
            
       	    params->viewer->getCamera()->setGraphicsContext( gc.get() );
	        params->viewer->getCamera()->setViewport( new osg::Viewport( 0, 0, traits->width, traits->height ) );
	        params->viewer->getCamera()->setProjectionMatrixAsPerspective(30.0f, (double)(traits->width)/(double)(traits->height), 1.0f, 10000.0f);
	        params->viewer->getCamera()->setClearColor( osg::Vec4f(0.2f, 0.2f,0.6f, 1.0f) );
	        params->viewer->setThreadingModel(osgViewer::Viewer::DrawThreadPerContext); // SingleThreaded / AutomaticSelection / ThreadPerCamera AutomaticSelection
	        params->viewer->setKeyEventSetsDone(0);
            params->viewer->setQuitEventSetsDone(false);

	        params->viewer->getCamera()->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	        params->viewer->getCamera()->setClearStencil(128);

            ::SetTimer(hwnd,1,10, NULL);
            params->finished = false;

            params->webcore.init(params->hinst, hwnd);

            return 0;
        }
        break;
      case WM_TIMER: 
   
          switch (wParam) 
          { 
              case 1: 
                if (params->viewer->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS || 
                    params->viewer->checkNeedToDoFrame() )
                {
                    params->viewer->frame();
                }
          } 
          break;
  

    case WM_SHOWWINDOW:
        {
            if(!initialized) {
                RECT rect;
		        BOOL success = ::GetWindowRect( hwnd, &rect );

                initialized = true;
                int w = rect.right-rect.left;
                int h = rect.bottom - rect.top;
                //params->scene->setup_viewer(params->viewer, NULL, w, h);
				
                osg::ref_ptr<osg::Group> root = new osg::Group();
                params->viewer->setSceneData( root );
                Controls::ControlCanvas* cs = Controls::ControlCanvas::get(  params->viewer, false );
                root->addChild( cs );

                osg::ref_ptr<app::CEFControl> browser_control = new app::CEFControl( &params->webcore );
	            cs->addControl( browser_control.get());
            }
        }
        break;
    case WM_DESTROY:
        params->viewer->setDone( true );
        PostQuitMessage( 0 );
        return 0;
    default:
        ret = DefWindowProc( hwnd, message, wParam, lParam );
        break;
    }
    return ret;
}
// -----------------------------------------------------------------------------------------------------------

AppWindow::AppWindow()
{
    m_app_params = new app::AppParams();
}

AppWindow::~AppWindow()
{
    delete m_app_params;
    m_app_params = 0L;
}

int AppWindow::init(void* hinstance)
{
  m_app_params->hinst = (HINSTANCE)hinstance;

  CefMainArgs main_args(m_app_params->hinst);
  CefRefPtr<ClientApp> app(new ClientApp());

  int exit_code = CefExecuteProcess(main_args, app.get());
  if (exit_code >= 0) {
    // The sub-process terminated, exit now.
    return exit_code;
  }

  m_settings.pack_loading_disabled = true;
  CefInitialize(main_args, m_settings, app.get());

  // Register the scheme handler with document root.

  std::string html_doc_dir = osgDB::getCurrentWorkingDirectory();

  std::string base_path = html_doc_dir + "/../../web";
  std::string index_html = osgDB::findFileInDirectory("index.html",base_path);
  if(index_html.empty()) {
      base_path = html_doc_dir + "/../web";
      index_html = osgDB::findFileInDirectory("index.html",base_path);
  }

  if(!index_html.empty()) {    
    app::init_scheme(base_path.c_str());
  }
  else {
    OE_FATAL << "can't find index file." << std::endl;
    exit(1);
  }

  CefAddCrossOriginWhitelistEntry("sample://site", "http", "google.com", true);
  return 0;
}

LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam,
                                LPARAM lParam) {
  switch (message) {
    case WM_COMMAND: {
      int wmId = LOWORD(wParam);
      switch (wmId) {
      }
    }
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateMessageWindow(HINSTANCE hInstance) {
  static TCHAR kWndClass[] = TEXT("ClientMessageWindow");

  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = MessageWndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = kWndClass;
  RegisterClassEx(&wc);

  return CreateWindow(kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0,
                      hInstance, 0);
}
 
void AppWindow::show()
{
    static TCHAR szAppName[] = TEXT("gui");
    
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = sizeof(app::AppParams*);    
    wndclass.hInstance = 0;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    if ( !RegisterClass(&wndclass) )
        return;
    
    HWND hwnd = CreateWindow( szAppName, // window class name
                                TEXT("osgearth cef"), // window caption
                                WS_OVERLAPPEDWINDOW, // window style
                                0, // initial x position
                                0, // initial y position
                                800, // initial x size
                                600, // initial y size
                                NULL, // parent window handle
                                NULL, // window menu handle
                                m_app_params->hinst, // program instance handle
                                reinterpret_cast<LPVOID>(m_app_params) ); // creation parameters

    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(m_app_params));
 
    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );
    
    if(!m_settings.multi_threaded_message_loop) {
	    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called.
	    CefRunMessageLoop();

	    // Shut down CEF.
	    CefShutdown();
    }
    else {
        // Create a hidden window for message processing.
        m_hMessageWnd = CreateMessageWindow(m_app_params->hinst);
        //ASSERT(hMessageWnd);

        MSG msg;

        // Run the application message loop.
        while (GetMessage(&msg, NULL, 0, 0)) {
          if (!TranslateAccelerator(msg.hwnd, 0L, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }

        DestroyWindow((HWND)m_hMessageWnd);
        m_hMessageWnd = NULL;

        int result = static_cast<int>(msg.wParam); 
    }
}

