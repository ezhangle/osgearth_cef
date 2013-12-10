#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_resource_handler.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/cef_url.h"
#include "cefclient/util.h"
#include <algorithm>

namespace app {

    namespace {

        static char s_doc_root[512];

        bool parse_url(const std::string& url,
            std::string* file_name,
            std::string* mime_type)
        {
            CefURLParts parts;
            CefParseURL(url, parts);
            std::string file = CefString(&parts.path);
            if (file.size() < 2)
                return false;

            file = file.substr(1);

            size_t pos = file.rfind(".");
            if (pos != std::string::npos) {
                std::string ext = file.substr(pos + 1);
                if (ext == "html")
                    *mime_type = "text/html";	
                else if (ext == "js")
                    *mime_type = "application/javascript";
                else if (ext == "css")
                    *mime_type = "text/css";
                else if (ext == "png")
                    *mime_type = "image/png";
                else if (ext == "json")
                    *mime_type = "application/json";
                else if (ext == "woff")
                    *mime_type = "application/x-font-woff";
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

        // Implementation of the schema handler for sample:// requests.
        class SampleSchemeHandler : public CefResourceHandler 
        {
        public:
            SampleSchemeHandler() {}

            virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                CefRefPtr<CefCallback> callback)
                OVERRIDE 
            {
                REQUIRE_IO_THREAD();

                bool handled = false;

                AutoLock lock_scope(this);

                std::string url = request->GetURL();

                std::string file_name;

                if (parse_url(url, &file_name, &mime_type_)) {
                    std::string path = s_doc_root;
                    path += "/";
                    path += file_name;
                    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForFile(path);
                    if (stream.get()) {
                        handled = true;
                        m_stream_handler = new CefStreamResourceHandler(mime_type_, stream);
                    }
                }
                else {
                    handled = false;
                }

                if (handled) {
                    // Indicate the headers are available.
                    callback->Continue();
                    return true;
                }

                return false;
            }

            virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                int64& response_length,
                CefString& redirectUrl) OVERRIDE 
            {
                m_stream_handler->GetResponseHeaders(response, response_length, redirectUrl);
            }

            virtual void Cancel() OVERRIDE {
                if(m_stream_handler.get()) {
                    m_stream_handler->Cancel();
                }
            }

            virtual bool ReadResponse(void* data_out,
                int bytes_to_read,
                int& bytes_read,
                CefRefPtr<CefCallback> callback)
                OVERRIDE 
            {
                return m_stream_handler->ReadResponse(data_out, bytes_to_read, bytes_read, callback);
            }

        private:
            CefRefPtr<CefStreamResourceHandler> m_stream_handler;
            std::string mime_type_;

            IMPLEMENT_REFCOUNTING(SampleSchemeHandler);
            IMPLEMENT_LOCKING(SampleSchemeHandler);
        };

        // Implementation of the factory for for creating schema handlers.
        class SampleSchemeHandlerFactory : public CefSchemeHandlerFactory {
        public:
            // Return a new scheme handler instance to handle the request.
            virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                const CefString& scheme_name,
                CefRefPtr<CefRequest> request)
                OVERRIDE {
                    REQUIRE_IO_THREAD();
                    return new SampleSchemeHandler();
            }

            IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
        };

    }; // end of namespace

    void register_scheme(CefRefPtr<CefSchemeRegistrar> registrar, std::vector<CefString>& cookiable_schemes) 
    {
        registrar->AddCustomScheme("sample", true, false, false);
    }

    void init_scheme(const char* doc_root) 
    {
        strcpy(s_doc_root, doc_root);
        CefRegisterSchemeHandlerFactory("sample", "site",
            new SampleSchemeHandlerFactory());
    }

}  // namespace aps
