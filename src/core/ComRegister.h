#pragma once

#ifdef _WIN32

#include <windows.h>

namespace tip {

// {12345678-1234-1234-1234-56789ABCDEF0}
extern const CLSID CLSID_TIPTextService;

HRESULT RegisterCOM();
HRESULT UnregisterCOM();

} // namespace tip

#endif // _WIN32
