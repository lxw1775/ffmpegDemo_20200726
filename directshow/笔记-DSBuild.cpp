// DSBuild implements a very simple program to render audio files
// or the audio portion of movies.
//
int main(int argc, char* argv[])
{
  IGraphBuilder *pGraph = NULL;
  IMediaControl *pControl = NULL;
  IMediaEvent *pEvent = NULL;
  IBaseFilter *pInputFileFilter = NULL;
  IBaseFilter *pDSoundRenderer = NULL;
  IPin *pFileOut = NULL, *pWAVIn = NULL;
  // Get the name of an audio or movie file to play.
  if (!GetMediaFileName()) {
    return(0);
  }
  // Initialize the COM library.
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (FAILED(hr))
  {
    printf("ERROR - Could not initialize COM library");
    return hr;
  }
  // Create the Filter Graph Manager object and retrieve its
  // IGraphBuilder interface.
  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void **)&pGraph);
  if (FAILED(hr))
  {
    printf("ERROR - Could not create the Filter Graph Manager.");
    CoUninitialize();
    return hr;
  }
  // Now get the media control interface...
  hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
  if (FAILED(hr)) {
    pGraph->Release();
    CoUninitialize();
    return hr;
  }
  // And the media event interface.
  hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
  if (FAILED(hr)) {
    pControl->Release();
    pGraph->Release();
    CoUninitialize();
    return hr;
  }
  // Build the graph.
  // Step one is to invoke AddSourceFilter
  // with the file name we picked out earlier.
  // Should be an audio file (or a movie file with an audio track).
  // AddSourceFilter instantiates the source filter,
  // adds it to the graph, and returns a pointer to the filter's
  // IBaseFilter interface.
  #ifndef UNICODE
  WCHAR wFileName[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName, MAX_PATH);
  hr = pGraph->AddSourceFilter(wFileName, wFileName, &pInputFileFilter);
  #else
  hr = pGraph->AddSourceFilter(wFileName, wFileName, &pInputFileFilter);
  #endif
  if (SUCCEEDED(hr)) {
    // Now create an instance of the audio renderer
    // and obtain a pointer to its IBaseFilter interface.
    hr = CoCreateInstance(CLSID_DSoundRender, NULL,
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&pDSoundRenderer);
    if (SUCCEEDED(hr)) {
      // And add the filter to the filter graph
      // using the member function AddFilter.
      hr = pGraph->AddFilter(pDSoundRenderer, L"Audio Renderer");
      if (SUCCEEDED(hr)) {
        // Now we need to connect the output pin of the source
        // to the input pin of the renderer.
        // Obtain the output pin of the source filter.
        // The local function GetPin does this.
        pFileOut = GetPin(pInputFileFilter, PINDIR_OUTPUT);
        if (pFileOut != NULL) { // Is the pin good?
          // Obtain the input pin of the WAV renderer.
          pWAVIn = GetPin(pDSoundRenderer, PINDIR_INPUT);
          if (pWAVIn != NULL) { // Is the pin good?
            // Connect the pins together:
            // We use the Filter Graph Manager's
            // member function Connect,
            // which uses Intelligent Connect.
            // If this fails, DirectShow couldn't
            // render the media file.
            hr = pGraph->Connect(pFileOut, pWAVIn);
          }
        }
      }
    }
  }
  if (SUCCEEDED(hr))
  {
    // Run the graph.
    hr = pControl->Run();
    if (SUCCEEDED(hr))
    {
      // Wait for completion.
      long evCode;
      pEvent->WaitForCompletion(INFINITE, &evCode);
      // Note: Do not use INFINITE in a real application
      // because it can block indefinitely.
    }
    hr = pControl->Stop();
  }
  // Before we finish, save the filter graph to a file.
  SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
  // Now release everything we instantiated--
  // that is, if it got instantiated.
  if(pFileOut) { // If it exists, non-NULL
    pFileOut->Release(); // Then release it
  }
  if (pWAVIn) {
    pWAVIn->Release();
  }
  if (pInputFileFilter) {
    pInputFileFilter->Release();
  }
  if (pDSoundRenderer) {
    pDSoundRenderer->Release();
  }
  pControl->Release();
  pEvent->Release();
  pGraph->Release();
  CoUninitialize();
  return 0;
}