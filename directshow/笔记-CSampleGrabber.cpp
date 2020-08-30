HRESULT CSampleGrabber::CheckInputType( const CMediaType * pmt )
{
  CheckPointer(pmt,E_POINTER);
  CAutoLock lock( &m_Lock );
  
  // if the major type is not set, then accept anything
  GUID g = *m_mtAccept.Type( );
  if( g == GUID_NULL )
  {
    return NOERROR;
  }
  
  // if the major type is set, don't accept anything else
  if( g != *pmt->Type( ) )
  {
    return VFW_E_INVALID_MEDIA_TYPE;
  }
  
  // subtypes must match, if set. if not set, accept anything
  g = *m_mtAccept.Subtype( );
  if( g == GUID_NULL )
  {
    return NOERROR;
  }
  if( g != *pmt->Subtype( ) )
  {
    return VFW_E_INVALID_MEDIA_TYPE;
  }
  
  // format types must match, if one is set
  g = *m_mtAccept.FormatType( );
  if( g == GUID_NULL )
  {
    return NOERROR;
  }
  if( g != *pmt->FormatType( ) )
  {
    return VFW_E_INVALID_MEDIA_TYPE;
  }
  
  // at this point, for this sample code, this is good enough,
  // but you may want to make it more strict
  return NOERROR;
}

HRESULT CSampleGrabber::Receive( IMediaSample * pms )
{
  CheckPointer(pms,E_POINTER);
  HRESULT hr;
  AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
  if (pProps->dwStreamId != AM_STREAM_MEDIA)
  {
    if( m_pOutput->IsConnected() )
    return m_pOutput->Deliver(pms);
    else
    return NOERROR;
  }
  
  if (UsingDifferentAllocators())
  {
    // We have to copy the data.
    pms = Copy(pms);
    if (pms == NULL)
    {
      return E_UNEXPECTED;
    }
  }
  
  // have the derived class transform the data
  hr = Transform(pms);
  if (FAILED(hr))
  {
    DbgLog((LOG_TRACE, 1, TEXT("Error from TransInPlace")));
    if (UsingDifferentAllocators())
    {
      pms->Release();
    }
    return hr;
  }
  if (hr == NOERROR)
  {
    hr = m_pOutput->Deliver(pms);
    This document was created by an unregistered ChmMagic, please go to http://www.bisenter.com to register it. Thanks
  }
  
  // release the output buffer. If the connected pin still needs it,
  // it will have addrefed it itself.
  if (UsingDifferentAllocators())
  {
    pms->Release();
  }
  return hr;
}

HRESULT CSampleGrabber::Transform ( IMediaSample * pms )
{
  CheckPointer(pms,E_POINTER);
  CAutoLock lock( &m_Lock );
  if( m_callback )
  {
    REFERENCE_TIME StartTime, StopTime;
    pms->GetTime( &StartTime, &StopTime);
    StartTime += m_pInput->CurrentStartTime( );
    StopTime += m_pInput->CurrentStartTime( );
    BOOL * pTypeChanged =
    &((CSampleGrabberInPin*) m_pInput)->m_bMediaTypeChanged;
    HRESULT hr =
    m_callback( pms, &StartTime, &StopTime, *pTypeChanged );
    *pTypeChanged = FALSE; // now that we notified user, can clear it
    return hr;
  }
  
  return NOERROR;
}

CUnknown * WINAPI CSampleGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
  ASSERT(phr);
  
  // assuming we don't want to modify the data
  CSampleGrabber *pNewObject = new CSampleGrabber(punk, phr, FALSE);
  if(pNewObject == NULL) {
    if (phr)
    *phr = E_OUTOFMEMORY;
  }
  return pNewObject;
}

CSampleGrabber::CSampleGrabber( IUnknown * pOuter, HRESULT * phr, bool ModifiesData )
    : CTransInPlaceFilter( TEXT("SampleGrabber"),
    (IUnknown*) pOuter, CLSID_GrabberSample,
    phr, (BOOL)ModifiesData )
    , m_callback( NULL )
{
  // this is used to override the input pin with our own
  m_pInput = (CTransInPlaceInputPin*) new CSampleGrabberInPin( this, phr );
  if( !m_pInput )
  {
    if (phr)
    *phr = E_OUTOFMEMORY;
  }
  
  // Ensure that the output pin gets created.
  // This is necessary because our
  // SetDeliveryBuffer() method assumes
  // that the input/output pins are created, but
  // the output pin isn't created until GetPin() is called. The
  // CTransInPlaceFilter::GetPin() method will create the output pin,
  // since we have not already created one.
  IPin *pOutput = GetPin(1);
  // The pointer is not AddRef'ed by GetPin(), so don't release it
}
STDMETHODIMP CSampleGrabber::NonDelegatingQueryInterface( REFIID riid, void ** ppv)
{
  CheckPointer(ppv,E_POINTER);
  if(riid == IID_IGrabberSample) {
    return GetInterface((IGrabberSample *) this, ppv);
  }
  else {
    return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
  }
}

STDMETHODIMP CSampleGrabber::SetAcceptedMediaType( const CMediaType * pmt )
{
  CAutoLock lock( &m_Lock );
  if( !pmt )
  {
    m_mtAccept = CMediaType( );
    return NOERROR;
  }
  HRESULT hr;
  hr = CopyMediaType( &m_mtAccept, pmt );
  return hr;
}

STDMETHODIMP CSampleGrabber::GetConnectedMediaType( CMediaType * pmt )
{
  if( !m_pInput || !m_pInput->IsConnected( ) )
  {
    return VFW_E_NOT_CONNECTED;
  }
  return m_pInput->ConnectionMediaType( pmt );
}

STDMETHODIMP CSampleGrabber::SetCallback( SAMPLECALLBACK Callback )
{
  CAutoLock lock( &m_Lock );
  m_callback = Callback;
  return NOERROR;
}
STDMETHODIMP CSampleGrabber::SetDeliveryBuffer( ALLOCATOR_PROPERTIES props,
BYTE * m_pBuffer )
{
  // have the input/output pins been created?
  if( !InputPin( ) || !OutputPin( ) )
  {
    return E_POINTER;
  }
  
  // they can't be connected
  // if we're going to be changing delivery buffers
  if( InputPin( )->IsConnected( ) || OutputPin( )->IsConnected( ) )
  {
    return E_INVALIDARG;
  }
  
  return ((CSampleGrabberInPin*)m_pInput)->SetDeliveryBuffer( props,
  m_pBuffer );
}