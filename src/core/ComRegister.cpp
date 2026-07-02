#include "ComRegister.h"

#ifdef _WIN32

#include "logger.h"

namespace tip {

// {TIP_IME_CLSID_PLACEHOLDER}
const CLSID CLSID_TIPTextService = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };

HRESULT RegisterCOM() {
    // TODO: implement registry entries for TSF registration
    TIP_LOG_INFO(L"RegisterCOM called");
    return S_OK;
}

HRESULT UnregisterCOM() {
    // TODO: remove registry entries
    TIP_LOG_INFO(L"UnregisterCOM called");
    return S_OK;
}

} // namespace tip

#endif // _WIN32
