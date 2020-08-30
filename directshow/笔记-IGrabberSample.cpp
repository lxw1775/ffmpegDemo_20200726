//-------------------------------------------------------------------------
// Define new GUID and IID for the grabber example so that they do NOT
// conflict with the official DirectX Grabber Sample filter
//-------------------------------------------------------------------------
// {2FA4F053-6D60-4cb0-9503-8E89234F3F73}
DEFINE_GUID(CLSID_GrabberSample,
0x2fa4f053, 0x6d60, 0x4cb0, 0x95, 0x3, 0x8e, 0x89, 0x23, 0x4f, 0x3f, 0x73);
DEFINE_GUID(IID_IGrabberSample,
0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);

// We define a callback typedef for this example.
// Normally, you would make the Grabber Sample support a COM interface,
// and in one of its methods you would pass in a pointer to a COM interface
// used for calling back.
typedef HRESULT (*SAMPLECALLBACK) (
IMediaSample * pSample,
REFERENCE_TIME * StartTime,
REFERENCE_TIME * StopTime,
BOOL TypeChanged );
// We define the interface the app can use to program us
MIDL_INTERFACE("6B652FFF-11FE-4FCE-92AD-0266B5D7C78F")

IGrabberSample : public IUnknown
{
  public:
  virtual HRESULT STDMETHODCALLTYPE SetAcceptedMediaType(
  const CMediaType *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
  CMediaType *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetCallback(
  SAMPLECALLBACK Callback) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDeliveryBuffer(
  ALLOCATOR_PROPERTIES props,
  BYTE *pBuffer) = 0;
};
Defining and Implementing the Filter Class

The definition of the filter class CSampleGrabber is closely based on the definition of the IGrabberSample interface. CSampleGrabber includes declarations for all the methods declared within IGrabberSample, but in this case, it actually includes the implementations of these methods. Here’s the definition of the CSampleGrabber class:


点击(此处)折叠或打开

class CSampleGrabber : public CTransInPlaceFilter,
public IGrabberSample
{
  friend class CSampleGrabberInPin;
  friend class CSampleGrabberAllocator;
  protected:
  CMediaType m_mtAccept;
  SAMPLECALLBACK m_callback;
  CCritSec m_Lock; // serialize access to our data
  BOOL IsReadOnly( ) { return !m_bModifiesData; }
  
  // PURE, override this to ensure we get
  // connected with the right media type
  HRESULT CheckInputType( const CMediaType * pmt );
  
  // PURE, override this to callback
  // the user when a sample is received
  HRESULT Transform( IMediaSample * pms );
  
  // override this so we can return S_FALSE directly.
  // The base class CTransInPlace
  // Transform( ) method is called by its
  // Receive( ) method. There is no way
  // to get Transform( ) to return an S_FALSE value
  // (which means "stop giving me data"),
  // to Receive( ) and get Receive( ) to return S_FALSE as well.
  HRESULT Receive( IMediaSample * pms );
  public:
  static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
  
  // Expose IGrabberSample
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  DECLARE_IUNKNOWN;
  CSampleGrabber( IUnknown * pOuter, HRESULT * pHr, BOOL ModifiesData );
  
  // IGrabberSample
  STDMETHODIMP SetAcceptedMediaType( const CMediaType * pmt );
  STDMETHODIMP GetConnectedMediaType( CMediaType * pmt );
  STDMETHODIMP SetCallback( SAMPLECALLBACK Callback );
  STDMETHODIMP SetDeliveryBuffer( ALLOCATOR_PROPERTIES props,
  BYTE * m_pBuffer );
};