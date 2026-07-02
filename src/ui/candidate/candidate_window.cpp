#include "candidate_window.h"

namespace tip {

class CandidateWindow::Impl {
public:
    bool created = false;
    bool visible = false;
    int posX = 0;
    int posY = 0;
    std::vector<CandidateItem> candidates;
};

CandidateWindow::CandidateWindow()
    : impl_(new Impl()) {
}

CandidateWindow::~CandidateWindow() {
    delete impl_;
}

bool CandidateWindow::Create() {
    // TODO: create layered Win32 window for candidate UI
    impl_->created = true;
    return true;
}

void CandidateWindow::Destroy() {
    impl_->created = false;
    impl_->visible = false;
}

void CandidateWindow::Show() {
    impl_->visible = true;
}

void CandidateWindow::Hide() {
    impl_->visible = false;
}

void CandidateWindow::UpdateCandidates(const std::vector<CandidateItem>& candidates) {
    impl_->candidates = candidates;
}

void CandidateWindow::MoveTo(int x, int y) {
    impl_->posX = x;
    impl_->posY = y;
}

} // namespace tip
