#pragma once

#ifdef _WIN32

#include <msctf.h>
#include <string>
#include <unknwn.h>

namespace tip {

class InputProcessor;
class CandidateWindow;
struct CandidateWindowStyle;

class TextService : public ITfTextInputProcessor,
                    public ITfKeyEventSink,
                    public ITfThreadMgrEventSink {
public:
    TextService();
    virtual ~TextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* threadMgr, TfClientId clientId) override;
    STDMETHODIMP Deactivate() override;

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL foreground) override;
    STDMETHODIMP OnTestKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) override;
    STDMETHODIMP OnTestKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) override;
    STDMETHODIMP OnKeyDown(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) override;
    STDMETHODIMP OnKeyUp(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL* eaten) override;
    STDMETHODIMP OnPreservedKey(ITfContext* context, REFGUID rguid, BOOL* eaten) override;

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr* docMgr) override;
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr* docMgr) override;
    STDMETHODIMP OnSetFocus(ITfDocumentMgr* docMgrFocus, ITfDocumentMgr* docMgrPrevFocus) override;
    STDMETHODIMP OnPushContext(ITfContext* context) override;
    STDMETHODIMP OnPopContext(ITfContext* context) override;

private:
    void UpdateCandidateWindow();
    void CommitText(ITfContext* context, const std::wstring& text);
    std::wstring GetDataDirectory() const;

    LONG refCount_;
    ITfThreadMgr* threadMgr_;
    TfClientId clientId_;
    DWORD keyEventSinkCookie_;
    DWORD threadMgrEventSinkCookie_;

    InputProcessor* inputProcessor_;
    CandidateWindow* candidateWindow_;
    CandidateWindowStyle* candidateStyle_;
    BOOL isComposing_;
};

} // namespace tip

#endif // _WIN32
