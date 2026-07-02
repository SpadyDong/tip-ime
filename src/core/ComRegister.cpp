#include "ComRegister.h"

#ifdef _WIN32

#include <msctf.h>
#include <strsafe.h>

#include "logger.h"

namespace tip {

// {12345678-1234-1234-1234-56789ABCDEF0}
const CLSID CLSID_TIPTextService = {
    0x12345678,
    0x1234,
    0x1234,
    { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 }
};

// {12345678-1234-1234-1234-56789ABCDEF1}
const GUID GUID_TIPProfile = {
    0x12345678,
    0x1234,
    0x1234,
    { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF1 }
};

namespace {

const wchar_t kDescription[] = L"TIP Pinyin IME";
const wchar_t kThreadingModel[] = L"Apartment";

HRESULT GetModulePath(wchar_t* path, DWORD size) {
    HMODULE module = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                            reinterpret_cast<LPCWSTR>(GetModulePath),
                            &module)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    DWORD len = GetModuleFileNameW(module, path, size);
    if (len == 0 || len >= size) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT SetRegistryString(HKEY key, const wchar_t* name, const wchar_t* value) {
    return RegSetValueExW(key, name, 0, REG_SZ,
                          reinterpret_cast<const BYTE*>(value),
                          static_cast<DWORD>((wcslen(value) + 1) * sizeof(wchar_t)));
}

HRESULT CreateKey(HKEY parent, const wchar_t* path, HKEY* outKey) {
    return RegCreateKeyExW(parent, path, 0, nullptr, REG_OPTION_NON_VOLATILE,
                           KEY_WRITE, nullptr, outKey, nullptr);
}

HRESULT RegisterClsid(const wchar_t* modulePath) {
    wchar_t clsidPath[256] = {};
    StringCchPrintfW(clsidPath, ARRAYSIZE(clsidPath),
                     L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                     CLSID_TIPTextService.Data1, CLSID_TIPTextService.Data2,
                     CLSID_TIPTextService.Data3, CLSID_TIPTextService.Data4[0],
                     CLSID_TIPTextService.Data4[1], CLSID_TIPTextService.Data4[2],
                     CLSID_TIPTextService.Data4[3], CLSID_TIPTextService.Data4[4],
                     CLSID_TIPTextService.Data4[5], CLSID_TIPTextService.Data4[6],
                     CLSID_TIPTextService.Data4[7]);

    HKEY key = nullptr;
    HRESULT hr = CreateKey(HKEY_CLASSES_ROOT, clsidPath, &key);
    if (FAILED(hr)) {
        return hr;
    }
    RegCloseKey(key);

    wchar_t serverPath[512] = {};
    StringCchPrintfW(serverPath, ARRAYSIZE(serverPath), L"%s\\InprocServer32", clsidPath);
    hr = CreateKey(HKEY_CLASSES_ROOT, serverPath, &key);
    if (FAILED(hr)) {
        return hr;
    }
    hr = SetRegistryString(key, nullptr, modulePath);
    if (FAILED(hr)) {
        RegCloseKey(key);
        return hr;
    }
    hr = SetRegistryString(key, L"ThreadingModel", kThreadingModel);
    RegCloseKey(key);
    if (FAILED(hr)) {
        return hr;
    }

    // Register TSF category (keyboard input processor).
    wchar_t categoryPath[512] = {};
    StringCchPrintfW(categoryPath, ARRAYSIZE(categoryPath),
                     L"%s\\Categories\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                     clsidPath,
                     GUID_TFCAT_TIP_KEYBOARD.Data1, GUID_TFCAT_TIP_KEYBOARD.Data2,
                     GUID_TFCAT_TIP_KEYBOARD.Data3, GUID_TFCAT_TIP_KEYBOARD.Data4[0],
                     GUID_TFCAT_TIP_KEYBOARD.Data4[1], GUID_TFCAT_TIP_KEYBOARD.Data4[2],
                     GUID_TFCAT_TIP_KEYBOARD.Data4[3], GUID_TFCAT_TIP_KEYBOARD.Data4[4],
                     GUID_TFCAT_TIP_KEYBOARD.Data4[5], GUID_TFCAT_TIP_KEYBOARD.Data4[6],
                     GUID_TFCAT_TIP_KEYBOARD.Data4[7]);
    hr = CreateKey(HKEY_CLASSES_ROOT, categoryPath, &key);
    if (FAILED(hr)) {
        return hr;
    }
    RegCloseKey(key);

    return S_OK;
}

HRESULT UnregisterClsid() {
    wchar_t clsidPath[256] = {};
    StringCchPrintfW(clsidPath, ARRAYSIZE(clsidPath),
                     L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                     CLSID_TIPTextService.Data1, CLSID_TIPTextService.Data2,
                     CLSID_TIPTextService.Data3, CLSID_TIPTextService.Data4[0],
                     CLSID_TIPTextService.Data4[1], CLSID_TIPTextService.Data4[2],
                     CLSID_TIPTextService.Data4[3], CLSID_TIPTextService.Data4[4],
                     CLSID_TIPTextService.Data4[5], CLSID_TIPTextService.Data4[6],
                     CLSID_TIPTextService.Data4[7]);
    RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidPath);
    return S_OK;
}

HRESULT RegisterTSFProfile(const wchar_t* modulePath) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return hr;
    }

    ITfInputProcessorProfileMgr* profileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfileMgr,
                          reinterpret_cast<void**>(&profileMgr));
    if (SUCCEEDED(hr)) {
        hr = profileMgr->RegisterProfile(CLSID_TIPTextService, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
                                         GUID_TIPProfile, kDescription,
                                         static_cast<ULONG>(wcslen(kDescription)),
                                         modulePath, static_cast<ULONG>(wcslen(modulePath)),
                                         0, 0, 0, 0);
        profileMgr->Release();
    }

    CoUninitialize();
    return hr;
}

HRESULT UnregisterTSFProfile() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return hr;
    }

    ITfInputProcessorProfileMgr* profileMgr = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfileMgr,
                          reinterpret_cast<void**>(&profileMgr));
    if (SUCCEEDED(hr)) {
        hr = profileMgr->UnregisterProfile(CLSID_TIPTextService,
                                           MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
                                           GUID_TIPProfile, 0);
        profileMgr->Release();
    }

    CoUninitialize();
    return hr;
}

} // namespace

HRESULT RegisterCOM() {
    wchar_t modulePath[MAX_PATH] = {};
    HRESULT hr = GetModulePath(modulePath, ARRAYSIZE(modulePath));
    if (FAILED(hr)) {
        TIP_LOG_ERROR(L"Failed to get module path");
        return hr;
    }

    hr = RegisterClsid(modulePath);
    if (FAILED(hr)) {
        TIP_LOG_ERROR(L"Failed to register CLSID");
        return hr;
    }

    hr = RegisterTSFProfile(modulePath);
    if (FAILED(hr)) {
        TIP_LOG_ERROR(L"Failed to register TSF profile");
        return hr;
    }

    TIP_LOG_INFO(L"RegisterCOM succeeded");
    return S_OK;
}

HRESULT UnregisterCOM() {
    UnregisterTSFProfile();
    UnregisterClsid();
    TIP_LOG_INFO(L"UnregisterCOM succeeded");
    return S_OK;
}

} // namespace tip

#endif // _WIN32
