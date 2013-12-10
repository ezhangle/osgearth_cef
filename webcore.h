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

class CEFImage : public osg::Image
{
public:    
    CEFImage(WebCore* web_core);
    
    CEFImage( const CEFImage& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY );
    
    CEFImage();
    META_Object( osg, CEFImage )
    
    void loadURL( const std::string& url, int w=0, int h=0 );
    
    virtual bool requiresUpdateCall() const;
    
    virtual void update( osg::NodeVisitor* nv );    

protected:
    virtual ~CEFImage();
  
    WebView* _webView;
    WebCore* _web_core;
};

} // end of namespace app

#endif