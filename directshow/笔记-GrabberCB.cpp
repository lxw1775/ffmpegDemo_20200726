class GrabberCB : public ISampleGrabberCB
{
  private:
  BITMAPINFOHEADER m_bmi; // Holds the bitmap format
  bool m_fFirstSample; // True if the next sample is the first one
  public:
  GrabberCB();
  ~GrabberCB();
  
  // IUnknown methods
  STDMETHODIMP_(ULONG) AddRef() { return 1; }
  STDMETHODIMP_(ULONG) Release() { return 2; }
  STDMETHOD(QueryInterface)(REFIID iid, void** ppv);
  // ISampleGrabberCB methods
  STDMETHOD(SampleCB)(double SampleTime, IMediaSample *pSample);
  STDMETHODIMP BufferCB(double, BYTE *, long) { return E_NOTIMPL; }
};

GrabberCB::GrabberCB() : m_fFirstSample(true)
{
}


GrabberCB::~GrabberCB()
{
}

// Support querying for ISampleGrabberCB interface
HRESULT GrabberCB::QueryInterface(REFIID iid, void**ppv)
{
  if (!ppv) { return E_POINTER; }
  if (iid == IID_IUnknown)
  {
    *ppv = static_cast<IUnknown*>(this);
  }
  else if (iid == IID_ISampleGrabberCB)
  {
    *ppv = static_cast<ISampleGrabberCB*>(this);
  }
  else
  {
    return E_NOINTERFACE;
  }
  AddRef(); // We don't actually ref count,
  // but in case we change the implementation later.
  return S_OK;
}

// SampleCB: This is where we process each sample.
HRESULT GrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
  HRESULT hr;

  // Get the pointer to the buffer.
  BYTE *pBuffer;
  hr = pSample->GetPointer(&pBuffer);
  
  // Tell the image processing class about it.
  g_stretch.SetImage(pBuffer);
  if (FAILED(hr))
  {
    OutputDebugString(TEXT("SampleCB: GetPointer FAILED\n"));
    return hr;
  }
  
  // Scan the image on the first sample.
  // Re-scan if there is a discontinuity.
  // (This will produce horrible results
  // if there are big scene changes in the
  // video that are not associated with discontinuities.
  // Might be safer to re-scan
  // each image, at a higher perf cost.)
  if (m_fFirstSample)
  {
    hr = g_stretch.ScanImage();
    m_fFirstSample = false;
  }
  else if (S_OK == pSample->IsDiscontinuity())
  {
    hr = g_stretch.ScanImage();
  }
  
  // Convert the image.
  hr = g_stretch.ConvertImage();
  return S_OK;
}