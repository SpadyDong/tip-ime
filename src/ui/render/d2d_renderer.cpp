#include "d2d_renderer.h"

namespace tip {

class D2DRenderer::Impl {
public:
    bool initialized = false;
};

D2DRenderer::D2DRenderer()
    : impl_(new Impl()) {
}

D2DRenderer::~D2DRenderer() {
    delete impl_;
}

bool D2DRenderer::Initialize(void* hwnd) {
    (void)hwnd;
    // TODO: create Direct2D factory, render target and brushes
    impl_->initialized = true;
    return true;
}

void D2DRenderer::Shutdown() {
    impl_->initialized = false;
}

void D2DRenderer::BeginDraw() {
}

void D2DRenderer::EndDraw() {
}

} // namespace tip
