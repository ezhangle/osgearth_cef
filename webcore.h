#ifndef _CEF_WEBCORE_H_
#define _CEF_WEBCORE_H_

#include <osg/Image>

namespace app {

struct WebCoreImpl;
class WebView;

class WebCore
{
public:
    WebCore();
    ~WebCore();

    void init(void* hinst, void* main_wnd);

    WebView* create_view(int w, int h);

private:
    WebCoreImpl* _i;

    friend struct WebViewImpl;
};

struct WebViewImpl;

class WebView
{
public:
    WebView(WebCore* webcore, int w, int h);
    ~WebView();

    void load_url(const char* url);
    void resize(int w, int h);

    bool send_pointer_event( int x, int y, int button_mask );
    bool send_key_event(int key, bool key_down);

    int width() const;
    int height() const;

    osg::Image* get_image(); // for textures

    WebViewImpl* _i;
};

} // end of namespace app

#endif