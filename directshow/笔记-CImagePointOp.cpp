HRESULT CImageOp::SetFormat(const VIDEOINFOHEADER& vih)
{
  // Check if UYVY.
  if (vih.bmiHeader.biCompression != 'YVYU')
  {
    return E_INVALIDARG;
  }
  int BytesPerPixel = vih.bmiHeader.biBitCount / 8;
  
  // If the target rectangle (rcTarget) is empty,
  // the image width and the stride are both biWidth.
  // Otherwise, image width is given by rcTarget
  // and the stride is given by biWidth.
  if (IsRectEmpty(&vih.rcTarget))
  {
    m_dwWidth = vih.bmiHeader.biWidth;
    m_lStride = m_dwWidth;
  }
  else
  {
    m_dwWidth = vih.rcTarget.right;
    m_lStride = vih.bmiHeader.biWidth;
  }
  
  // Stride for UYVY is rounded to the nearest DWORD.
  m_lStride = (m_lStride * BytesPerPixel + 3) & ~3;
  
  // biHeight can be < 0, but YUV is always top-down.
  m_dwHeight = abs(vih.bmiHeader.biHeight);
  m_iBitDepth = vih.bmiHeader.biBitCount;
  return S_OK;
}


HRESULT CEqualize::_ScanImage()
{
  DWORD iRow, iPixel; // looping variables
  BYTE *pRow = m_pImg; // pointer to the first row in the buffer
  DWORD histogram[LUMA_RANGE]; // basic histogram
  double nrm_histogram[LUMA_RANGE]; // normalized histogram
  ZeroMemory(histogram, sizeof(histogram));
  
  
  // Create a histogram.
  // For each pixel, find the luma and increment the count for that
  // pixel. Luma values are translated
  // from the nominal 16-235 range to a 0-219 array.
  for (iRow = 0; iRow < m_dwHeight; iRow++)
  {
    UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);
    for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
    {
      BYTE luma = pPixel->y;
      luma = static_cast<BYTE>(clipYUV(luma)) - MIN_LUMA;
      histogram[luma]++;
    }
    pRow += m_lStride;
  }
  
  // Build the cumulative histogram.
  for (int i = 1; i < LUMA_RANGE; i++)
  {
    // The i'th entry is the sum of the previous entries.
    histogram[i] = histogram[i-1] + histogram[i];
  }
  
  // Normalize the histogram.
  DWORD area = NumPixels();
  for (int i = 0; i < LUMA_RANGE; i++)
  {
    nrm_histogram[i] =
    static_cast<double>( LUMA_RANGE * histogram[i] ) / area;
  }

  // Create the LUT.
  for (int i = 0; i < LUMA_RANGE; i++)
  {
    // Clip the result to the nominal luma range.
    long rounded = static_cast<long>(nrm_histogram[i] + 0.5);
    long clipped = clip(rounded, 0, LUMA_RANGE - 1);
    m_LookUpTable[i] = static_cast<BYTE>(clipped) + MIN_LUMA;
  }
  return S_OK;
}

HRESULT CImagePointOp::_ConvertImage()
{
  DWORD iRow, iPixel; // looping variables
  BYTE *pRow = m_pImg; // pointer to the first row in the buffer
  for (iRow = 0; iRow < m_dwHeight; iRow++)
  {
    UYVY_PIXEL *pPixel = reinterpret_cast<UYVY_PIXEL*>(pRow);
    for (iPixel = 0; iPixel < m_dwWidth; iPixel++, pPixel++)
    {
      // Translate luma back to 0-219 range.
      BYTE luma = (BYTE)clipYUV(pPixel->y) - MIN_LUMA;
      
      // Convert from LUT.
      // The result is already in the correct 16-239 range.
      pPixel->y = m_LookUpTable[luma];
    }
    pRow += m_lStride;
  }
  return S_OK;
}