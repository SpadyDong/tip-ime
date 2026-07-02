#include "ComRegister.h"

#ifdef _WIN32

#include <new>
#include <windows.h>

#include "TextService.h"

namespace tip {

class ClassFactory : public IClassFactory {
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) {
            return E_INVALIDARG;
        }
        *ppv = nullptr;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
            *ppv = static_cast<IClassFactory*>(this);
        } else {
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&refCount_);
    }

    STDMETHODIMP_(ULONG) Release() override {
        LONG count = InterlockedDecrement(&refCount_);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    STDMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** ppv) override {
        if (!ppv) {
            return E_INVALIDARG;
        }
        *ppv = nullptr;
        if (outer) {
            return CLASS_E_NOAGGREGATION;
        }

        TextService* service = new (std::nothrow) TextService();
        if (!service) {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = service->QueryInterface(riid, ppv);
        service->Release();
        return hr;
    }

    STDMETHODIMP LockServer(BOOL lock) override {
        if (lock) {
            InterlockedIncrement(&serverLockCount_);
        } else {
            InterlockedDecrement(&serverLockCount_);
        }
        return S_OK;
    }

private:
    LONG refCount_ = 1;
    static LONG serverLockCount_;
};

LONG ClassFactory::serverLockCount_ = 0;

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

    if (!IsEqualGUID(rclsid, tip::CLSID_TIPTextService)) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    tip::ClassFactory* factory = new (std::nothrow) tip::ClassFactory();
    if (!factory) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();
    return hr;
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
