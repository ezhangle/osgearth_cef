
#ifndef CEF_CLIENT_APP_H_
#define CEF_CLIENT_APP_H_
#pragma once

#include <string>
#include <vector>
#include "include/cef_app.h"

class ClientApp : public CefApp {
 public:

  ClientApp();

 private:
  // CefApp methods.
  virtual void OnRegisterCustomSchemes(
      CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;

  // Schemes that will be registered with the global cookie manager.
  std::vector<CefString> cookieable_schemes_;

  IMPLEMENT_REFCOUNTING(ClientApp);
};

#endif  
