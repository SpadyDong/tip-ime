#pragma once

#ifdef _WIN32

#include <windows.h>

namespace tip {

HRESULT RegisterCOM();
HRESULT UnregisterCOM();

} // namespace tip

#endif // _WIN32
