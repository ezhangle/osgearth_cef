
#ifndef _CEF_CLIENT_HANDLER_H_
#define _CEF_CLIENT_HANDLER_H_
#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include "include/cef_client.h"
#include "util.h"

// ClientHandler implementation.
class ClientHandler : public CefClient,
                      public CefKeyboardHandler, 
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
					  public CefRequestHandler, /* for custom scheme*/
                      public CefRenderHandler 
{
 public:

  ClientHandler();
  virtual ~ClientHandler();

  // CefClient methods
  virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  } 
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
    return this;
  } 
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
    return this;
  } 

// CefLifeSpanHandler methods
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
 
  // CefLoadHandler methods
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;

  // CefRequestHandler methods
#if 0
  virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
      CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request) OVERRIDE;
#endif
  virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                   const CefString& url,
                                   bool& allow_os_execution) OVERRIDE;

  // CefRenderHandler methods
  virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
                                 CefRect& rect) OVERRIDE;
  virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect) OVERRIDE;
  virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
                              int viewX,
                              int viewY,
                              int& screenX,
                              int& screenY) OVERRIDE;
  virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
                             CefScreenInfo& screen_info) OVERRIDE;
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer,
                       int width,
                       int height) OVERRIDE;

  CefRefPtr<CefBrowser> GetBrowser() { 
      return m_Browser; 
  }
    
  void set_render_handler(CefRenderHandler* handler);
  const char* get_startup_url();
  void set_startup_url(const char* startup_url);

  void ShowDevTools(CefRefPtr<CefBrowser> browser);

 private:
  CefRefPtr<CefRenderHandler> m_render_handler;
  std::string m_startup_url;

 protected:
  // The child browser window
  CefRefPtr<CefBrowser> m_Browser;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(ClientHandler);
  // Include the default locking implementation.
  IMPLEMENT_LOCKING(ClientHandler);
};

#endif  // _CEF_CLIENT_HANDLER_H_
