#ifndef _CEF_CONTROL_H_
#define _CEF_CONTROL_H_

#include <osgEarthUtil/Controls>

using namespace osgEarth::Util::Controls;

namespace app {
class WebCore;
class WebView;

class CEFControl : public Control
{
public:
    CEFControl(WebCore* web_core);

    /** dtor */
    virtual ~CEFControl() { }

public: // Control
    virtual void calcSize( const ControlContext& context, osg::Vec2f& out_size ); 
    virtual void draw( const ControlContext& cx );

protected:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, ControlContext& cx );
private:
    WebCore* _web_core;
    WebView* _web_view;
    osg::Geometry* _geom;
    float _opacity;
};

} // end of namespace app
#endif