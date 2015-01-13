#pragma once

// forward declaration
class CImageScatterAnimation;

//
// CAppWindow class
//

class CAppWindow
    : public CWindowImpl<CAppWindow, CWindow, CWinTraitsOR<0,0,CNullTraits> >
{
public:
    CAppWindow(list<wstring>& imagesList);
    ~CAppWindow();

    BEGIN_MSG_MAP(CAppWnd)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    END_MSG_MAP();

    VOID UpdateView();
    VOID Repaint();

private:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

private:
    list<wstring> m_imagesList;
    list<wstring>::const_iterator m_iterator;
    auto_ptr<CImageScatterAnimation> m_animation;
    UINT_PTR m_nTimer;
    BOOL m_bUpdate;

    HDC m_hDC;
    HGDIOBJ m_hOldBmp;
    HBITMAP m_hBmp;

    HDC m_hScreenDC;
    HGDIOBJ m_hScreenOldBmp;
    HBITMAP m_hScreenBmp;
};
