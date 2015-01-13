#include "stdafx.h"
#include "imghelp.h"
#include "advbitmap.h"
#include <math.h>

//
// Consts
//

static const double _PI = 3.1415926535897932384626433832795;
static const double DEG_TO_RAD = _PI / 180.0;
static const double RAD_TO_DEG = 180.0 / _PI;

//
// Helpers
//

#define RadToDeg(aRad) ((aRad) * RAD_TO_DEG)
#define DegToRad(aDeg) ((aDeg) * DEG_TO_RAD)

inline INT Round(const double& dValue) throw()
{
    return (INT)( dValue + (( dValue > 0 ) ? 0.5 : -0.5) );
}

inline INT Random(UINT nMaxWidth)
{
    if ( nMaxWidth != 0 )
        return ( rand() % ((INT)nMaxWidth) ) - ( rand() % ((INT)nMaxWidth)/2 );
    else
        return 0;
}

//
// CImageHelper class
//

// CImageHelper::GetEncoderClsid

BOOL CImageHelper::GetEncoderClsid(
                    LPCWSTR szFormat, 
                    CLSID* pClsid) throw()
{
   UINT num = 0;
   UINT size = 0;

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if ( size == 0 )
      return FALSE;

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if ( pImageCodecInfo == NULL )
      return FALSE;

   GetImageEncoders(num, size, pImageCodecInfo);

   for ( UINT j = 0; j < num; ++j )
   {
      if ( wcscmp(pImageCodecInfo[j].MimeType, szFormat) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return TRUE;
      }    
   }

   free(pImageCodecInfo);
   return FALSE;
}

// CImageHelper::FrameImage

