#pragma once

//
// CImageHelper static class
//

class CImageHelper
{
public:
    static BOOL GetEncoderClsid(
            LPCWSTR szFormat, 
            CLSID* pClsid
            ) throw();

    static auto_ptr<Image> FrameImage(
            Image* pImage, 
            UINT nFrameThick, 
            Color clrFrame = Color::WhiteSmoke
            ) throw(...); // exception

    static auto_ptr<Image> ScaleImage(
            Image* pImage, 
            const Size& sizeMax
            ) throw(...); // exception

    static auto_ptr<Image> ScaleImage(
            Image* pImage, 
            const double& dRatio
            ) throw(...); // exception

    static auto_ptr<Image> ScaleAndFrameImage(
            Image* pImage,
            UINT nFrameThick,
            const Size& sizeMax,
            Color clrFrame = Color::WhiteSmoke
            ) throw(...); // exception

    static auto_ptr<Image> ScaleAndFrameImage(
            Image* pImage,
            UINT nFrameThick,
            const double& dRatio,
            Color clrFrame = Color::WhiteSmoke
            ) throw(...); // exception

    static auto_ptr<Image> CreateSolidImage(
            const Size& size,
            Color clrBackground = Color::Black
            ) throw(...);
    static auto_ptr<Image> CreateSolidImage(
            const UINT nWidth, const UINT nHeight,
            Color clrBackground = Color::Black
            ) throw(...);
};

//
// CImageScatterAnimation class
//

class CImageScatterAnimation
{
private:
    friend class CImagesScatter;

    CImageScatterAnimation(auto_ptr<Image>& image,
        const Point& ptImageLeftTop, const double& dImageAngleDeg,
        Color clrFrame, const Rect& rectView);   

public:
    CImageScatterAnimation(CImageScatterAnimation&);
    ~CImageScatterAnimation();

    void ResetAnimation();
    bool NextAnimation(HBITMAP hDstBitmap);

private:
    // target parameters
    auto_ptr<Image> m_image;
    const Point m_ptImageLeftTop;
    const double m_dImageAngleDeg; 
    const Color m_clrFrame;

    // current parameters
    double m_dX;
    double m_dY;
    double m_dAngleDeg;
    long m_nStepCount;
    long m_nStep;
};

//
// CImagesScatter class
//

class CImagesScatter
{
public:
    CImagesScatter();
    ~CImagesScatter();

    void Clear() throw();
    void AddImage(auto_ptr<Image>& pImage) throw(...); // exception
    void AddImage(LPCWSTR szImage) throw(...); // exception

    auto_ptr<Image> Generate(
            UINT nWidth, 
            UINT nHeight, 
            const double& dMaxAngleDeg,
            UINT nMaxOffset,
            UINT nFrameThick,
            Color clrFrame = Color::WhiteSmoke,
            Color clrBackground = Color::White
            ) const throw(...); // exception

    static void DrawScatterImage(
        HBITMAP hDstBitmap, 
        const Rect& rect,
        Image* pImage,
        const double& dMaxAngleDeg,
        UINT nMaxOffset,
        UINT nFrameThick,
        Color clrFrame
        ) throw(...); // exception

    static auto_ptr<CImageScatterAnimation> CreateScatterImageAnimation(
        const Rect& rect,
        Image* pImage,
        const double& dMaxAngleDeg,
        UINT nMaxOffset,
        UINT nFrameThick,
        Color clrFrame
        ) throw(...); // exception

    static void DrawImage(
        HBITMAP hDstBitmap,
        Image* pSrcImage,
        const Point& pt,
        const double& dAngleDeg,
        Color clrBackground
        ) throw(...); // exception

private:
    typedef list<Image*> _ImageList;
    _ImageList m_imageList;
};

//
// CPositionGenerator static class
//

class CPositionGenerator
{
public:
    static void Generate(
            const Size& sizeView,
            const Size& sizeObjectOriginal,
            const double& dMaxAngleDeg,
            const UINT nMaxDeviation, 
            Point* pptLeftTop, 
            double* pdAngleDeg, 
            Size* pSizeObject,
            double* pdRatio = NULL
            ) throw();

    static Size GetObjectSize(
            const Size& sizeView,
            const Size& sizeObjectOriginal,
            const double& dAngleDeg,
            double* pdRatio = NULL,
            Size* psizeObjectBound = NULL
            ) throw();

    static Rect GetBoundingRect(
            const Size& sizeObject, 
            const double& dAngleDeg
            ) throw();
};
