#include "webcore.h"
#include <osgGA/GUIEventAdapter>

#include "cefclient/client_handler.h"
#include "include/cef_render_handler.h"
#include "include/cef_runnable.h"
#include "include/cef_browser.h"

namespace app {

    static const int max_string = 100;
    static TCHAR szOSRWindowClass[max_string] = "CEFClientWindowClass";

    struct WebCoreImpl
    {
        WebCoreImpl()
            : hinst(0L), main_wnd(0L)
        {
            client_handler = new ClientHandler();
        }

        CefRefPtr<ClientHandler> client_handler;
        HINSTANCE hinst;
        HWND main_wnd;
    };

    // ----------------------------- WebView

    struct WebViewImpl : public CefRenderHandler
    {
        WebViewImpl(WebCore* webcore_)
            : webcore(webcore_->_i), width(640), height(480)
        {
            pbo = new osg::PixelBufferObject;
            image = new osg::Image();

            if (!image->getPixelBufferObject())
            {
                pbo->setCopyDataAndReleaseGLBufferObject(true);
                pbo->setUsage(GL_DYNAMIC_DRAW_ARB);
                image->setPixelBufferObject(pbo.get());
            }
        }

        void resize(int w, int h)
        {
            width = w;
            height = h;

            CefRefPtr<CefBrowserHost> browser = get_browser_host();
            if(browser.get() != 0L) {
                browser->WasResized();
            }
        }

        bool send_key_event(int key, bool key_down)
        {
            CefRefPtr<CefBrowserHost> browser = get_browser_host();
            if(browser.get() == 0L) {
                return false;
            }
            CefKeyEvent event;
            //event.windows_key_code = wParam;
            event.native_key_code = key;
            event.is_system_key = false;

            if (key_down)
                event.type = KEYEVENT_RAWKEYDOWN;
            else
                event.type = KEYEVENT_KEYUP;

            if (browser.get())
                browser->SendKeyEvent(event);     
        }

        bool send_pointer_event( int x, int y, int button_mask )
        {
            CefRefPtr<CefBrowserHost> browser = get_browser_host();
            if(browser.get() == 0L) {
                return false;
            }
            switch ( button_mask )
            {
            case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
                if ( !last_button_mask ) {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                    CefBrowserHost::MouseButtonType btnType = MBT_LEFT;
                    int last_click_count = 1;
                    // Todo: try correct.
                    browser->SendMouseClickEvent(mouse_event, btnType, false, last_click_count); 
                }
                else {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    // mouse_event.modifiers = GetCefMouseModifiers(wParam);
                    browser->SendMouseMoveEvent(mouse_event, false);
                }
                break;
            case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
                if ( !last_button_mask ) {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                    CefBrowserHost::MouseButtonType btnType = MBT_MIDDLE;
                    int last_click_count = 1;
                    // Todo: try correct.
                    browser->SendMouseClickEvent(mouse_event, btnType, false, last_click_count); 
                }
                else {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    // mouse_event.modifiers = GetCefMouseModifiers(wParam);
                    browser->SendMouseMoveEvent(mouse_event, false);
                }
                break;
            case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
                if ( !last_button_mask ) {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                    CefBrowserHost::MouseButtonType btnType = MBT_RIGHT;
                    int last_click_count = 1;
                    // Todo: try correct.
                    browser->SendMouseClickEvent(mouse_event, btnType, false, last_click_count); 
                }
                else {
                    CefMouseEvent mouse_event;
                    mouse_event.x = x;
                    mouse_event.y = y;
                    // mouse_event.modifiers = GetCefMouseModifiers(wParam);
                    browser->SendMouseMoveEvent(mouse_event, false);
                }
                break;
            default:
                CefMouseEvent mouse_event;
                mouse_event.x = x;
                mouse_event.y = y;
                // mouse_event.modifiers = GetCefMouseModifiers(wParam);
                browser->SendMouseMoveEvent(mouse_event, false);
                break;
            }

            if ( last_button_mask!=0 && button_mask==0 )
            {
                switch ( last_button_mask )
                {
                case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
                    {
                        CefMouseEvent mouse_event;
                        mouse_event.x = x;
                        mouse_event.y = y;
                        //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                        CefBrowserHost::MouseButtonType btnType = MBT_LEFT;
                        int last_click_count = 1;
                        browser->SendMouseClickEvent(mouse_event, btnType, true, last_click_count); 
                    }
                    break;
                case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
                    {
                        CefMouseEvent mouse_event;
                        mouse_event.x = x;
                        mouse_event.y = y;
                        //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                        CefBrowserHost::MouseButtonType btnType = MBT_MIDDLE;
                        int last_click_count = 1;
                        // Todo: try correct.
                        browser->SendMouseClickEvent(mouse_event, btnType, true, last_click_count); 
                    }
                    break;
                case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
                    {
                        CefMouseEvent mouse_event;
                        mouse_event.x = x;
                        mouse_event.y = y;
                        //mouse_event.modifiers = GetCefMouseModifiers(wParam);

                        CefBrowserHost::MouseButtonType btnType = MBT_RIGHT;
                        int last_click_count = 1;
                        // Todo: try correct.
                        browser->SendMouseClickEvent(mouse_event, btnType, true, last_click_count); 
                    }

                    break;
                }
            }

            last_button_mask = button_mask;
            return true;
        }

