#pragma once

namespace tip {

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();

    bool Initialize();
    void Shutdown();
    void UpdateStatus(bool chineseMode);
};

} // namespace tip
