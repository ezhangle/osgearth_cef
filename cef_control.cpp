#include "cef_control.h"
#include "webcore.h"

#include <osgDB/WriteFile>

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

using namespace osgEarth::Util::Controls;

namespace {
	osg::Geometry* newGeometry()
	{
		osg::Geometry* geom = new osg::Geometry();
		geom->setUseVertexBufferObjects( true );
		geom->setUseDisplayList( false );
		geom->setDataVariance( osg::Object::DYNAMIC );
		return geom;
	}
}

namespace app {


	// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CEFControl::CEFControl( WebCore* web_core ) :
_opacity      ( 1.0f ), _web_core(web_core)
{
	setPadding(0);
	setMargin(0);
	_web_view = web_core->create_view(200,200);
}

void CEFControl::calcSize(const ControlContext& cx, osg::Vec2f& out_size)
{
	if ( visible() == true )
	{

		_renderSize.set( 0, 0 );

		int w;
		int h;

		if(width().isSet() || height().isSet()) {
			w = width().value();
			h = height().value();
		}
		else {
			w = cx._vp->width();
			h = cx._vp->height();
		}

		if(_web_view->width() != w || _web_view->height() != h) { 
			_web_view->resize( w, h);
		} 

		_renderSize.set( w, h );

		out_size.set(
			margin().left() + margin().right() + _renderSize.x(),
			margin().top() + margin().bottom() + _renderSize.y() );

	}
	else
	{
		_renderSize.set( _web_view->width(), _web_view->height() );

		out_size.set(
			margin().left() + margin().right() + _renderSize.x(),
			margin().top() + margin().bottom() + _renderSize.y() );
	}
}

#undef CEFControl_TEXRECT


bool CEFControl::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, ControlContext& cx )
{
	if (ea.getHandled()) return false;

	bool handled = false;

	osg::Vec2f render_pos = renderPos();
	osg::Vec2f render_size = renderSize();


	int x = ea.getX();
	int y = cx._vp->height() - ea.getY();

	switch(ea.getEventType())
	{
	case(osgGA::GUIEventAdapter::RESIZE):
		{
			int w = ea.getWindowWidth();
			int h = ea.getWindowHeight();
			_web_view->resize(w,h);
			break;
		}
	case(osgGA::GUIEventAdapter::MOVE):
	case(osgGA::GUIEventAdapter::DRAG):
	case(osgGA::GUIEventAdapter::PUSH):
	case(osgGA::GUIEventAdapter::RELEASE):
		{
			osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);			
			x = x - render_pos.x();
			y = y - render_pos.y();
			_web_view->send_pointer_event(x, y, ea.getButtonMask());
			handled = true;
			break;
		}
	case(osgGA::GUIEventAdapter::KEYDOWN):
	case(osgGA::GUIEventAdapter::KEYUP):
		{
			osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
			_web_view->send_key_event(ea.getKey(), ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN);
			handled = true;
			break;
		}

	case(osgGA::GUIEventAdapter::FRAME):
		{
			osg::Timer_t time = osg::Timer::instance()->tick();

			//_web_core->Update();

			//aa.requestRedraw();
			aa.requestContinuousUpdate();
			//dirty();
			break;
		}
	}
	return handled;
}

void
	CEFControl::draw( const ControlContext& cx )
{
	clearGeode();
	if ( visible() && parentIsVisible() )
	{
		osg::Geometry* g = newGeometry();

		float rx = osg::round( _renderPos.x() );
		float ry = osg::round( _renderPos.y() );
		float vph = cx._vp->height();

		osg::Vec3Array* verts = new osg::Vec3Array(4);
		g->setVertexArray( verts );

		(*verts)[0].set( rx, vph - ry, 0 );
		(*verts)[1].set( rx, vph - ry - _renderSize.y(), 0 );
		(*verts)[2].set( rx + _renderSize.x(), vph - ry - _renderSize.y(), 0 );
		(*verts)[3].set( rx + _renderSize.x(), vph - ry, 0 );

		g->addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4 ) );

		osg::Vec4Array* c = new osg::Vec4Array(1);
		(*c)[0] = osg::Vec4f(1,1,1,_opacity);
		g->setColorArray( c );
		g->setColorBinding( osg::Geometry::BIND_OVERALL );

		bool flip = true;
		osg::Vec2Array* t = new osg::Vec2Array(4);

		(*t)[0].set( 0, flip? 0 : 1 );
		(*t)[1].set( 0, flip? 1 : 0 );
		(*t)[2].set( 1, flip? 1 : 0 );
		(*t)[3].set( 1, flip? 0 : 1 );
		osg::Texture2D* tex = new osg::Texture2D( _web_view->get_image() );

		g->setTexCoordArray( 0, t );

		tex->setResizeNonPowerOfTwoHint(false);

		tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
		tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
		tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

		g->getOrCreateStateSet()->setTextureAttributeAndModes( 0, tex, osg::StateAttribute::ON );
		g->getOrCreateStateSet()->setAttributeAndModes(new osg::Program(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED); 

		osg::TexEnv* texenv = new osg::TexEnv( osg::TexEnv::MODULATE );
		g->getStateSet()->setTextureAttributeAndModes( 0, texenv, osg::StateAttribute::ON );

		getGeode()->addDrawable( g );

		_dirty = false;
	}
}

} // end of namespace app