        // CefRenderHandler methods
        virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
            CefRect& rect)
        {
            RECT window_rect = {0};

            HWND root_window = ::GetDesktopWindow();
            if (::GetWindowRect(root_window, &window_rect)) {
                rect = CefRect(window_rect.left,
                    window_rect.top,
                    window_rect.right - window_rect.left,
                    window_rect.bottom - window_rect.top);
                return true;
            }
            return false;     
        }

        virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
            CefRect& rect)
        {
            rect.x = rect.y = 0;
            rect.width = width;
            rect.height = height;
            return true; 
        }

        virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
            int viewX,
            int viewY,
            int& screenX,
            int& screenY)
        {
            // Convert the point from view coordinates to actual screen coordinates.
            POINT screen_pt = {viewX, viewY};
            screenX = screen_pt.x;
            screenY = screen_pt.y;
            return true; 
        }

        virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
            CefScreenInfo& screen_info)
        {
            screen_info.depth = 32;
            screen_info.depth_per_component = 8;
            return true;
        }

        virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
        {
        }

        virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
            const CefRect& rect)
        {
            if (rect.width <= 0 || rect.height <= 0)
                return;
        }

        virtual void OnPaint(CefRefPtr<CefBrowser> browser,
            PaintElementType type,
            const RectList& dirtyRects,
            const void* buffer,
            int width,
            int height)
        {
            if(type == PET_VIEW) {
                image->setImage( width, height, 1, 4, GL_BGRA, GL_UNSIGNED_BYTE,
                    (unsigned char*)(buffer), osg::Image::NO_DELETE );
            }
        }

        virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
            CefCursorHandle cursor)
        {
        }

        ///
        // Called when the scroll offset has changed.
        ///
        /*--cef()--*/
        virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser) 
        {
        }

        CefRefPtr<CefBrowserHost> get_browser_host()
        {
            CefRefPtr<CefBrowserHost> browser;
            CefRefPtr<CefBrowser> browser_ = webcore->client_handler->GetBrowser();
            if(browser_) {
                browser = browser_->GetHost();
                return browser;
            }
            else {
                return 0L;
            }
        }

        WebCoreImpl* webcore;
        osg::ref_ptr<osg::Image> image;
        osg::ref_ptr<osg::PixelBufferObject> pbo;

        int width, height;
        bool render_task_pending;
        int last_button_mask;

        IMPLEMENT_REFCOUNTING(WebViewImpl);

    };

    WebView::WebView(WebCore* webcore, int w, int h)
        : _i(new WebViewImpl(webcore))
    {
    }

    void WebView::load_url(const char* url) 
    {
    }

    void WebView::resize(int w, int h)
    {    
        _i->resize(w,h);
    }



    bool WebView::send_pointer_event( int x, int y, int button_mask )
    {
        return _i->send_pointer_event(x,y,button_mask);
    }

    bool WebView::send_key_event(int key, bool key_down)
    {
        return _i->send_key_event(key, key_down);
        return true;
    }

    int WebView::width() const
    {
        return _i->width;
    }

    int WebView::height() const
    {
        return _i->height;
    }

    osg::Image* WebView::get_image()
    {
        return _i->image;
    }

    WebView::~WebView()
    {
        delete _i;
    }

    // ----------------------------- WebCore

    WebCore::WebCore()
        : _i(new WebCoreImpl())
    {
    }

    void WebCore::init(void* hinst, void* main_wnd)
    {
        _i->hinst = (HINSTANCE)hinst;
        _i->main_wnd = (HWND)main_wnd;
    }

    WebCore::~WebCore()
    {
        if(_i) {
            delete _i;
        }
    }

    WebView* WebCore::create_view(int w, int h) 
    {
        bool transparent = true;
        WebView* web_view = new WebView(this, w, h);

        _i->client_handler->set_render_handler(web_view->_i);

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = w;
        rect.bottom = h;

        CefWindowInfo info;
        CefBrowserSettings settings;
        info.SetAsOffScreen(0L); // for popups: _i->main_wnd
        settings.file_access_from_file_urls = STATE_ENABLED;
        settings.plugins = STATE_DISABLED;
        settings.webgl = STATE_DISABLED;
        settings.java = STATE_DISABLED;
        settings.caret_browsing = STATE_ENABLED;
        info.SetTransparentPainting(transparent ? TRUE : FALSE);

        // Creat the new child browser window
        CefBrowserHost::CreateBrowser(info,  _i->client_handler.get(),
            _i->client_handler->get_startup_url(), settings);

        return web_view;
    }
        

}; // end of namespace app


