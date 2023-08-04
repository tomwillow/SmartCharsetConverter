#include <ControlStyle.h>

#include "DialogMain.h"

#include <tstring.h>
#include <ErrorFunction.h>

#include <stdexcept>

using namespace std;

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR szCmdLine, int nCmdShow)
#endif
    try {
    Split_UnitTest();
    InitCommonControls();
    SupportHighDPI();

    {
        DialogMain dialogMain;
        auto ret = dialogMain.DoModal();
        if (ret == -1) {
            auto code = GetLastError();
            throw runtime_error(string("DoModal fail. \ncode = ") + to_string(code) + string("\ninfo = ") +
                                to_string(GetLastErrorString(code)));
        }
    }

    return 0;
} catch (exception &e) {
    wstring content = to_wstring(e.what());
    MessageBox(0, content.c_str(), L"Error", MB_OK | MB_ICONERROR);
    return -1;
}