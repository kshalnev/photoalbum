#pragma once

#include <math.h> // sqrt

//
// Pixel format structs
//

#pragma pack(push, 1)

// 24bpp pixel format
struct RGB24
{
    BYTE Blue;
    BYTE Green;
    BYTE Red;
};

// 32bpp pixel format
struct RGB32
{
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Reserved;
};

template <INT bpp> struct PIXELFORMAT {};
template <> struct PIXELFORMAT<24> : public RGB24 {};
template <> struct PIXELFORMAT<32> : public RGB32 {};

#pragma pack(pop)

// 24bpp colors comparison

inline bool operator == (const RGB24 &a, const RGB24 &b)
{ return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue; }
inline bool operator == (const RGB24 &a, COLORREF clr)
{ return a.Red == GetRValue(clr) && a.Green == GetGValue(clr) && a.Blue == GetBValue(clr); }
inline bool operator == (COLORREF clr, const RGB24 &a)
{ return (operator == (a, clr)); }

inline bool operator != (const RGB24 &a, const RGB24 &b)
{ return !(operator == (a, b)); }
inline bool operator != (const RGB24 &a, COLORREF clr)
{ return !(operator == (a, clr)); }
inline bool operator != (COLORREF clr, const RGB24 &a)
{ return !(operator == (a, clr)); }

// 32bpp colors comparison

inline bool operator == (const RGB32 &a, const RGB32 &b)
{ return a.Red == b.Red && a.Green == b.Green && a.Blue == b.Blue; }
inline bool operator == (const RGB32 &a, COLORREF clr)
{ return a.Red == GetRValue(clr) && a.Green == GetGValue(clr) && a.Blue == GetBValue(clr); }
inline bool operator == (COLORREF clr, const RGB32 &a)
{ return (operator == (a, clr)); }

inline bool operator != (const RGB32 &a, const RGB32 &b)
{ return !(operator == (a, b)); }
inline bool operator != (const RGB32 &a, COLORREF clr)
{ return !(operator == (a, clr)); }
inline bool operator != (COLORREF clr, const RGB32 &a)
{ return !(operator == (a, clr)); }

//
// XFORM matrix 
//

// XFORM_MATRIX transformation matrix struct
// Analogous to MSDN XFORM but type is DOUBLE instead of FLOAT
// | eM11 eM12 0 |
// | eM21 eM22 0 |
// | eDx  eDy  1 |
struct XFORM_MATRIX 
{ 
  double eM11; 
  double eM12; 
  double eM21; 
  double eM22; 
  double eDx; 
  double eDy; 
};

// Multiply matrices mRes = mA x mB
// Template method for to be applicable for XFORM and XFORM_MATRIX structs
template <typename TXFORM>
VOID MultMatrix(
        TXFORM *pmRes, 
        const TXFORM *pmA, 
        const TXFORM *pmB)
{
    // 3D matrix is:
    // | eM11 eM12 0 |    | m00 m01 m02 |
    // | eM21 eM22 0 | or | m10 m11 m12 |
    // | eDx  eDy  1 |    | m20 m21 m22 |
    // Multiplication result is:
    // | r00 r01 r02 | = | (a00xb00 + a01xb10 + a02xb20) (a00xb01 + a01xb11 + a02xb21) (a00xb02 + a01xb12 + a02xb22) |
    // | r10 r11 r12 | = | (a10xb00 + a11xb10 + a12xb20) (a10xb01 + a11xb11 + a12xb21) (a10xb02 + a11xb12 + a12xb22) | 
    // | r20 r21 r22 | = | (a20xb00 + a21xb10 + a22xb20) (a20xb01 + a21xb11 + a22xb21) (a20xb02 + a21xb12 + a22xb22) |
    // Since for XFORM_MATRIX m02 = 0, m12 = 0 and m22 = 1 (i.e. for a and b matrices) then result is
    // | r00 r01 r02 | = | (a00xb00 + a01xb10)       (a00xb01 + a01xb11)       0 |
    // | r10 r11 r12 | = | (a10xb00 + a11xb10)       (a10xb01 + a11xb11)       0 | 
    // | r20 r21 r22 | = | (a20xb00 + a21xb10 + b20) (a20xb01 + a21xb11 + b21) 1 |
    
    ASSERT(pmRes != NULL);
    ASSERT(pmA != NULL);
    ASSERT(pmB != NULL);

    pmRes->eM11 = pmA->eM11 * pmB->eM11 + pmA->eM12 * pmB->eM21;            // a00xb00 + a01xb10
    pmRes->eM12 = pmA->eM11 * pmB->eM12 + pmA->eM12 * pmB->eM22;            // a00xb01 + a01xb11
    pmRes->eM21 = pmA->eM21 * pmB->eM11 + pmA->eM22 * pmB->eM21;            // a10xb00 + a11xb10
    pmRes->eM22 = pmA->eM21 * pmB->eM12 + pmA->eM22 * pmB->eM22;            // a10xb01 + a11xb11
    pmRes->eDx  = pmA->eDx  * pmB->eM11 + pmA->eDy  * pmB->eM21 + pmB->eDx; // a20xb00 + a21xb10 + b20
    pmRes->eDy  = pmA->eDx  * pmB->eM12 + pmA->eDy  * pmB->eM22 + pmB->eDy; // a20xb01 + a21xb11 + b21
}

