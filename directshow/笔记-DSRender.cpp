// DSRender.cpp
// A very simple program to render media files using DirectShow
//
int main(int argc, char* argv[])
{
  IGraphBuilder *pGraph = NULL; // Graph builder interface
  IMediaControl *pControl = NULL; // Media control interface
  IMediaEvent *pEvent = NULL; // Media event interface
  if (!GetMediaFileName()) { // Local function to get a file name
    return(0); // If we didn't get it, exit
  }
  // Initialize the COM library.
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (FAILED(hr))
  {
    // We'll send our error messages to the console.
    printf("ERROR - Could not initialize COM library");
    return hr;
  }
  // Create the Filter Graph Manager and query for interfaces.
  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
  IID_IGraphBuilder, (void **)&pGraph);
  if (FAILED(hr)) // FAILED is a macro that tests the return value
  {
    printf("ERROR - Could not create the Filter Graph Manager.");
    return hr;
  }
  // Use IGraphBuilder::QueryInterface (inherited from IUnknown)
  // to get the IMediaControl interface.
  hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
  if (FAILED(hr))
  {
    printf("ERROR - Could not obtain the Media Control interface.");
    pGraph->Release(); // Clean up after ourselves
    pGraph = NULL;
    CoUninitialize(); // And uninitialize COM
    return hr;
  }
  // And get the Media Event interface, too.
  hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
  if (FAILED(hr))
  {
    printf("ERROR - Could not obtain the Media Event interface.");
    pGraph->Release(); // Clean up after ourselves
    pControl->Release();
    CoUninitialize(); // And uninitialize COM
    return hr;
  }
  // To build the filter graph, only one call is required.
  // We make the RenderFile call to the Filter Graph Manager
  // to which we pass the name of the media file.
  #ifndef UNICODE
  WCHAR wFileName[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, g_PathFileName, -1, wFileName,
  MAX_PATH);
  // This is all that's required to create a filter graph
  // that will render a media 
  hr = pGraph->RenderFile((LPCWSTR)wFileName, NULL);
  #else
  hr = pGraph->RenderFile((LPCWSTR)g_PathFileName, NULL);
  #endif
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
    // And stop the filter graph.
    hr = pControl->Stop();
    // Before we finish, save the filter graph to a file.
    SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
  }
  // Now release everything and clean up.
  pControl->Release();
  pEvent->Release();
  pGraph->Release();
  CoUninitialize();
  return 0;
}