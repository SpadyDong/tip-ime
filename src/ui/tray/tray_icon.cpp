#include "tray_icon.h"

namespace tip {

TrayIcon::TrayIcon() {
}

TrayIcon::~TrayIcon() {
}

bool TrayIcon::Initialize() {
    // TODO: register system tray icon
    return true;
}

void TrayIcon::Shutdown() {
}

void TrayIcon::UpdateStatus(bool chineseMode) {
    (void)chineseMode;
    // TODO: switch tray icon based on input language
}

} // namespace tip