//
// Advanced bitmap API
//

// Calculate bound box for transformed rect 
inline VOID AAGetTransformationBoundBox(
        const RECT *prcSrc, 
        const XFORM_MATRIX *pMatrix, 
        RECT *prDst)
{
    ASSERT(prcSrc != NULL);
    ASSERT(pMatrix != NULL);
    ASSERT(prDst != NULL);

    const POINT ptSrc[4] = 
    {  
        { prcSrc->left, prcSrc->top },
        { prcSrc->right, prcSrc->top },
        { prcSrc->left, prcSrc->bottom },
        { prcSrc->right, prcSrc->bottom },
    };
    const POINT *ppt = ptSrc;
    for (INT i=0 ; i<4 ; ++i, ++ppt)
    {
        // Transformation formula is:
        // dx = sx * eM11 + sy * eM21 + eDx
        // dy = sx * eM12 + sy * eM22 + eDy
        INT x = (INT)( ppt->x * pMatrix->eM11 + ppt->y * pMatrix->eM21 + pMatrix->eDx );
        INT y = (INT)( ppt->x * pMatrix->eM12 + ppt->y * pMatrix->eM22 + pMatrix->eDy );
        if (i == 0)
        {
            prDst->left = prDst->right = x;
            prDst->top = prDst->bottom = y;
        }
        else
        {
            if (prDst->left > x)
                prDst->left = x;
            if (prDst->right < x)
                prDst->right = x;
            if (prDst->top > y)
                prDst->top = y;
            if (prDst->bottom < y)
                prDst->bottom = y;
        }
    }
}

