#include "CommandLineParser.h"

std::vector<std::wstring> GetCommandLineArgs() {
    std::vector<std::wstring> args;
    LPWSTR *szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL == szArglist) {
        throw std::runtime_error("CommandLineToArgvW failed");
    }

    for (i = 0; i < nArgs; i++) {
        args.push_back(szArglist[i]);
    }

    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(szArglist);

    return args;
}
