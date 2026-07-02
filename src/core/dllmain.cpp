#include "ComRegister.h"

#ifdef _WIN32

#include <windows.h>

#include "TextService.h"

namespace tip {

const CLSID CLSID_TIPTextService = { 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };

} // namespace tip

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (!ppv) {
        return E_INVALIDARG;
    }
    *ppv = nullptr;

    if (IsEqualGUID(rclsid, tip::CLSID_TIPTextService)) {
        // TODO: implement class factory
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() {
    return S_OK;
}

STDAPI DllRegisterServer() {
    return tip::RegisterCOM();
}

STDAPI DllUnregisterServer() {
    return tip::UnregisterCOM();
}

#else

// Non-Windows stub so the file is not empty on other platforms.
void tip_dllmain_stub() {}

#endif // _WIN32