// AAGetAverageColor method
// Calculate average color in extent.
// Extent is defined as [nX...nX+nAvrCntX)[nY...nY+nAvrCntY)
// If defined pClrKey not null then pixels coresponding to color key
// will be replaced by pSubstitutePixel color
template <typename PIXELSRC, typename PIXELDST, typename PIXELSUB>
INT AAGetAverageColor(
        const BITMAP *pSrcBitmap, 
        INT nX, 
        INT nY, 
        INT nAvrCntX, 
        INT nAvrCntY, 
        PIXELDST *pAvrPixel, 
        const COLORREF *pClrKey = NULL, 
        PIXELSUB *pSubstitutePixel = NULL)
{
    ASSERT(pAvrPixel != NULL);
    ASSERT(pSrcBitmap != NULL);
    ASSERT(pSrcBitmap->bmBits != NULL);
    ASSERT(pSrcBitmap->bmBitsPixel == sizeof(PIXELSRC) * 8);

    #ifdef _DEBUG
    if (pClrKey != NULL) 
        ASSERT(pSubstitutePixel != NULL);
    #endif
    
    INT nWidth = pSrcBitmap->bmWidth;
    INT nHeight = pSrcBitmap->bmHeight;
    INT nWidthBytes = pSrcBitmap->bmWidthBytes;

    INT yFrom = nY, yTo = nY + nAvrCntY;
    INT xFrom = nX, xTo = nX + nAvrCntX;

    if (xTo < xFrom)
    {
        int i = xFrom;
        xFrom = xTo;
        xTo = i;
    }

    if (yTo < yFrom) 
    {
        int i = yFrom;
        yFrom = yTo; 
        yTo = i;
    }

    if (xFrom >= nWidth)
        xFrom = nWidth - 1;
    else if (xFrom < 0)
        xFrom = 0;

    if (xTo >= nWidth)
        xTo = nWidth;
    else if (xTo < 0)
        xTo = 0;

    if (yFrom >= nHeight)
        yFrom = nHeight - 1;
    else if (yFrom < 0)
        yFrom = 0;

    if (yTo >= nHeight)
        yTo = nHeight;
    else if (yTo < 0)
        yTo = 0;

    ASSERT(xFrom >= 0 && xFrom < nWidth);
    ASSERT(xTo >= 0 && xTo <= nWidth);
    ASSERT(yFrom >= 0 && yFrom < nHeight);
    ASSERT(yTo >= 0 && yTo <= nHeight);

    INT nCnt = 0;
    INT nR = 0, nG = 0, nB = 0;

    const BYTE *pBits = (const BYTE *)pSrcBitmap->bmBits + yFrom * nWidthBytes + xFrom * sizeof(PIXELSRC);
    const BYTE *pBitsTo = pBits + (yTo - yFrom) * nWidthBytes;
    INT nPixelsInRow = xTo - xFrom;

    do 
    {
        const PIXELSRC *p = (const PIXELSRC *)pBits;
        const PIXELSRC *pTo = p + nPixelsInRow;

        do
        {
            if (pClrKey != NULL && *pClrKey == *p)
            {
                ASSERT(pSubstitutePixel != NULL);
                nR += pSubstitutePixel->Red;
                nG += pSubstitutePixel->Green;
                nB += pSubstitutePixel->Blue;
            }
            else
            {
                nR += p->Red;
                nG += p->Green;
                nB += p->Blue;
            }

            ++nCnt;
            ++p;
        }
        while (p < pTo);

        pBits += nWidthBytes;
    }
    while (pBits < pBitsTo);
    
    ASSERT(nCnt > 0);
    
    pAvrPixel->Red   = (BYTE)(nR / nCnt);
    pAvrPixel->Green = (BYTE)(nG / nCnt);
    pAvrPixel->Blue  = (BYTE)(nB / nCnt);

    return nCnt;
}

