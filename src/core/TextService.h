#pragma once

#ifdef _WIN32

#include <msctf.h>
#include <unknwn.h>

#include "candidate_window.h"
#include "engine/state/input_state.h"
#include "input_processor.h"

namespace tip {

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
    HRESULT AdviseKeyEventSink();
    HRESULT UnadviseKeyEventSink();
    HRESULT AdviseThreadMgrEventSink();
    HRESULT UnadviseThreadMgrEventSink();

    HRESULT HandleKey(ITfContext* context, WPARAM wParam, LPARAM lParam, BOOL keyDown, BOOL* eaten);
    HRESULT CommitText(ITfContext* context, const wchar_t* text);
    HRESULT ProcessKey(ITfContext* context, WPARAM wParam, BOOL keyDown, BOOL* eaten);
    void UpdateCandidateWindow(ITfContext* context);
    void HideCandidateWindow();
    void OnCandidateWindowClick(int action, int param);
    void OnLanguageToggled();
    void UpdateLanguageIndicator();

private:
    LONG refCount_;
    ITfThreadMgr* threadMgr_;
    TfClientId clientId_;
    DWORD keyEventSinkCookie_;
    DWORD threadMgrEventSinkCookie_;
    ITfComposition* composition_;
    InputProcessor inputProcessor_;
    CandidateWindow candidateWindow_;
    InputState inputState_;
    bool shiftLeftPressed_;
    bool shiftRightPressed_;
    bool shiftUsed_;
};

} // namespace tip

#endif // _WIN32
