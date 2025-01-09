#include "CommandLineParser.h"
#include "tstring.h"

std::vector<std::string> GetCommandLineArgs() {
    std::vector<std::string> args;
    LPWSTR *szArglist;
    int nArgs;
    int i;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL == szArglist) {
        throw std::runtime_error("CommandLineToArgvW failed");
    }

    for (i = 0; i < nArgs; i++) {
        args.push_back(to_utf8(szArglist[i]));
    }

    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(szArglist);

    return args;
}