// Transform (rotate/scale) bitmap and set onto destination 
// bitmap in predefined left/top position
template <typename PIXELSRC, typename PIXELDST>
VOID AATransformBltTempl(
        const BITMAP *pDstBitmap, 
        INT nDstX,
        INT nDstY,
        const BITMAP *pSrcBitmap,
        INT nSrcX,
        INT nSrcY,
        INT nSrcWidth,
        INT nSrcHeight,
        const XFORM_MATRIX *pMatrix,
        const COLORREF *pClrKey = NULL)
{
    ASSERT(pMatrix != NULL);
    ASSERT(pDstBitmap != NULL);
    ASSERT(pDstBitmap->bmBits != NULL);
    ASSERT(pSrcBitmap != NULL);
    ASSERT(pSrcBitmap->bmBits != NULL);
    ASSERT(pSrcBitmap->bmBitsPixel == sizeof(PIXELSRC) * 8);
    ASSERT(pDstBitmap->bmBitsPixel == sizeof(PIXELDST) * 8);

    // Destination bitmap
    INT nDstBitmapWidth = pDstBitmap->bmWidth;
    INT nDstBitmapHeight = pDstBitmap->bmHeight;
    INT nDstBitmapWidthBytes = pDstBitmap->bmWidthBytes;

    // Source bitmap
    INT nSrcBitmapWidth = pSrcBitmap->bmWidth;
    INT nSrcBitmapHeight = pSrcBitmap->bmHeight;
    INT nSrcBitmapWidthBytes = pSrcBitmap->bmWidthBytes;

    // Override matrix for adjust eDx and eDy for matrix
    // Since bitmap transformed from src to dst then eDx and eDy shoul be:
    XFORM_MATRIX matrix(*pMatrix);
    matrix.eDx = nDstX - nSrcX;
    matrix.eDy = nDstY - nSrcY;
    pMatrix = &matrix;

    // Calculation destination position
    RECT rDst = { 0 };
    RECT rSrc = { nSrcX-1, nSrcY-1, nSrcX+nSrcWidth+1, nSrcY+nSrcHeight+1 };
    AAGetTransformationBoundBox(&rSrc, pMatrix, &rDst);
    if (rDst.left < 0)
        rDst.left = 0;
    if (rDst.top < 0)
        rDst.top = 0;
    if (rDst.right >= nDstBitmapWidth)
        rDst.right = nDstBitmapWidth - 1;
    if (rDst.bottom >= nDstBitmapHeight)
        rDst.bottom = nDstBitmapHeight - 1;

    // Ratio source size to destination size
    // Scaling/rotation matrix is:
    // | ( cos x kx) (sin x ky) ... |  i.e. | m00 m01 ... |
    // | (-sin x kx) (cos x ky) ... |       | m10 m10 ... |
    // | ...                    ... |       | ...         |
    // i.e.
    // kx = sqrt( m00 x m00 + m10 x m10 )
    // ky = sqrt( m01 x m01 + m11 x m11 )
    double kx = 1.0 / sqrt( matrix.eM11 * matrix.eM11 + matrix.eM21 * matrix.eM21 ); // src pixel per one dst pixel 
    double ky = 1.0 / sqrt( matrix.eM12 * matrix.eM12 + matrix.eM22 * matrix.eM22 ); // src pixel per one dst pixel 

    // Averaging points number
    INT nAvrSrcX = (INT)kx; 
    INT nAvrSrcY = (INT)ky; 
    if (kx > nAvrSrcX)
        nAvrSrcX += 1;
    if (ky > nAvrSrcY)
        nAvrSrcY += 1;

    // Transformation is
    const double &eM11 = pMatrix->eM11;
    const double &eM12 = pMatrix->eM12;
    const double &eM21 = pMatrix->eM21;
    const double &eM22 = pMatrix->eM22;
    const double &eDx  = pMatrix->eDx;
    const double &eDy  = pMatrix->eDy;

    // Transformation formula is:
    // dx = sx * eM11 + sy * eM21 + eDx
    // dy = sx * eM12 + sy * eM22 + eDy
    // for case eM21 = eM12 = 0 
    // dx = sx * eM11 + eDx
    // dy = sy * eM22 + eDy

    // Revert transformation formula is:
    // sx = (eM21 * (dy - eDy) - eM22 * (dx - eDx)) / (eM12 * eM21 - eM11 * eM22)
    // sy = (dx - eDx) / eM21 - sx * eM11 / eM21
    // for case eM21 = eM12 = 0 
    // sx = (dx - eDx) / eM11
    // sy = (dy - eDy) / eM22
    // Precalculates are:
    double d  = eM12 * eM21 - eM11 * eM22;
    double a1 = eM21 / d;
    double a2 = eM22 / d;
    double b1 = eDx * a2 - eDy * a1;
    double e1 = eM11 / eM21;
    double e2 = eDx / eM21;
    double a3 = (e1 * a2 + 1.0 / eM21);
    double a4 = e1 * a1;
    double b2 = e2 + e1 * b1;
    // Therefore formula is:
    // sx = a1 * dy - a2 * dx + b1
    // sy = a3 * dx - a4 * dy - b2
    double a5 = eDx / eM11;
    double a6 = eDy / eM22;
    double a7 = 1.0 / eM11;
    double a8 = 1.0 / eM22;
    // Therefore formula for case without rotation is:
    // sx = dx * a7 - a5
    // sy = dy * a8 - a6
  
    BOOL bNoRotation = (eM21 >= -1e-6 && eM21 <= 1e-6);
    ASSERT((eM21 >= -1e-6 && eM21 <= 1e-6) == (eM12 >= -1e-6 && eM12 <= 1e-6));

    BOOL bPixelBilinear = (kx <= 1.75 && ky <= 1.75);
        // Two cases diffently handled:
        // 1. Pixel bilinear filtering
        //    when source is smaller than destination (S < D)
        // 2. Chuncks bilinear filtering
        //    when source is bigger than destination (S > D)

    BYTE *pDst = (BYTE *)pDstBitmap->bmBits + rDst.top * nDstBitmapWidthBytes;
    const BYTE *pSrc = (const BYTE *)pSrcBitmap->bmBits;

    // Scaling ratio
    const INT iSHIFT = 8;
    const INT iSCALE = 1 << iSHIFT;
    const double dSCALE = (double)iSCALE;

    INT nSrcBitmapWidthScaled = (nSrcBitmapWidth << iSHIFT);
    INT nSrcBitmapHeightScaled = (nSrcBitmapHeight << iSHIFT);
    INT nSrcLastAvailIndexX = nSrcBitmapWidth-1;
    INT nSrcLastAvailIndexY = nSrcBitmapHeight-1;

    INT sxFrom = 0, syFrom = 0; // start for sx and sy per dy
    INT sxStep = 0, syStep = 0; // step for sx and sy per dx
    INT sxNext = 0, syNext = 0; // step for sx and sy per dy
    INT rxStep = 0, ryStep = 0; // remainder for step per dx
    INT rxNext = 0, ryNext = 0; // remainder for step per dy
    INT rxFrom = 0, ryFrom = 0; // remainder of from point

    // Remainder compensators
    INT sxStepCorr = 1, syStepCorr = 1;
    INT sxNextCorr = 1, syNextCorr = 1;

    if (bNoRotation)
    {
        // sx = dx * a7 - a5
        // sy = dy * a8 - a6
        double fxFrom = ((double)(rDst.left) * a7 - a5) * dSCALE + 1e-6;
        double fyFrom = ((double)(rDst.top) * a8 - a6) * dSCALE + 1e-6;
        double fxStep = a7 * dSCALE;
        double fyNext = a8 * dSCALE;

        sxFrom = (INT)(fxFrom); 
        syFrom = (INT)(fyFrom);
        sxStep = (INT)(fxStep);
        syStep = 0;
        syNext = (INT)(fyNext);
        sxNext = 0;
        rxStep = (INT)((fxStep - (double)sxStep) * dSCALE);
        ryNext = (INT)((fyNext - (double)syNext) * dSCALE);
        rxFrom = (INT)((fxFrom - (double)sxFrom) * dSCALE);
        ryFrom = (INT)((fyFrom - (double)syFrom) * dSCALE);
    }
    else
    {
        // sx = a1 * dy - a2 * dx + b1
        // sy = a3 * dx - a4 * dy - b2
        double fxFrom = ((a1 * (double)(rDst.top) + b1) - a2 * (double)(rDst.left)) * dSCALE;
        double fyFrom = (a3 * (double)(rDst.left) - (b2 + a4 * (double)(rDst.top))) * dSCALE;
        double fxStep = -a2 * dSCALE; // step per one dx 
        double fxNext = a1 * dSCALE;  // step per one dy
        double fyStep = a3 * dSCALE;  // step per one dx
        double fyNext = -a4 * dSCALE; // step per one dy

        sxFrom = (INT)(fxFrom);
        syFrom = (INT)(fyFrom); 
        sxStep = (INT)(fxStep); 
        syStep = (INT)(fyStep); 
        sxNext = (INT)(fxNext);
        syNext = (INT)(fyNext);
        rxStep = (INT)((fxStep - (double)sxStep) * dSCALE);
        ryStep = (INT)((fyStep - (double)syStep) * dSCALE);
        rxNext = (INT)((fxNext - (double)sxNext) * dSCALE);
        ryNext = (INT)((fyNext - (double)syNext) * dSCALE);
        rxFrom = (INT)((fxFrom - (double)sxFrom) * dSCALE);
        ryFrom = (INT)((fyFrom - (double)syFrom) * dSCALE);
    }

    if (rxFrom < 0)
        sxFrom -= 1, rxFrom += iSCALE;
    if (ryFrom < 0)
        syFrom -= 1, ryFrom += iSCALE;
    if (rxStep < 0)
        rxStep = -rxStep, sxStepCorr = -1;
    if (ryStep < 0)
        ryStep = -ryStep, syStepCorr = -1;
    if (rxNext < 0)
        rxNext = -rxNext, sxNextCorr = -1;
    if (ryNext < 0)
        ryNext = -ryNext, syNextCorr = -1;

    // Accumulative remainder
    INT rxNextAcc = rxFrom, ryNextAcc = ryFrom;

    for (INT dy = rDst.top ; dy <= rDst.bottom ; ++dy, pDst += nDstBitmapWidthBytes)
    {   
        // Remainder compensation
        rxNextAcc += rxNext; 
        ryNextAcc += ryNext;
        if (rxNextAcc >= iSCALE)
            rxNextAcc -= iSCALE, sxFrom += sxNextCorr;
        if (ryNextAcc >= iSCALE)
            ryNextAcc -= iSCALE, syFrom += syNextCorr;

        INT sx = sxFrom, sy = syFrom;

        // Accumulative remainder
        INT rxStepAcc = rxNextAcc, ryStepAcc = ryNextAcc;

        PIXELDST *pDstPixel = (PIXELDST *)pDst + rDst.left;

        for (INT dx = rDst.left ; dx <= rDst.right ; ++dx, ++pDstPixel)
        {
            // Remainder compensation
            rxStepAcc += rxStep;
            ryStepAcc += ryStep;
            if (rxStepAcc >= iSCALE)
                rxStepAcc -= iSCALE, sx += sxStepCorr;
            if (ryStepAcc >= iSCALE)
                ryStepAcc -= iSCALE, sy += syStepCorr;

            if (sx > -iSCALE && sy > -iSCALE && sx < nSrcBitmapWidthScaled && sy < nSrcBitmapHeightScaled)
            {
                INT x = sx >> iSHIFT; // source point X
                INT y = sy >> iSHIFT; // source point Y

                ASSERT(x >= -1 && x < nSrcBitmapWidth);
                ASSERT(y >= -1 && y < nSrcBitmapHeight);

                if (bPixelBilinear)
                {
                    // Blending nearest pixels
                    // Destination point placed inside 0 area

                    INT du  = (sx % iSCALE) >> (iSHIFT - 8); 
                    INT dv  = (sy % iSCALE) >> (iSHIFT - 8);
                    if (du < 0)
                        du += iSCALE;
                    if (dv < 0)
                        dv += iSCALE;
                    INT dui = 255 - du;
                    INT dvi = 255 - dv;

                    INT a = dui * dvi; // blending coefficient for pixel 1    a b
                    INT b = du  * dvi; // blending coefficient for pixel 2    c d
                    INT c = dui * dv;  // blending coefficient for pixel 3
                    INT d = du  * dv;  // blending coefficient for pixel 4

                    // Source pixels 1, 2, 3 and 4
                    const PIXELSRC *p1 = (const PIXELSRC *)(pSrc + y * nSrcBitmapWidthBytes) + x; // row 0
                    const PIXELSRC *p3 = (const PIXELSRC *)((BYTE *)p1 + nSrcBitmapWidthBytes);   // row 1
                    if (y == -1)
                        p1 = p3, a = 0, b = 0;
                    else if (y == nSrcLastAvailIndexY)
                        p3 = p1, c = 0, d = 0;
                    const PIXELSRC *p2 = p1 + 1; // col 0
                    const PIXELSRC *p4 = p3 + 1; // col 1
                    if (x == -1)
                        p1 = p2, p3 = p4, a = 0, c = 0;
                    else if (x == nSrcLastAvailIndexX)
                        p2 = p1, p4 = p3, b = 0, d = 0;

                    if (pClrKey != NULL)
                    {
                        COLORREF clrKey = *pClrKey;
                        if (*p1 == clrKey)
                            a = 0;
                        if (*p2 == clrKey)
                            b = 0;
                        if (*p3 == clrKey)
                            c = 0;
                        if (*p4 == clrKey)
                            d = 0;
                    }

                    INT ratio = a + b + c + d;

                    // Calculation transparency
                    // 0 fully transparent, 255 fully opaque
                    INT bAlpha = (INT)( ratio / 255 );

                    if (bAlpha > 0)
                    {
                        BYTE Red   = (BYTE)( ( p1->Red   * a + p2->Red   * b + p3->Red   * c + p4->Red   * d ) / ratio );
                        BYTE Green = (BYTE)( ( p1->Green * a + p2->Green * b + p3->Green * c + p4->Green * d ) / ratio );
                        BYTE Blue  = (BYTE)( ( p1->Blue  * a + p2->Blue  * b + p3->Blue  * c + p4->Blue  * d ) / ratio );

                        if (bAlpha == 255)
                        {
                            pDstPixel->Red = Red;
                            pDstPixel->Green = Green;
                            pDstPixel->Blue = Blue;
                        } 
                        else 
                        {
                            ASSERT(bAlpha > 0 && bAlpha < 255);
                            INT bOneMinusAlpha = 255 - bAlpha;
                            pDstPixel->Red   = (BYTE)( (pDstPixel->Red   * bOneMinusAlpha + Red   * bAlpha) >> 8 );
                            pDstPixel->Green = (BYTE)( (pDstPixel->Green * bOneMinusAlpha + Green * bAlpha) >> 8 );
                            pDstPixel->Blue  = (BYTE)( (pDstPixel->Blue  * bOneMinusAlpha + Blue  * bAlpha) >> 8 );
                        }
                    }
                }
                else 
                {
                    // Blending nearest chunks
                    // Destination point placed inside in areas intersection

                    INT dui = (INT)( (double)((sx % iSCALE) >> (iSHIFT - 8)) / kx );
                    INT dvi = (INT)( (double)((sy % iSCALE) >> (iSHIFT - 8)) / ky ); 
                    INT du  = 255 - dui;
                    INT dv  = 255 - dvi;

                    // Averaged source color of chunk 1, 2, 3 and 4
                    PIXELSRC p[4]; 
                    PIXELSRC *p1 = p, *p2 = p+1, *p3 = p+2, *p4 = p+3;

                    // Calculation average chunks color and 
                    // weight of chunks for dest pixel color blending
                    INT na = AAGetAverageColor<PIXELSRC>(pSrcBitmap, x, y, -nAvrSrcX, -nAvrSrcY, p+0, pClrKey, pDstPixel);
                    INT nb = AAGetAverageColor<PIXELSRC>(pSrcBitmap, x, y,  nAvrSrcX, -nAvrSrcY, p+1, pClrKey, pDstPixel);
                    INT nc = AAGetAverageColor<PIXELSRC>(pSrcBitmap, x, y, -nAvrSrcX,  nAvrSrcY, p+2, pClrKey, pDstPixel);
                    INT nd = AAGetAverageColor<PIXELSRC>(pSrcBitmap, x, y,  nAvrSrcX,  nAvrSrcY, p+3, pClrKey, pDstPixel);
            
                    double a = na * dui * dvi; // blending coefficient for chunk 0     a b
                    double b = nb * du  * dvi; // blending coefficient for chunk 1     c d
                    double c = nc * dui * dv;  // blending coefficient for chunk 2
                    double d = nd * du  * dv;  // blending coefficient for chunk 3
                    double ratio = a + b + c + d;

                    pDstPixel->Red   = (BYTE)( ( p1->Red   * a + p2->Red   * b + p3->Red   * c + p4->Red   * d ) / ratio );
                    pDstPixel->Green = (BYTE)( ( p1->Green * a + p2->Green * b + p3->Green * c + p4->Green * d ) / ratio );
                    pDstPixel->Blue  = (BYTE)( ( p1->Blue  * a + p2->Blue  * b + p3->Blue  * c + p4->Blue  * d ) / ratio );
                }
            }

            sx += sxStep, sy += syStep;
        }

        sxFrom += sxNext, syFrom += syNext;
    }
}

