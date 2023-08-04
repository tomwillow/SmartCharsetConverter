#include "ErrorFunction.h"

using namespace std;

// 根据错误码返回对应的错误信息
std::tstring GetLastErrorString(DWORD errorCode) {
    TCHAR *lpMsgBuf = nullptr;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                  errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (TCHAR *)&lpMsgBuf, 0, NULL);

    tstring ret(lpMsgBuf);

    LocalFree(lpMsgBuf);
    return ret;
}