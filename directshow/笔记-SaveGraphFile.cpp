// Pass it a file name in wszPath, and it will save the filter graph
// to that file.
HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath)
{
  const WCHAR wszStreamName[] = L"ActiveMovieGraph";
  HRESULT hr;
  IStorage *pStorage = NULL;
  // First, create a document file that will hold the GRF file
  hr = StgCreateDocfile(
  wszPath,
  STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE |
  STGM_SHARE_EXCLUSIVE,
  0, &pStorage);
  if(FAILED(hr))
  {
    return hr;
  }
  // Next, create a stream to store.
  IStream *pStream;
  hr = pStorage->CreateStream(
  wszStreamName,
  STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
  0, 0, &pStream);
  if (FAILED(hr))
  {
    pStorage->Release();
    return hr;
  }
  // The IpersistStream::Save method converts a stream
  // into a persistent object.
  IPersistStream *pPersist = NULL;
  pGraph->QueryInterface(IID_IPersistStream,
  reinterpret_cast<void**>(&pPersist));
  hr = pPersist->Save(pStream, TRUE);
  pStream->Release();
  pPersist->Release();
  if (SUCCEEDED(hr))
  {
    hr = pStorage->Commit(STGC_DEFAULT);
  }
  pStorage->Release();
  return hr;
}