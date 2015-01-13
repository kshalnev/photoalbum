#include "stdafx.h"
#include "imghelp.h"
#include "appwnd.h"

//
// CGdiPlusInit class
//

class CGdiPlusInit
{
public:
    CGdiPlusInit()
    {
        ::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
    }
    ~CGdiPlusInit()
    {
        ::GdiplusShutdown(m_gdiplusToken);
    }
private:
    GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR m_gdiplusToken;
};

//
// Helper
//

void EnumImageFiles(LPCWSTR szPath, 
                    LPCWSTR szMask,
                    list<wstring>* pImageFiles) throw(...) // exception
{
    const UINT nMaxLen = 1023;
    WCHAR szWildcard[nMaxLen + 1] = { 0 };
    _snwprintf_s(szWildcard, nMaxLen, nMaxLen, L"%s%s", szPath, szMask);

    const UINT nPathLen = wcslen(szPath);

    WIN32_FIND_DATA wfd = { 0 };
    HANDLE hFind = ::FindFirstFileW(szWildcard, &wfd);
    if ( hFind != NULL && hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            wcscpy_s(szWildcard + nPathLen, nMaxLen - nPathLen, wfd.cFileName);
            pImageFiles->push_back( szWildcard ); // exception
        }
        while( ::FindNextFileW(hFind, &wfd) );

        ::FindClose(hFind);
        hFind = NULL;
    }

    wcscpy_s(szWildcard + nPathLen, nMaxLen - nPathLen, L"*.*");

    hFind = ::FindFirstFileW(szWildcard, &wfd);
    if ( hFind != NULL && hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            if ( _wcsicmp(wfd.cFileName, L".") != 0 && _wcsicmp(wfd.cFileName, L"..") != 0 )
            {
                if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                {
                    WCHAR szSubPath[nMaxLen + 1] = { 0 };
                    _snwprintf_s(szSubPath, nMaxLen, nMaxLen, L"%s%s\\", szPath, wfd.cFileName);

                    EnumImageFiles(szSubPath, szMask, pImageFiles);
                }
            }
        }
        while( ::FindNextFileW(hFind, &wfd) );

        ::FindClose(hFind);
        hFind = NULL;
    }
}

//
// Entry point
//

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR szCmdLine, int)
{
    CGdiPlusInit gdiPlusInit;

    const UINT nMaxLen = 1023;
    WCHAR szPath[nMaxLen + 1] = { 0 };

    wcscpy_s(szPath, nMaxLen - 1, szCmdLine);
    UINT nLen = wcslen(szPath);
    if ( nLen > 0 )
    {
        if ( szPath[nLen-1] != L'\\' && szPath[nLen-1] != L'/' )
        {
            ++nLen;
            szPath[nLen-1] = L'\\';
            szPath[nLen] = L'\0';
        }
    }

    list<wstring> imagesList;
    EnumImageFiles(szPath, L"*.bmp", &imagesList);
    EnumImageFiles(szPath, L"*.jpg", &imagesList);
    EnumImageFiles(szPath, L"*.jpeg", &imagesList);
    EnumImageFiles(szPath, L"*.png", &imagesList);

    if ( imagesList.empty() )
    {
        return 0;
    }

    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

#ifndef _DEBUG
    RECT rect = { dm.dmPelsWidth-800, dm.dmPelsHeight-600, dm.dmPelsWidth, dm.dmPelsHeight };
#else
    RECT rect = { dm.dmPelsWidth-320, dm.dmPelsHeight-200, dm.dmPelsWidth, dm.dmPelsHeight };
#endif

    CAppWindow wnd(imagesList);
    HWND hWnd = wnd.Create(NULL, rect, NULL, WS_POPUP);
    ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

    ::ShowWindow(hWnd, SW_SHOW);
    while (::IsWindow(hWnd))
    {
        MSG msg;
        if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            wnd.UpdateView();
            wnd.Repaint();
        }
    }

    return 0;
}
