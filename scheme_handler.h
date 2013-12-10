#ifndef SCHEME_HANDLER_H_
#define SCHEME_HANDLER_H_
#pragma once

#include "include/internal/cef_ptr.h"
#include "include/internal/cef_types_wrappers.h"
#include "include/cef_scheme.h"

namespace app {

void register_scheme(CefRefPtr<CefSchemeRegistrar> registrar, std::vector<CefString>& cookiable_schemes);

void init_scheme(const char* doc_root);

} // end of namespace

#endif