#include "client_app.h"  
#include "../scheme_handler.h"

ClientApp::ClientApp() {
}

void ClientApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) 
{
    app::register_scheme(registrar, cookieable_schemes_);
}

