#include "TextService.h"

#ifdef _WIN32

#include "logger.h"

namespace tip {

TextService::TextService()
    : refCount_(1)
    , threadMgr_(nullptr)
    , clientId_(0)
    , keyEventSinkCookie_(TF_INVALID_COOKIE)
    , threadMgrEventSinkCookie_(TF_INVALID_COOKIE) {
}

TextService::~TextService() {
}

STDMETHODIMP TextService::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) {
        return E_INVALIDARG;
    }
    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor)) {
        *ppv = static_cast<ITfTextInputProcessor*>(this);
    } else if (IsEqualIID(riid, IID_ITfKeyEventSink)) {
        *ppv = static_cast<ITfKeyEventSink*>(this);
    } else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink)) {
        *ppv = static_cast<ITfThreadMgrEventSink*>(this);
    } else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) TextService::AddRef() {
    return InterlockedIncrement(&refCount_);
}

STDMETHODIMP_(ULONG) TextService::Release() {
    LONG count = InterlockedDecrement(&refCount_);
    if (count == 0) {
        delete this;
    }
    return count;
}

STDMETHODIMP TextService::Activate(ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr_ = threadMgr;
    threadMgr_->AddRef();
    clientId_ = clientId;
    TIP_LOG_INFO(L"TextService activated");
    return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
    if (threadMgr_) {
        threadMgr_->Release();
        threadMgr_ = nullptr;
    }
    TIP_LOG_INFO(L"TextService deactivated");
    return S_OK;
}

STDMETHODIMP TextService::OnSetFocus(BOOL foreground) {
    (void)foreground;
    return S_OK;
}

STDMETHODIMP TextService::OnTestKeyFocus(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)context;
    (void)wParam;
    (void)lParam;
    *eaten = FALSE;
    return S_OK;
}

STDMETHODIMP TextService::OnKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)context;
    (void)wParam;
    (void)lParam;
    *eaten = FALSE;
    return S_OK;
}

STDMETHODIMP TextService::OnKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)context;
    (void)wParam;
    (void)lParam;
    *eaten = FALSE;
    return S_OK;
}

STDMETHODIMP TextService::OnPreservedKey(ITfContext* context, REFGUID rguid, BOOL* eaten) {
    (void)context;
    (void)rguid;
    *eaten = FALSE;
    return S_OK;
}

STDMETHODIMP TextService::OnInitDocumentMgr(ITfDocumentMgr* docMgr) {
    (void)docMgr;
    return S_OK;
}

STDMETHODIMP TextService::OnUninitDocumentMgr(ITfDocumentMgr* docMgr) {
    (void)docMgr;
    return S_OK;
}

STDMETHODIMP TextService::OnSetFocus(ITfDocumentMgr* docMgrFocus, ITfDocumentMgr* docMgrPrevFocus) {
    (void)docMgrFocus;
    (void)docMgrPrevFocus;
    return S_OK;
}

STDMETHODIMP TextService::OnPushContext(ITfContext* context) {
    (void)context;
    return S_OK;
}

STDMETHODIMP TextService::OnPopContext(ITfContext* context) {
    (void)context;
    return S_OK;
}

} // namespace tip

#endif // _WIN32
