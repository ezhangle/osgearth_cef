#include "app_window.h"

#include <ios>

#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static void* hinst = 0L;

int main(int argc, char **argv);

int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow) 
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hinst = hInstance;
    return main(__argc, __argv);
}

int main(int argc, char** argv)
{
    //osgEarth::setNotifyLevel(osg::DEBUG_INFO);  // osg::NotifySeverity::DEBUG_INFO osg::INFO


    osg::DisplaySettings::instance()->setMinimumNumStencilBits( 8 ); 
    osg::DisplaySettings::instance()->setNumMultiSamples(8);

    int cores = osgEarth::Registry::capabilities().getNumProcessors();

    osg::DisplaySettings::instance()->setNumOfDatabaseThreadsHint( osg::clampAbove(cores, 2) );
    osg::DisplaySettings::instance()->setNumOfHttpDatabaseThreadsHint( osg::clampAbove(cores/2, 1) );

    AppWindow w;
    w.init((void*)hinst);

    w.show();

    return 0L;
}