auto_ptr<Image> CImageHelper::FrameImage(
                    Image* pImage, 
                    UINT nFrameThick, 
                    Color clrFrame /* = Color::WhiteSmoke */
                    ) throw(...) // exception
{
    const UINT nWidth = nFrameThick + pImage->GetWidth() + nFrameThick;
    const UINT nHeight = nFrameThick + pImage->GetHeight() + nFrameThick;

    auto_ptr<Image> pNewImage( new Bitmap(nWidth, nHeight, PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    SolidBrush brushFrame(clrFrame);
    graphics.FillRectangle(&brushFrame, 0, 0, nWidth, nHeight);

    graphics.DrawImage(pImage, (INT)nFrameThick, (INT)nFrameThick);

    return pNewImage;
}

// CImageHelper::ScaleImage

auto_ptr<Image> CImageHelper::ScaleImage(
                    Image* pImage, 
                    const Size& sizeMax
                    ) throw(...) // exception
{
    const UINT nWidth = pImage->GetWidth();
    const UINT nHeight = pImage->GetHeight();

    const double dWidthRatio = ((double)sizeMax.Width) / ((double)nWidth);
    const double dHeightRatio = ((double)sizeMax.Height) / ((double)nHeight);
    const double dRatio = min(dWidthRatio, dHeightRatio);    

    const UINT nNewWidth = (UINT)Round(dRatio * ((double)nWidth));
    const UINT nNewHeight = (UINT)Round(dRatio * ((double)nHeight));

    auto_ptr<Image> pNewImage( new Bitmap(nNewWidth, nNewHeight, PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.DrawImage(pImage,
        Rect(0, 0, nNewWidth, nNewHeight),
        0, 0, nWidth, nHeight,
        UnitPixel);

    return pNewImage;
}

auto_ptr<Image> CImageHelper::ScaleImage(
                    Image* pImage, 
                    const double& dRatio
                    ) throw(...) // exception
{
    const UINT nWidth = pImage->GetWidth();
    const UINT nHeight = pImage->GetHeight();
    const UINT nNewWidth = (UINT)Round(dRatio * ((double)nWidth));
    const UINT nNewHeight = (UINT)Round(dRatio * ((double)nHeight));

    auto_ptr<Image> pNewImage( new Bitmap(nNewWidth, nNewHeight, PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.DrawImage(pImage,
        Rect(0, 0, nNewWidth, nNewHeight),
        0, 0, nWidth, nHeight,
        UnitPixel);

    return pNewImage;
}

// CImageHelper::ScaleAndFrameImage

auto_ptr<Image> CImageHelper::ScaleAndFrameImage(
            Image* pImage,
            UINT nFrameThick,
            const Size& sizeMax,
            Color clrFrame /* = Color::WhiteSmoke */
            ) throw(...) // exception
{
    const Size sizeMaxNoFrame(sizeMax.Width - 2 * nFrameThick, 
                              sizeMax.Height - 2 * nFrameThick);

    const UINT nWidth = pImage->GetWidth();
    const UINT nHeight = pImage->GetHeight();

    const double dWidthRatio = ((double)sizeMaxNoFrame.Width) / ((double)nWidth);
    const double dHeightRatio = ((double)sizeMaxNoFrame.Height) / ((double)nHeight);
    const double dRatio = min(dWidthRatio, dHeightRatio);    

    const UINT nNewWidthNoFrame = (UINT)Round(dRatio * ((double)nWidth));
    const UINT nNewHeightNoFrame = (UINT)Round(dRatio * ((double)nHeight));

    auto_ptr<Image> pNewImage( new Bitmap(nFrameThick + nNewWidthNoFrame + nFrameThick, 
                                          nFrameThick + nNewHeightNoFrame + nFrameThick, 
                                          PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    const double dShadowRatio = 0.91;
    const Color clrThinFrame(
        (BYTE)Round((double)clrFrame.GetR() * dShadowRatio), 
        (BYTE)Round((double)clrFrame.GetG() * dShadowRatio), 
        (BYTE)Round((double)clrFrame.GetB() * dShadowRatio));

    SolidBrush brushThinFrame(clrThinFrame);
    graphics.FillRectangle(&brushThinFrame, 0, 0, pNewImage->GetWidth(), pNewImage->GetHeight());

    SolidBrush brushFrame(clrFrame);
    graphics.FillRectangle(&brushFrame, 1, 1, pNewImage->GetWidth()-2, pNewImage->GetHeight()-2);

    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.DrawImage(pImage,
        Rect(nFrameThick, nFrameThick, nNewWidthNoFrame, nNewHeightNoFrame),
        0, 0, nWidth, nHeight,
        UnitPixel);

    return pNewImage;
}

auto_ptr<Image> CImageHelper::ScaleAndFrameImage(
            Image* pImage,
            UINT nFrameThick,
            const double& dRatio,
            Color clrFrame /* = Color::WhiteSmoke */
            ) throw(...) // exception
{
    const UINT nWidth = pImage->GetWidth();
    const UINT nHeight = pImage->GetHeight();

    const UINT nNewWidthNoFrame = (UINT)Round(dRatio * ((double)nWidth)) - 2 * nFrameThick;
    const UINT nNewHeightNoFrame = (UINT)Round(dRatio * ((double)nHeight)) - 2 * nFrameThick;

    auto_ptr<Image> pNewImage( new Bitmap(nFrameThick + nNewWidthNoFrame + nFrameThick, 
                                          nFrameThick + nNewHeightNoFrame + nFrameThick, 
                                          PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    SolidBrush brushFrame(clrFrame);
    graphics.FillRectangle(&brushFrame, 0, 0, pNewImage->GetWidth(), pNewImage->GetHeight());

    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.DrawImage(pImage,
        Rect(nFrameThick, nFrameThick, nNewWidthNoFrame, nNewHeightNoFrame),
        0, 0, nWidth, nHeight,
        UnitPixel);

    return pNewImage;
}

// CImageHelper::CreateSolidImage

auto_ptr<Image> CImageHelper::CreateSolidImage(
                    const Size& size,
                    Color clrBackground /* = Color::Black */
                    ) throw(...)
{
    return CreateSolidImage(size.Width, size.Height, clrBackground);
}

auto_ptr<Image> CImageHelper::CreateSolidImage(
                    const UINT nWidth, const UINT nHeight,
                    Color clrBackground /* = Color::Black */
                    ) throw(...)
{
    auto_ptr<Image> pNewImage( new Bitmap(nWidth, nHeight, PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    SolidBrush brushFrame(clrBackground);
    graphics.FillRectangle(&brushFrame, 0, 0, nWidth, nHeight);

    return pNewImage;
}

//
// CImageScatterAnimation class
//

CImageScatterAnimation::CImageScatterAnimation(auto_ptr<Image>& image,
        const Point& ptImageLeftTop, const double& dImageAngleDeg,
        Color clrFrame, const Rect& rectView)
    : m_image(image)
    , m_ptImageLeftTop(ptImageLeftTop)
    , m_dImageAngleDeg(dImageAngleDeg)
    , m_clrFrame(clrFrame)
{
    long minDim = min(m_image->GetWidth(), m_image->GetHeight());
    m_nStepCount = min(rectView.Width, rectView.Height);
    m_dX = (double)(rectView.Width - minDim) / (double)m_nStepCount;
    m_dY = (double)(rectView.Height - minDim) / (double)m_nStepCount;
    m_dAngleDeg = 0.25 * 360.0 / (double)m_nStepCount;
    m_nStep = 0;

    if ( rand() % 2 == 1 )
        m_dX *= -1;
    if ( rand() % 2 == 1 )
        m_dY *= -1;
}

CImageScatterAnimation::CImageScatterAnimation(CImageScatterAnimation& other)
    : m_image(other.m_image)
    , m_ptImageLeftTop(other.m_ptImageLeftTop)
    , m_dImageAngleDeg(other.m_dImageAngleDeg)
    , m_clrFrame(other.m_clrFrame)
{
}

CImageScatterAnimation::~CImageScatterAnimation()
{
}

void CImageScatterAnimation::ResetAnimation()
{
    m_nStep = 0;
}

bool CImageScatterAnimation::NextAnimation(HBITMAP hDstBitmap)
{
    const long nStep = m_nStep++;

    if ( nStep >= m_nStepCount )
    {
        CImagesScatter::DrawImage(hDstBitmap, m_image.get(), 
            m_ptImageLeftTop, m_dImageAngleDeg, m_clrFrame); // exception
    }
    else
    {
        Point pt(
            (int)( (double)m_ptImageLeftTop.X + m_dX * (double)(nStep - m_nStepCount) ),
            (int)( (double)m_ptImageLeftTop.Y + m_dY * (double)(nStep - m_nStepCount) ) );

        double dAngleDeg = 
            m_dImageAngleDeg - m_dAngleDeg * (double)(nStep - m_nStepCount);

        CImagesScatter::DrawImage(hDstBitmap, m_image.get(), 
            pt, dAngleDeg, m_clrFrame); // exception
    }

    return (m_nStep <= m_nStepCount);
}

//
// CImagesScatter class
//

// CImagesScatter constructor/destructor

CImagesScatter::CImagesScatter()
{
}

CImagesScatter::~CImagesScatter()
{
    Clear();
}

// CImagesScatter::Clear

void CImagesScatter::Clear() throw()
{
    _ImageList::iterator i = m_imageList.begin(), iend = m_imageList.end();
    for ( ; i!= iend; ++i )
    {
        delete *i;
    }
    m_imageList.clear();
}

// CImagesScatter::AddImage

void CImagesScatter::AddImage(LPCWSTR szImage) throw(...) // exception
{
    auto_ptr<Image> pImage( new Image(szImage) ); // exception
    AddImage(pImage); // exception
}

void CImagesScatter::AddImage(auto_ptr<Image>& pImage) throw(...) // exception
{
    m_imageList.push_back(pImage.get()); // exception
    pImage.release();
}

// CImagesScatter::Generate

auto_ptr<Image> CImagesScatter::Generate(
                    UINT nWidth, 
                    UINT nHeight,
                    const double& dMaxAngleDeg,
                    UINT nMaxOffset,
                    UINT nFrameThick,
                    Color clrFrame /* = Color::WhiteSmoke */,
                    Color clrBackground /* = Color::White */
                    ) const throw(...) // exception
{
    const Rect rectNewImage(0, 0, nWidth, nHeight);

    auto_ptr<Image> pNewImage( new Bitmap(nWidth, nHeight, PixelFormat32bppARGB) ); // exception

    Graphics graphics(pNewImage.get());
    graphics.ResetTransform();

    SolidBrush brushFrame(clrBackground);
    graphics.FillRectangle(&brushFrame, 0, 0, nWidth, nHeight);

    HBITMAP hBitmap;
    ((Bitmap*)pNewImage.get())->GetHBITMAP(clrBackground, &hBitmap);

    pNewImage.reset();

    _ImageList::const_iterator i = m_imageList.begin(), iend = m_imageList.end();
    for ( UINT cntr = 0; i!= iend; ++i, ++cntr )
    {
        DrawScatterImage(hBitmap, rectNewImage, *i, dMaxAngleDeg, 
                         nMaxOffset, nFrameThick, clrFrame); // exception
    }

    pNewImage = auto_ptr<Image>( Bitmap::FromHBITMAP(hBitmap, NULL) );

    ::DeleteObject(hBitmap);

    return pNewImage;
}

// CImagesScatter::DrawScatterImage

void CImagesScatter::DrawScatterImage(
                    HBITMAP hDstBitmap,
                    const Rect& rect,
                    Image* pImage,
                    const double& dMaxAngleDeg,
                    UINT nMaxOffset,
                    UINT nFrameThick,
                    Color clrFrame
                    ) throw(...) // exception
{
    const Size sizeView(rect.Width, rect.Height);
    const Size sizeImageOriginal(pImage->GetWidth(), pImage->GetHeight());

    // generate shift, scale and rotation
    Size sizeImage;
    Point ptImageLeftTop;
    double dImageAngleDeg;
    CPositionGenerator::Generate(sizeView, sizeImageOriginal, dMaxAngleDeg, nMaxOffset, 
                                 &ptImageLeftTop, &dImageAngleDeg, &sizeImage);

    // make new image that scaled and has frame
    auto_ptr<Image> image = CImageHelper::ScaleAndFrameImage(pImage, nFrameThick, sizeImage, clrFrame); // exception

    // update size image to avoid floating mistakes
    sizeImage.Width = image->GetWidth();
    sizeImage.Height = image->GetHeight();

    // calculate image bounding box
    const Rect& rectBound = CPositionGenerator::GetBoundingRect(sizeImage, dImageAngleDeg);

    // adjust left/top position
    ptImageLeftTop.X += abs( rectBound.X );
    ptImageLeftTop.Y += abs( rectBound.Y );

    // draw image
    DrawImage(hDstBitmap, image.get(), ptImageLeftTop, dImageAngleDeg, clrFrame);
}

auto_ptr<CImageScatterAnimation> CImagesScatter::CreateScatterImageAnimation(
                    const Rect& rect,
                    Image* pImage,
                    const double& dMaxAngleDeg,
                    UINT nMaxOffset,
                    UINT nFrameThick,
                    Color clrFrame
                    ) throw(...) // exception
{
    const Size sizeView(rect.Width, rect.Height);
    const Size sizeImageOriginal(pImage->GetWidth(), pImage->GetHeight());

    // generate shift, scale and rotation
    Size sizeImage;
    Point ptImageLeftTop;
    double dImageAngleDeg;
    CPositionGenerator::Generate(sizeView, sizeImageOriginal, dMaxAngleDeg, nMaxOffset, 
                                 &ptImageLeftTop, &dImageAngleDeg, &sizeImage);

    // make new image that scaled and has frame
    auto_ptr<Image> image = CImageHelper::ScaleAndFrameImage(pImage, nFrameThick, sizeImage, clrFrame); // exception

    // update size image to avoid floating mistakes
    sizeImage.Width = image->GetWidth();
    sizeImage.Height = image->GetHeight();

    // calculate image bounding box
    const Rect& rectBound = CPositionGenerator::GetBoundingRect(sizeImage, dImageAngleDeg);

    // adjust left/top position
    ptImageLeftTop.X += abs( rectBound.X );
    ptImageLeftTop.Y += abs( rectBound.Y );

    auto_ptr<CImageScatterAnimation>
        animator(new CImageScatterAnimation(image, ptImageLeftTop, dImageAngleDeg, clrFrame, rect)); // exception

    return animator;
}

void CImagesScatter::DrawImage(
                    HBITMAP hDstBitmap,
                    Image* pSrcImage,
                    const Point& pt,
                    const double& dAngleDeg,
                    Color clrBackground
                    ) throw(...) // exception
{
    HBITMAP hSrcBitmap = NULL;
    ((Bitmap*)pSrcImage)->GetHBITMAP(clrBackground, &hSrcBitmap);
    
    BITMAP bmpDst = { 0 }, bmpSrc = { 0 };
    ::GetObject(hDstBitmap, sizeof(BITMAP), &bmpDst);
    ::GetObject(hSrcBitmap, sizeof(BITMAP), &bmpSrc);

    const double& dSine = sin( DegToRad(dAngleDeg) );
    const double& dCosine = cos( DegToRad(dAngleDeg) );
    XFORM_MATRIX xForm = { 0 };
    xForm.eM11 = dCosine;
    xForm.eM12 = dSine;
    xForm.eM21 = -dSine;
    xForm.eM22 = dCosine;
    xForm.eDx = pt.X;
    xForm.eDy = pt.Y;

    AATransformBlt(&bmpDst, pt.X, pt.Y, &bmpSrc, 0, 0, bmpSrc.bmWidth, bmpSrc.bmHeight, &xForm);

    ::DeleteObject(hSrcBitmap);
}

//
// CPositionGenerator class 
//

// CPositionGenerator::Generate

void CPositionGenerator::Generate(
                    const Size& sizeView,
                    const Size& sizeObjectOriginal,
                    const double& dMaxAngleDeg,
                    const UINT nMaxDeviation, 
                    Point* pptLeftTop, 
                    double* pdAngleDeg, 
                    Size* pSizeObject,
                    double* pdRatio /* = NULL */
                    ) throw()
{
    const double dAngleDeg = Random( Round(dMaxAngleDeg) );
    const INT nOffsetX = Random( nMaxDeviation );
    const INT nOffsetY = Random( nMaxDeviation );
   
    RECT rectView = { 0, 0, sizeView.Width, sizeView.Height };
    ::OffsetRect(&rectView, nOffsetX, nOffsetY);
    if ( rectView.left < 0 )
        rectView.left = 0;
    if ( rectView.right > sizeView.Width )
        rectView.right = sizeView.Width;
    if ( rectView.top < 0 )
        rectView.top = 0;
    if ( rectView.bottom > sizeView.Height )
        rectView.bottom = sizeView.Height;

    const Size sizeAdjView( rectView.right - rectView.left, rectView.bottom - rectView.top );

    double dRatio;
    Size sizeObjectBound;
    const Size& sizeObject = GetObjectSize(sizeAdjView, sizeObjectOriginal, dAngleDeg,
                                            &dRatio, &sizeObjectBound);

    const Point ptLeftTop(rectView.left + (sizeAdjView.Width - sizeObjectBound.Width) / 2,
                          rectView.top +  (sizeAdjView.Height - sizeObjectBound.Height) / 2);

    *pSizeObject = sizeObject;
    *pdAngleDeg = dAngleDeg;
    *pptLeftTop = ptLeftTop;
    if ( pdRatio != NULL ) 
        *pdRatio = dRatio;
}

// CPositionGenerator::GetObjectSize

Size CPositionGenerator::GetObjectSize(
                    const Size& sizeView, 
                    const Size& sizeObjectOriginal,
                    const double& dAngleDeg,
                    double* pdRatio /* = NULL */,
                    Size* pSizeObjectBound /* = NULL */
                    ) throw()
{
    const Rect& rectObjectBound = GetBoundingRect(sizeObjectOriginal, dAngleDeg);
    const Size sizeObjectBound(rectObjectBound.Width, rectObjectBound.Height);

    const double dWidthRatio = (double)sizeObjectBound.Width / (double)sizeView.Width;
    const double dHeightRatio = (double)sizeObjectBound.Height / (double)sizeView.Height;   
    const double dRatio = max(dWidthRatio, dHeightRatio);    

    if ( pSizeObjectBound != NULL )
    {
        *pSizeObjectBound = Size(
            Round((double)sizeObjectBound.Width / dRatio),
            Round((double)sizeObjectBound.Height / dRatio));
    }
    if ( pdRatio != NULL )
    {
        *pdRatio = dRatio;
    }

    return Size(
        Round((double)sizeObjectOriginal.Width / dRatio),
        Round((double)sizeObjectOriginal.Height / dRatio));
}

// CPositionGenerator::GetBoundingSize

Rect CPositionGenerator::GetBoundingRect(
                    const Size& sizeObject, 
                    const double& dAngleDeg
                    ) throw()
{
    const double& dSine = sin( DegToRad(dAngleDeg) );
    const double& dCosine = cos( DegToRad(dAngleDeg) );

    // x' = x * eM11 + y * eM21 + eDx 
    // y' = x * eM12 + y * eM22 + eDy
    // eM11 = cosine, eM21 = -sine, eM12 = sine, eM22 = cosine

    const double x[2] = { 0, (double)sizeObject.Width };
    const double y[2] = { 0, (double)sizeObject.Height }; 

    INT xMin = INT_MAX, yMin = xMin, 
        xMax = INT_MIN, yMax = xMax;
    for ( INT ix = 0; ix < 2; ++ix )
    {
        for ( INT iy = 0; iy < 2; ++iy )
        {
            INT xt = Round( x[ix] * dCosine - y[iy] * dSine );
            INT yt = Round( x[ix] * dSine + y[iy] * dCosine );
            if ( xMin > xt ) xMin = xt;
            if ( xMax < xt ) xMax = xt;
            if ( yMin > yt ) yMin = yt;
            if ( yMax < yt ) yMax = yt;
        }
    }

    const Rect rectBound(xMin, yMin, xMax - xMin, yMax - yMin);

    return rectBound;
}
