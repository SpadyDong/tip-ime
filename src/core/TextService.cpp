#include "TextService.h"

#ifdef _WIN32

#include <cstring>
#include <new>

#include "logger.h"

namespace tip {

namespace {

class CommitEditSession : public ITfEditSession {
public:
    CommitEditSession(ITfContext* context, const wchar_t* text)
        : refCount_(1), context_(context), text_(text) {
        if (context_) {
            context_->AddRef();
        }
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) {
            return E_INVALIDARG;
        }
        *ppv = nullptr;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfEditSession)) {
            *ppv = static_cast<ITfEditSession*>(this);
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

    STDMETHODIMP DoEditSession(TfEditCookie cookie) override {
        if (!context_) {
            return E_FAIL;
        }

        ITfInsertAtSelection* insertAtSelection = nullptr;
        HRESULT hr = context_->QueryInterface(IID_ITfInsertAtSelection,
                                              reinterpret_cast<void**>(&insertAtSelection));
        if (FAILED(hr)) {
            return hr;
        }

        ITfRange* range = nullptr;
        hr = insertAtSelection->InsertTextAtSelection(cookie, TF_IAS_QUERYONLY,
                                                       L"", 0, &range);
        insertAtSelection->Release();
        if (FAILED(hr) || !range) {
            return hr;
        }

        ULONG textLen = static_cast<ULONG>(wcslen(text_));
        if (textLen > 0) {
            range->SetText(cookie, 0, text_, textLen);
        }
        range->Release();
        return S_OK;
    }

private:
    LONG refCount_;
    ITfContext* context_;
    const wchar_t* text_;
};

} // namespace

TextService::TextService()
    : refCount_(1)
    , threadMgr_(nullptr)
    , clientId_(0)
    , keyEventSinkCookie_(TF_INVALID_COOKIE)
    , threadMgrEventSinkCookie_(TF_INVALID_COOKIE)
    , composition_(nullptr) {
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
    if (!threadMgr) {
        return E_INVALIDARG;
    }

    threadMgr_ = threadMgr;
    threadMgr_->AddRef();
    clientId_ = clientId;

    HRESULT hr = AdviseThreadMgrEventSink();
    if (FAILED(hr)) {
        TIP_LOG_ERROR(L"AdviseThreadMgrEventSink failed");
        Deactivate();
        return hr;
    }

    hr = AdviseKeyEventSink();
    if (FAILED(hr)) {
        TIP_LOG_ERROR(L"AdviseKeyEventSink failed");
        Deactivate();
        return hr;
    }

    TIP_LOG_INFO(L"TextService activated");
    return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
    UnadviseKeyEventSink();
    UnadviseThreadMgrEventSink();

    if (composition_) {
        composition_->Release();
        composition_ = nullptr;
    }

    if (threadMgr_) {
        threadMgr_->Release();
        threadMgr_ = nullptr;
    }
    clientId_ = 0;

    TIP_LOG_INFO(L"TextService deactivated");
    return S_OK;
}

HRESULT TextService::AdviseKeyEventSink() {
    if (!threadMgr_) {
        return E_FAIL;
    }

    ITfSource* source = nullptr;
    HRESULT hr = threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source));
    if (FAILED(hr)) {
        return hr;
    }

    hr = source->AdviseSink(IID_ITfKeyEventSink, static_cast<ITfKeyEventSink*>(this),
                            &keyEventSinkCookie_);
    source->Release();
    return hr;
}

HRESULT TextService::UnadviseKeyEventSink() {
    if (!threadMgr_ || keyEventSinkCookie_ == TF_INVALID_COOKIE) {
        return S_OK;
    }

    ITfSource* source = nullptr;
    HRESULT hr = threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source));
    if (FAILED(hr)) {
        return hr;
    }

    hr = source->UnadviseSink(keyEventSinkCookie_);
    source->Release();
    keyEventSinkCookie_ = TF_INVALID_COOKIE;
    return hr;
}

HRESULT TextService::AdviseThreadMgrEventSink() {
    if (!threadMgr_) {
        return E_FAIL;
    }

    ITfSource* source = nullptr;
    HRESULT hr = threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source));
    if (FAILED(hr)) {
        return hr;
    }

    hr = source->AdviseSink(IID_ITfThreadMgrEventSink,
                            static_cast<ITfThreadMgrEventSink*>(this),
                            &threadMgrEventSinkCookie_);
    source->Release();
    return hr;
}

HRESULT TextService::UnadviseThreadMgrEventSink() {
    if (!threadMgr_ || threadMgrEventSinkCookie_ == TF_INVALID_COOKIE) {
        return S_OK;
    }

    ITfSource* source = nullptr;
    HRESULT hr = threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source));
    if (FAILED(hr)) {
        return hr;
    }

    hr = source->UnadviseSink(threadMgrEventSinkCookie_);
    source->Release();
    threadMgrEventSinkCookie_ = TF_INVALID_COOKIE;
    return hr;
}

STDMETHODIMP TextService::OnSetFocus(BOOL foreground) {
    (void)foreground;
    return S_OK;
}

STDMETHODIMP TextService::OnTestKeyFocus(ITfContext* context, WPARAM wParam, LPARAM lParam,
                                         BOOL* eaten) {
    return HandleKey(context, wParam, lParam, false, eaten);
}

STDMETHODIMP TextService::OnKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam,
                                    BOOL* eaten) {
    return HandleKey(context, wParam, lParam, true, eaten);
}

STDMETHODIMP TextService::OnKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam,
                                  BOOL* eaten) {
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

HRESULT TextService::HandleKey(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL keyDown,
                               BOOL* eaten) {
    (void)lParam;
    *eaten = FALSE;

    if (!context) {
        return S_OK;
    }

    // Minimal demo: map lowercase ASCII keys to fixed Chinese characters.
    // 'a' -> "啊", 'b' -> "吧", ..., 'z' -> "在".
    if (keyDown && wParam >= 'a' && wParam <= 'z') {
        static const wchar_t* kDemoMap[26] = {
            L"啊", L"吧", L"从", L"的", L"额", L"发", L"个", L"和", L"是", L"就",
            L"看", L"了", L"吗", L"你", L"哦", L"平", L"去", L"人", L"三", L"他",
            L"有", L"我", L"下", L"一", L"在", L"做"
        };
        *eaten = TRUE;
        return CommitText(context, kDemoMap[wParam - 'a']);
    }

    // Space commits current composition (demo simply dismisses it).
    if (keyDown && wParam == VK_SPACE) {
        *eaten = TRUE;
        return CommitText(context, L"");
    }

    return S_OK;
}

HRESULT TextService::CommitText(ITfContext* context, const wchar_t* text) {
    if (!context) {
        return E_INVALIDARG;
    }

    CommitEditSession* session = new (std::nothrow) CommitEditSession(context, text);
    if (!session) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = S_OK;
    context->RequestEditSession(clientId_, session, TF_ES_SYNC | TF_ES_READWRITE, &hr);
    session->Release();
    return hr;
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
