#include "ResourceLoader.h"

#include <Windows.h>

#include <cassert>

std::vector<char> LoadCustomFileFromResource(int id, const std::wstring &resourceType) {

    HRSRC hResource = FindResourceW(NULL, MAKEINTRESOURCE(id), resourceType.c_str());
    // 处理资源未找到的情况
    assert(hResource);

    HGLOBAL hResData = LoadResource(NULL, hResource);
    // 处理资源加载失败的情况
    assert(hResData);

    DWORD dwResSize = SizeofResource(NULL, hResource);
    // 处理资源大小为0的情况
    assert(dwResSize);

    LPBYTE lpResData = (LPBYTE)LockResource(hResData);
    // 处理资源锁定失败的情况
    assert(lpResData);

    std::vector<char> ret(dwResSize);
    memcpy(ret.data(), lpResData, dwResSize);

    return ret;
}