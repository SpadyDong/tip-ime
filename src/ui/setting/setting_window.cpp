#ifdef _WIN32

#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    MessageBoxW(nullptr, L"TIP输入法设置面板", L"TIP Setting", MB_OK);
    return 0;
}

#else

int main() {
    return 0;
}

#endif // _WIN32
