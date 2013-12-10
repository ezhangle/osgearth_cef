#include "client_handler.h"
#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_path_util.h"
#include "include/cef_process_util.h"
#include "include/cef_runnable.h"
#include "include/cef_url.h"
#include "include/wrapper/cef_stream_resource_handler.h"


ClientHandler::ClientHandler()
{
  m_Browser = 0L;
  if (m_startup_url.empty()) {
	m_startup_url = "sample://site/index.html"; 
  }
}

ClientHandler::~ClientHandler() {
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if (!m_Browser.get())   {
	m_Browser = browser;
  }
} 

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
  REQUIRE_UI_THREAD();

  m_Browser = NULL;
} 

void ClientHandler::set_startup_url(const char* startup_url)
{
  m_startup_url = startup_url;
}

const char* ClientHandler::get_startup_url()
{
  return m_startup_url.c_str();
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  ErrorCode errorCode,
  const CefString& errorText,
  const CefString& failedUrl) 
{
  REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
	return;

  // Don't display an error for external protocols that we allow the OS to
  // handle. See OnProtocolExecution().
  if (errorCode == ERR_UNKNOWN_URL_SCHEME) {
	std::string urlStr = frame->GetURL();
	if (urlStr.find("sample:") == 0)
	  return;
  }

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body><h2>Failed to load URL " << std::string(failedUrl) <<
	" with error " << std::string(errorText) << " (" << errorCode <<
	").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

// --- [ Request Handler] ---

namespace {

  // Retrieve the file name and mime type based on the specified url.
  bool parse_url(const std::string& url,
	std::string* file_name,
	std::string* mime_type) {
	  // Retrieve the path component.
	  CefURLParts parts;
	  CefParseURL(url, parts);
	  std::string file = CefString(&parts.path);
	  if (file.size() < 2)
		return false;

	  // Remove the leading slash.
	  file = file.substr(1);

	  // Verify that the file name is valid.
	  for(size_t i = 0; i < file.size(); ++i) {
		const char c = file[i];
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '.')
		  return false;
	  }

	  // Determine the mime type based on the file extension, if any.
	  size_t pos = file.rfind(".");
	  if (pos != std::string::npos) {
		std::string ext = file.substr(pos + 1);
		if (ext == "html")
		  *mime_type = "text/html";
		else if (ext == "png")
		  *mime_type = "image/png";
		else
		  return false;
	  } else {
		// Default to an html extension if none is specified.
		*mime_type = "text/html";
		file += ".html";
	  }

	  *file_name = file;
	  return true;
  }

} // end of anonymous namespace
void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
  const CefString& url,
  bool& allow_os_execution)
{
  std::string urlStr = url;

  if (urlStr.find("sample:") == 0)
	allow_os_execution = true;
}

// --- [ Render Handler] ---
void ClientHandler::set_render_handler(CefRenderHandler* handler)
{
  m_render_handler = handler;
}

bool ClientHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser,
  CefRect& rect) 
{
  if(m_render_handler.get() == 0L) {
	return false;
  }

  return m_render_handler->GetRootScreenRect(browser, rect);
}

bool ClientHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) 
{
  if(m_render_handler.get() == 0L) {
	return false;
  }

  return m_render_handler->GetViewRect(browser, rect);
}

bool ClientHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser,
  int viewX,
  int viewY,
  int& screenX,
  int& screenY) 
{
  if(m_render_handler.get() == 0L) {
	return false;
  }

  return m_render_handler->GetScreenPoint(browser, viewX, viewY, screenX, screenY);
}

bool ClientHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser,
  CefScreenInfo& screen_info) 
{
  if(m_render_handler.get() == 0L) {
	return false;
  }

  return m_render_handler->GetScreenInfo(browser, screen_info);
}

void ClientHandler::OnPaint(CefRefPtr<CefBrowser> browser,
  PaintElementType type,
  const RectList& dirtyRects,
  const void* buffer,
  int width,
  int height) 
{
  if(m_render_handler.get() != 0L) {
	m_render_handler->OnPaint(browser, type, dirtyRects, buffer, width, height);
  }
}


void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser) {
 
}