//
// API
//

inline VOID AATransformBlt(
        const BITMAP *pDstBitmap, 
        INT nDstX,
        INT nDstY,
        const BITMAP *pSrcBitmap,
        INT nSrcX,
        INT nSrcY,
        INT nSrcWidth,
        INT nSrcHeight,
        const XFORM_MATRIX *pMatrix,
        const COLORREF *pClrKey = NULL)
{
    ASSERT(pMatrix != NULL);
    ASSERT(pDstBitmap != NULL);
    ASSERT(pDstBitmap->bmBits != NULL);
    ASSERT(pSrcBitmap != NULL);
    ASSERT(pSrcBitmap->bmBits != NULL);
    ASSERT(pSrcBitmap->bmBitsPixel == 24 || pSrcBitmap->bmBitsPixel == 32);
    ASSERT(pDstBitmap->bmBitsPixel == 24 || pDstBitmap->bmBitsPixel == 32);

    typedef PIXELFORMAT<24> PF24;
    typedef PIXELFORMAT<32> PF32;

    if (pSrcBitmap->bmBitsPixel == pDstBitmap->bmBitsPixel)
    {
        switch(pSrcBitmap->bmBitsPixel)
        {
        case 24:
            AATransformBltTempl<PF24, PF24>(
                    pDstBitmap, nDstX, nDstY, pSrcBitmap, nSrcX, nSrcY, nSrcWidth, nSrcHeight, pMatrix, pClrKey);
            break;
        case 32:
            AATransformBltTempl<PF32, PF32>(
                    pDstBitmap, nDstX, nDstY, pSrcBitmap, nSrcX, nSrcY, nSrcWidth, nSrcHeight, pMatrix, pClrKey);
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    else if (pDstBitmap->bmBitsPixel == 24)
    {
        if (pSrcBitmap->bmBitsPixel == 32)
            AATransformBltTempl<PF32, PF24>(
                    pDstBitmap, nDstX, nDstY, pSrcBitmap, nSrcX, nSrcY, nSrcWidth, nSrcHeight, pMatrix, pClrKey);
        else
            ASSERT(FALSE);
    }
    else if (pDstBitmap->bmBitsPixel == 32)
    {
        if (pSrcBitmap->bmBitsPixel == 24)
            AATransformBltTempl<PF24, PF32>(
                    pDstBitmap, nDstX, nDstY, pSrcBitmap, nSrcX, nSrcY, nSrcWidth, nSrcHeight, pMatrix, pClrKey);
        else
            ASSERT(FALSE);
    }
}

inline VOID AAStretchBlt(
        const BITMAP *pDstBitmap, 
        INT nDstX, 
        INT nDstY, 
        INT nDstWidth, 
        INT nDstHeight, 
        const BITMAP *pSrcBitmap,
        const COLORREF *pClrKey = NULL,
        BOOL bInvertYSrc = FALSE)
{
    ASSERT(pSrcBitmap != NULL);
    ASSERT(pDstBitmap != NULL);

    XFORM_MATRIX matrix;
    matrix.eM11 = (double)nDstWidth / (double)pSrcBitmap->bmWidth;
    matrix.eM22 = (double)nDstHeight / (double)pSrcBitmap->bmHeight;
    matrix.eM21 = 0;
    matrix.eM12 = 0; 
    matrix.eDx  = 0;
    matrix.eDy  = 0;

    if (bInvertYSrc)
    {
        matrix.eM22 = -matrix.eM22;
        nDstY += nDstHeight;
    }

    AATransformBlt(
        pDstBitmap, 
        nDstX, 
        nDstY, 
        pSrcBitmap, 
        0, 
        0, 
        pSrcBitmap->bmWidth, 
        pSrcBitmap->bmHeight, 
        &matrix, 
        pClrKey);
}


