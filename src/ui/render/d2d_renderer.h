#pragma once

namespace tip {

class D2DRenderer {
public:
    D2DRenderer();
    ~D2DRenderer();

    bool Initialize(void* hwnd);
    void Shutdown();
    void BeginDraw();
    void EndDraw();

private:
    class Impl;
    Impl* impl_;
};

} // namespace tip
