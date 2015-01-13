#include "stdafx.h"
#include "appwnd.h"
#include "imghelp.h"

//
// CAppWindow class
//

CAppWindow::CAppWindow(list<wstring>& imagesList)
    : m_nTimer(-1)
    , m_hDC(NULL)
    , m_hBmp(NULL)
    , m_hOldBmp(NULL)
    , m_hScreenDC(NULL)
    , m_hScreenOldBmp(NULL)
    , m_hScreenBmp(NULL)
    , m_bUpdate(TRUE)
{
    m_imagesList.swap(imagesList);
    m_iterator = m_imagesList.begin(); 
}

CAppWindow::~CAppWindow()
{
}

LRESULT CAppWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    // UINT32 nID = 1;
    // UINT32 nElapseMsec = 10;
    // m_nTimer = SetTimer(nID, nElapseMsec);

    bHandled = FALSE;
    return 0;
}

LRESULT CAppWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    ::SelectObject(m_hDC, m_hOldBmp);
    m_hOldBmp = NULL;
    ::DeleteDC(m_hDC); 
    m_hDC = NULL;
    ::DeleteObject(m_hBmp);
    m_hBmp = NULL;

    ::SelectObject(m_hScreenDC, m_hScreenOldBmp);
    m_hScreenOldBmp = NULL;
    ::DeleteDC(m_hScreenDC); 
    m_hScreenDC = NULL;
    ::DeleteObject(m_hScreenBmp);
    m_hScreenBmp = NULL;
    
    // KillTimer(m_nTimer);
    // m_nTimer = -1;

    bHandled = FALSE;
    return 0;
}

LRESULT CAppWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    if ( m_bUpdate )
    {
        UpdateView();
        m_bUpdate = FALSE;
    }

    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(&ps);
    
    RECT rect;
    GetClientRect(&rect);

    BitBlt(hDC, 0, 0, rect.right, rect.bottom, m_hDC, 0, 0, SRCCOPY);

    EndPaint(&ps);

    bHandled = TRUE;
    return 0;
}

LRESULT CAppWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    ::DestroyWindow(m_hWnd);
    bHandled = TRUE;
    return 0;
}

LRESULT CAppWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    UpdateView();
    Invalidate(FALSE);
    bHandled = TRUE;
    return 0;
}

VOID CAppWindow::Repaint()
{
    RECT rect;
    GetClientRect(&rect);

    HDC hDC = GetDC();

    BitBlt(hDC, 0, 0, rect.right, rect.bottom, m_hDC, 0, 0, SRCCOPY);

    ReleaseDC(hDC);
}

VOID CAppWindow::UpdateView()
{
    if ( m_iterator == m_imagesList.end() )
        m_iterator = m_imagesList.begin();
    if ( m_iterator == m_imagesList.end() )
        return;

    RECT rect;
    GetClientRect(&rect);

    if ( m_hBmp == NULL )
    {
        auto_ptr<Bitmap> m_image( new Bitmap(rect.right, rect.bottom, PixelFormat32bppARGB) );
        m_image->GetHBITMAP(Color::Black, &m_hBmp);
        m_image.reset();

        BITMAP bmp = { 0 };
        ::GetObject(m_hBmp, sizeof(bmp), &bmp);

        m_hDC = CreateCompatibleDC(NULL);
        m_hOldBmp = ::SelectObject(m_hDC, m_hBmp);
    }

    if ( m_hScreenBmp == NULL )
    {
        auto_ptr<Bitmap> m_image( new Bitmap(rect.right, rect.bottom, PixelFormat32bppARGB) );
        m_image->GetHBITMAP(Color::Black, &m_hScreenBmp);
        m_image.reset();

        BITMAP bmp = { 0 };
        ::GetObject(m_hScreenBmp, sizeof(bmp), &bmp);

        m_hScreenDC = CreateCompatibleDC(NULL);
        m_hScreenOldBmp = ::SelectObject(m_hScreenDC, m_hScreenBmp);
    }

    LPCWSTR wszName = (m_iterator++)->c_str();
    auto_ptr<Image> image( new Image(wszName) ); // exception
    CImagesScatter::DrawScatterImage(
        m_hBmp,
        Rect(10, 10, rect.right - 20, rect.bottom - 20), // client area to draw
        image.get(), // image to draw
        80.0, // max angle
        30, // max offset
        10, // frame thick
        Color::WhiteSmoke // frame color
        ); 

    return;

    if ( m_animation.get() == NULL )
    {
        LPCWSTR wszName = m_iterator->c_str();

        auto_ptr<Image> image( new Image(wszName) ); // exception

        m_animation = CImagesScatter::CreateScatterImageAnimation(
            Rect(10, 10, rect.right - 20, rect.bottom - 20), // client area to draw
            image.get(), // image to draw
            80.0, // max angle
            30, // max offset
            10, // frame thick
            Color::WhiteSmoke // frame color
            ); 

        ++m_iterator;
    }

    BitBlt(m_hDC, 0, 0, rect.right, rect.bottom, m_hScreenDC, 0, 0, SRCCOPY);

    if ( !m_animation->NextAnimation(m_hBmp) )
    {
        m_animation.reset();

        BitBlt(m_hScreenDC, 0, 0, rect.right, rect.bottom, m_hDC, 0, 0, SRCCOPY);
    }
}
