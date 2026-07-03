#include "TextService.h"

#ifdef _WIN32

#include <windows.h>
#include <string>
#include <vector>

#include "engine/config/config_manager.h"
#include "engine/input_processor.h"
#include "logger.h"
#include "ui/candidate/candidate_window.h"
#include "utils/string_utils.h"

namespace tip {

extern HMODULE g_tipModule;

namespace {

constexpr int kCandidateWindowOffsetY = 20;

bool IsLetterKey(WPARAM wParam) {
    return (wParam >= L'A' && wParam <= L'Z') || (wParam >= L'a' && wParam <= L'z');
}

bool IsDigitKey(WPARAM wParam) {
    return wParam >= L'0' && wParam <= L'9';
}

bool IsPinyinInputKey(WPARAM wParam) {
    return IsLetterKey(wParam) || wParam == VK_BACK || wParam == VK_ESCAPE || wParam == VK_SPACE;
}

bool IsPageKey(WPARAM wParam) {
    return wParam == VK_OEM_COMMA || wParam == VK_OEM_PERIOD;
}

class TextEditSession : public ITfEditSession {
public:
    TextEditSession(ITfContext* context, const std::wstring& text)
        : context_(context)
        , text_(text)
        , refCount_(1) {
        context_->AddRef();
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

    STDMETHODIMP DoEditSession(TfEditCookie ec) override {
        ITfInsertAtSelection* insert = nullptr;
        HRESULT hr = context_->QueryInterface(IID_ITfInsertAtSelection, reinterpret_cast<void**>(&insert));
        if (SUCCEEDED(hr) && insert) {
            ITfRange* range = nullptr;
            insert->InsertTextAtSelection(ec, 0, text_.c_str(), static_cast<ULONG>(text_.size()), &range);
            if (range) {
                range->Release();
            }
            insert->Release();
        }
        return S_OK;
    }

private:
    ITfContext* context_;
    std::wstring text_;
    LONG refCount_;
};

} // namespace

TextService::TextService()
    : refCount_(1)
    , threadMgr_(nullptr)
    , clientId_(0)
    , keyEventSinkCookie_(TF_INVALID_COOKIE)
    , threadMgrEventSinkCookie_(TF_INVALID_COOKIE)
    , inputProcessor_(nullptr)
    , candidateWindow_(nullptr)
    , candidateStyle_(nullptr)
    , isComposing_(FALSE) {
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

std::wstring TextService::GetDataDirectory() const {
    if (!g_tipModule) {
        return L"";
    }
    wchar_t path[MAX_PATH] = {};
    DWORD len = ::GetModuleFileNameW(g_tipModule, path, MAX_PATH);
    if (len == 0) {
        return L"";
    }
    std::wstring modulePath(path, len);
    size_t pos = modulePath.find_last_of(L"\\/");
    if (pos == std::wstring::npos) {
        return L"";
    }
    std::wstring moduleDir = modulePath.substr(0, pos);
    return moduleDir + L"\\data";
}

STDMETHODIMP TextService::Activate(ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr_ = threadMgr;
    threadMgr_->AddRef();
    clientId_ = clientId;

    std::wstring dataDir = GetDataDirectory();
    if (!dataDir.empty()) {
        ConfigManager::Instance().Load(dataDir + L"\\config\\default.ini");
    }

    candidateStyle_ = new CandidateWindowStyle();
    if (!dataDir.empty()) {
        LoadCandidateWindowStyleFromFile(dataDir + L"\\skins\\default.json", *candidateStyle_);
    }

    inputProcessor_ = new InputProcessor();
    if (!dataDir.empty()) {
        inputProcessor_->Initialize(dataDir + L"\\dictionary\\base_dict.txt");
    } else {
        inputProcessor_->Initialize(L"");
    }

    candidateWindow_ = new CandidateWindow();
    candidateWindow_->Create();
    candidateWindow_->SetStyle(*candidateStyle_);
    candidateWindow_->SetCornerRadius(ConfigManager::Instance().GetConfig().cornerRadius);

    ITfKeystrokeMgr* keystrokeMgr = nullptr;
    HRESULT hr = threadMgr_->QueryInterface(IID_ITfKeystrokeMgr, reinterpret_cast<void**>(&keystrokeMgr));
    if (SUCCEEDED(hr) && keystrokeMgr) {
        keystrokeMgr->AdviseKeyEventSink(clientId_, this, TRUE, &keyEventSinkCookie_);
        keystrokeMgr->Release();
    }

    ITfSource* source = nullptr;
    hr = threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source));
    if (SUCCEEDED(hr) && source) {
        source->AdviseSink(IID_ITfThreadMgrEventSink,
                           static_cast<ITfThreadMgrEventSink*>(this),
                           &threadMgrEventSinkCookie_);
        source->Release();
    }

    TIP_LOG_INFO(L"TextService activated");
    return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
    ITfKeystrokeMgr* keystrokeMgr = nullptr;
    if (threadMgr_ && keyEventSinkCookie_ != TF_INVALID_COOKIE &&
        SUCCEEDED(threadMgr_->QueryInterface(IID_ITfKeystrokeMgr, reinterpret_cast<void**>(&keystrokeMgr)))) {
        keystrokeMgr->UnadviseKeyEventSink(keyEventSinkCookie_);
        keystrokeMgr->Release();
    }
    keyEventSinkCookie_ = TF_INVALID_COOKIE;

