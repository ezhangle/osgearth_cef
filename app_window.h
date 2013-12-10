#ifndef _APP_WINDOW_H_
#define _APP_WINDOW_H_

#include "include/cef_base.h"

namespace app {
    struct AppParams;
};

namespace osg {
    class ArgumentParser;
}

class AppWindow 
{

public:
	AppWindow();
	~AppWindow();

	int init(void* hinstance);
	void show();
private:
    app::AppParams* m_app_params;
    void* m_hMessageWnd;
	CefSettings m_settings;
};

#endif 
