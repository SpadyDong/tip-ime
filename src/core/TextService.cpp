#include "TextService.h"

#ifdef _WIN32

#include <cstring>
#include <new>
#include <shellapi.h>

#include "config_manager.h"
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
    candidateWindow_.SetClickCallback([this](int action, int param) {
        OnCandidateWindowClick(action, param);
    });
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

    ConfigManager::Instance().Load(L"data/config/default.ini");
    inputProcessor_.Initialize();

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

    inputProcessor_.Shutdown();

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
    return ProcessKey(context, wParam, keyDown, eaten);
}

HRESULT TextService::ProcessKey(ITfContext* context, WPARAM wParam, BOOL keyDown, BOOL* eaten) {
    *eaten = FALSE;

    if (!context) {
        return S_OK;
    }

    bool handled = false;

    if (keyDown) {
        if (wParam >= 'a' && wParam <= 'z') {
            inputProcessor_.AppendPinyinChar(static_cast<wchar_t>(wParam));
            handled = true;
        } else if (wParam >= 'A' && wParam <= 'Z') {
            inputProcessor_.AppendPinyinChar(static_cast<wchar_t>(wParam - 'A' + 'a'));
            handled = true;
        } else if (wParam == VK_BACK) {
            inputProcessor_.Backspace();
            handled = true;
        } else if (wParam == VK_ESCAPE) {
            inputProcessor_.Clear();
            handled = true;
        } else if (wParam == VK_PRIOR) {
            inputProcessor_.PageUp();
            handled = true;
        } else if (wParam == VK_NEXT) {
            inputProcessor_.PageDown();
            handled = true;
        } else if (wParam == VK_SPACE || wParam == VK_RETURN) {
            // Commit the first candidate on the current page.
            if (inputProcessor_.SelectCandidate(0)) {
                handled = true;
            }
        } else if (wParam >= '0' && wParam <= '9') {
            size_t index = (wParam == '0') ? 9 : (wParam - '1');
            if (inputProcessor_.SelectCandidate(index)) {
                handled = true;
            }
        }
    }

    if (handled) {
        *eaten = TRUE;
        std::wstring committed = inputProcessor_.GetCommittedText();
        if (!committed.empty()) {
            CommitText(context, committed.c_str());
            HideCandidateWindow();
        } else {
            UpdateCandidateWindow(context);
        }
    }

    return S_OK;
}

void TextService::UpdateCandidateWindow(ITfContext* context) {
    (void)context;

    auto rawPinyin = inputProcessor_.GetRawPinyin();
    auto candidates = inputProcessor_.GetPagedCandidates();

    if (rawPinyin.empty() || candidates.empty()) {
        HideCandidateWindow();
        return;
    }

    std::vector<CandidateItem> items;
    for (size_t i = 0; i < candidates.size(); ++i) {
        items.push_back({ candidates[i].text, candidates[i].pinyin, static_cast<int>(i) });
    }

    if (!candidateWindow_.Create()) {
        return;
    }

    POINT pt = {};
    if (GetCaretPos(&pt)) {
        // Client coordinates of the focused window; convert to screen coordinates.
        HWND hwndForeground = GetForegroundWindow();
        ClientToScreen(hwndForeground, &pt);
    } else {
        GetCursorPos(&pt);
    }

    candidateWindow_.SetPageInfo(static_cast<int>(inputProcessor_.GetCurrentPage()),
                                 static_cast<int>(inputProcessor_.GetTotalPages()));
    candidateWindow_.MoveTo(pt.x, pt.y + 20);
    candidateWindow_.UpdateCandidates(items);
    candidateWindow_.Show();
}

void TextService::HideCandidateWindow() {
    candidateWindow_.Hide();
}

void TextService::OnCandidateWindowClick(int action, int param) {
    if (action == kCandidateActionSettings) {
        // Launch the standalone settings application asynchronously.
        ShellExecuteW(nullptr, L"open", L"tip_setting.exe", nullptr, nullptr, SW_SHOWNORMAL);
        return;
    }

    bool needUpdate = false;
    if (action == kCandidateActionPrevPage) {
        needUpdate = inputProcessor_.PageUp();
    } else if (action == kCandidateActionNextPage) {
        needUpdate = inputProcessor_.PageDown();
    } else if (action == kCandidateActionSelect) {
        if (!inputProcessor_.SelectCandidate(static_cast<size_t>(param))) {
            return;
        }
    } else {
        return;
    }

    std::wstring committed = inputProcessor_.GetCommittedText();
    if (!committed.empty()) {
        // We need a context to commit text; try to obtain the current context.
        ITfDocumentMgr* docMgr = nullptr;
        if (threadMgr_ && SUCCEEDED(threadMgr_->GetFocus(&docMgr))) {
            ITfContext* context = nullptr;
            if (SUCCEEDED(docMgr->GetTop(&context))) {
                CommitText(context, committed.c_str());
                context->Release();
            }
            docMgr->Release();
        }
        HideCandidateWindow();
        return;
    }

    if (needUpdate) {
        UpdateCandidateWindow(nullptr);
    }
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