    ITfSource* source = nullptr;
    if (threadMgr_ && threadMgrEventSinkCookie_ != TF_INVALID_COOKIE &&
        SUCCEEDED(threadMgr_->QueryInterface(IID_ITfSource, reinterpret_cast<void**>(&source)))) {
        source->UnadviseSink(threadMgrEventSinkCookie_);
        source->Release();
    }
    threadMgrEventSinkCookie_ = TF_INVALID_COOKIE;

    if (candidateWindow_) {
        candidateWindow_->Destroy();
        delete candidateWindow_;
        candidateWindow_ = nullptr;
    }

    if (inputProcessor_) {
        inputProcessor_->Shutdown();
        delete inputProcessor_;
        inputProcessor_ = nullptr;
    }

    delete candidateStyle_;
    candidateStyle_ = nullptr;

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

STDMETHODIMP TextService::OnTestKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)context;
    (void)lParam;
    *eaten = FALSE;

    if (!inputProcessor_) {
        return S_OK;
    }

    bool composing = !inputProcessor_->GetRawPinyin().empty();
    if (composing && (IsPinyinInputKey(wParam) || IsDigitKey(wParam) || IsPageKey(wParam))) {
        *eaten = TRUE;
    } else if (!composing && IsLetterKey(wParam)) {
        *eaten = TRUE;
    }
    return S_OK;
}

STDMETHODIMP TextService::OnTestKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)context;
    (void)wParam;
    (void)lParam;
    *eaten = FALSE;
    return S_OK;
}

void TextService::CommitText(ITfContext* context, const std::wstring& text) {
    if (!context || text.empty()) {
        return;
    }
    TextEditSession* session = new TextEditSession(context, text);
    HRESULT hr = E_FAIL;
    context->RequestEditSession(clientId_, session, TF_ES_SYNC | TF_ES_READWRITE, &hr);
    session->Release();
}

void TextService::UpdateCandidateWindow() {
    if (!inputProcessor_ || !candidateWindow_) {
        return;
    }

    auto page = inputProcessor_->GetPageCandidates();
    if (page.empty()) {
        candidateWindow_->Hide();
        isComposing_ = FALSE;
        return;
    }

    int pageSize = ConfigManager::Instance().GetConfig().candidateCount;
    int pageStart = inputProcessor_->GetCurrentPage() * pageSize;

    std::vector<CandidateItem> items;
    for (size_t i = 0; i < page.size(); ++i) {
        items.push_back({ page[i].text, page[i].pinyin, static_cast<int>(pageStart + i) });
    }

    candidateWindow_->SetCornerRadius(ConfigManager::Instance().GetConfig().cornerRadius);
    candidateWindow_->UpdateCandidates(items, inputProcessor_->GetSelectedIndex());

    POINT pt = {};
    if (!::GetCaretPos(&pt)) {
        pt.x = 100;
        pt.y = 100;
    }
    candidateWindow_->MoveTo(pt.x, pt.y + kCandidateWindowOffsetY);
    candidateWindow_->Show();
    isComposing_ = TRUE;
}

STDMETHODIMP TextService::OnKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) {
    (void)lParam;
    *eaten = FALSE;

    if (!inputProcessor_) {
        return S_OK;
    }

    bool composing = !inputProcessor_->GetRawPinyin().empty();

    if (IsLetterKey(wParam)) {
        inputProcessor_->AppendPinyinChar(static_cast<wchar_t>(wParam));
        UpdateCandidateWindow();
        *eaten = TRUE;
    } else if (wParam == VK_BACK) {
        if (composing) {
            inputProcessor_->Backspace();
            UpdateCandidateWindow();
            *eaten = TRUE;
        }
    } else if (wParam == VK_ESCAPE) {
        if (composing) {
            inputProcessor_->Clear();
            candidateWindow_->Hide();
            isComposing_ = FALSE;
            *eaten = TRUE;
        }
    } else if (wParam == VK_SPACE) {
        if (composing && !inputProcessor_->GetPageCandidates().empty()) {
            if (inputProcessor_->SelectCurrentCandidate()) {
                CommitText(context, inputProcessor_->GetCommittedText());
            }
            candidateWindow_->Hide();
            isComposing_ = FALSE;
            *eaten = TRUE;
        }
    } else if (wParam == VK_OEM_COMMA) {
        if (composing) {
            inputProcessor_->PageUp();
            UpdateCandidateWindow();
            *eaten = TRUE;
        }
    } else if (wParam == VK_OEM_PERIOD) {
        if (composing) {
            inputProcessor_->PageDown();
            UpdateCandidateWindow();
            *eaten = TRUE;
        }
    } else if (IsDigitKey(wParam) && wParam != L'0') {
        if (composing) {
            size_t index = static_cast<size_t>(wParam - L'1');
            if (index < inputProcessor_->GetPageCandidates().size() &&
                inputProcessor_->SelectCandidateByIndex(index)) {
                CommitText(context, inputProcessor_->GetCommittedText());
                candidateWindow_->Hide();
                isComposing_ = FALSE;
                *eaten = TRUE;
            }
        }
    }

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
