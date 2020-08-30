class CSampleGrabberAllocator : public CMemAllocator
{
friend class CSampleGrabberInPin;
friend class CSampleGrabber;
protected:
// our pin who created us
CSampleGrabberInPin * m_pPin;
public:
CSampleGrabberAllocator( CSampleGrabberInPin * pParent, HRESULT *phr )
: CMemAllocator( TEXT("SampleGrabberAllocator\0"), NULL, phr )
, m_pPin( pParent )
{
};
~CSampleGrabberAllocator( )
{
// wipe out m_pBuffer before we try to delete it.
// It's not an allocated buffer,
// and the default destructor will try to free it!
m_pBuffer = NULL;
}
HRESULT Alloc( );
void ReallyFree();
// Override to reject anything that does not match the actual buffer
// that was created by the application
STDMETHODIMP SetProperties(ALLOCATOR_PROPERTIES *pRequest,
ALLOCATOR_PROPERTIES *pActual);
};


HRESULT CSampleGrabberAllocator::Alloc( )
{
  // look at the base class code to see where this came 
  CAutoLock lck(this);
  
  // Check he has called SetProperties
  HRESULT hr = CBaseAllocator::Alloc();
  if (FAILED(hr)) {
    return hr;
  }
  
  // If the requirements haven't changed then don't reallocate
  if (hr == S_FALSE) {
    ASSERT(m_pBuffer);
    return NOERROR;
  }
  ASSERT(hr == S_OK); // we use this fact in the loop below
  
  // Free the old resources
  if (m_pBuffer) {
    ReallyFree();
  }
 
  // Compute the aligned size
  LONG lAlignedSize = m_lSize + m_lPrefix;
  if (m_lAlignment > 1)
  {
    LONG lRemainder = lAlignedSize % m_lAlignment;
    if (lRemainder != 0)
    {
      lAlignedSize += (m_lAlignment - lRemainder);
    }
  }
  ASSERT(lAlignedSize % m_lAlignment == 0);
  
  // don't create the buffer - use what was passed to us
  //
  m_pBuffer = m_pPin->m_pBuffer;
  if (m_pBuffer == NULL) {
    return E_OUTOFMEMORY;
  }
  LPBYTE pNext = m_pBuffer;
  CMediaSample *pSample;
  ASSERT(m_lAllocated == 0);
  
  // Create the new samples -
  // we have allocated m_lSize bytes for each sample
  // plus m_lPrefix bytes per sample as a prefix. We set the pointer to
  // the memory after the prefix -
  // so that GetPointer() will return a pointer to m_lSize bytes.
  for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize)
  {
    pSample = new CMediaSample(
    NAME("Sample Grabber memory media sample"),
    This document was created by an unregistered ChmMagic, please go to http://www.bisenter.com to register it. Thanks
    this,
    &hr,
    pNext + m_lPrefix, // GetPointer() value
    m_lSize); // not including prefix
    ASSERT(SUCCEEDED(hr));
    if (pSample == NULL)
    return E_OUTOFMEMORY;
    
    // This CANNOT fail
    m_lFree.Add(pSample);
  }
  m_bChanged = FALSE;
  return NOERROR;
}

void CSampleGrabberAllocator::ReallyFree()
{
  // look at the base class code to see where this came 
  // Should never be deleting this unless all buffers are freed
  ASSERT(m_lAllocated == m_lFree.GetCount());
  
  // Free up all the CMediaSamples
  CMediaSample *pSample;
  for (;;)
  {
    pSample = m_lFree.RemoveHead();
    if (pSample != NULL)
    {
      delete pSample;
    }
    else
    This document was created by an unregistered ChmMagic, please go to http://www.bisenter.com to register it. Thanks
    {
      break;
    }
  }
  m_lAllocated = 0;
  // don't free the buffer - let the app do it
}

HRESULT CSampleGrabberAllocator::SetProperties(
ALLOCATOR_PROPERTIES *pRequest,
ALLOCATOR_PROPERTIES *pActual
)
{
  HRESULT hr = CMemAllocator::SetProperties(pRequest, pActual);
  if (FAILED(hr))
  {
    return hr;
  }
  ALLOCATOR_PROPERTIES *pRequired = &(m_pPin->m_allocprops);
  
  if (pRequest->cbAlign != pRequired->cbAlign)
  {
    return VFW_E_BADALIGN;
  }
  
  if (pRequest->cbPrefix != pRequired->cbPrefix)
  {
    return E_FAIL;
  }
  
  if (pRequest->cbBuffer > pRequired->cbBuffer)
  {
    return E_FAIL;
  }
  
  if (pRequest->cBuffers > pRequired->cBuffers)
  {
    return E_FAIL;
  }
  *pActual = *pRequired;
  m_lCount = pRequired->cBuffers;
  m_lSize = pRequired->cbBuffer;
  m_lAlignment = pRequired->cbAlign;
  m_lPrefix = pRequired->cbPrefix;
  return S_OK;
}