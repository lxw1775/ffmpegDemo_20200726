int main(int argc, char* argv[])
{
  ICaptureGraphBuilder2 *pCaptureGraph = NULL; // Capture graph builder
  IGraphBuilder *pGraph = NULL;                // Graph builder object
  IMediaControl *pControl = NULL;              // Media control object
  IFileSinkFilter *pSink = NULL;               // File sink object
  IBaseFilter *pAudioInputFilter = NULL;       // Audio Capture filter
  IBaseFilter *pVideoInputFilter = NULL;       // Video Capture filter
  IBaseFilter *pASFWriter = NULL;              // WM ASF File filter
  
  // Initialize the COM library.
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr))
  {
    // We'll send our error messages to the console.
    printf("ERROR - Could not initialize COM library");
    return hr;
  }
  
  // Create the Capture Graph Builder and query for interfaces.
  hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
                        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
                        (void **)&pCaptureGraph);
  if (FAILED(hr))      // FAILED is a macro that tests the return value
  {
    printf("ERROR - Could not create the Filter Graph Manager.");
    return hr;
  }
  
  // Use a method of the Capture Graph Builder
  // to create an output path for the stream.
  hr = pCaptureGraph->SetOutputFileName(&MEDIASUBTYPE_Asf,
                                        L"C:\\MyWebcam.ASF", &pASFWriter, &pSink);

  // Now configure the ASF Writer.
  // Present the property pages for this filter.
  hr = ShowFilterPropertyPages(pASFWriter);

  // Now get the Filter Graph Manager
  // that was created by the Capture Graph Builder.
  hr = pCaptureGraph->GetFiltergraph(&pGraph);

  // Using QueryInterface on the graph builder,
  // get the Media Control object.
  hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
  if (FAILED(hr))
  {
    printf("ERROR - Could not create the Media Control object.");
    pGraph->Release();        // Clean up after ourselves
    CoUninitialize();         // And uninitalize COM
    return hr;
  }

  // Get an audio capture filter.
  // There are several to choose from,
  // so we need to enumerate them, pick one, and
  // then add the audio capture filter to the filter graph.
  hr = GetAudioInputFilter(&pAudioInputFilter, L"Logitech");
  if (SUCCEEDED(hr)) {
    hr = pGraph->AddFilter(pAudioInputFilter, L"Webcam Audio Capture");
  }

  // Now create the video input filter from the webcam.
  hr = GetVideoInputFilter(&pVideoInputFilter, L"Logitech");
  if (SUCCEEDED(hr)) {
    hr = pGraph->AddFilter(pVideoInputFilter, L"Webcam Video Capture");
  }

  // Use another method of the Capture Graph Builder
  // to provide a render path for video preview.
  IBaseFilter *pIntermediate = NULL;
  hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW,
                                   &MEDIATYPE_Video, pVideoInputFilter, NULL, NULL);

  // Now add the video capture to the output file.
  hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE,
                                   &MEDIATYPE_Video, pVideoInputFilter, NULL, pASFWriter);

  // And do the same for the audio.
  hr = pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE,
                                   &MEDIATYPE_Audio, pAudioInputFilter, NULL, pASFWriter);
  if (SUCCEEDED(hr))
  {
    // Run the graph.
    hr = pControl->Run();
    if (SUCCEEDED(hr))
    {
      // Wait patiently for completion of the recording.
      wprintf(L"Started recording...press Enter to stop recording.\n");

      // Wait for completion.
      char ch;
      ch = getchar(); // We wait for keyboard input
    }

    // And stop the filter graph.
    hr = pControl->Stop();
    wprintf(L"Stopped recording.\n"); // To the console

    // Before we finish, save the filter graph to a file.
    SaveGraphFile(pGraph, L"C:\\MyGraph.GRF");
  }

  // Now release everything and clean up.
  pSink->Release();
  pASFWriter->Release();
  pVideoRenderer->Release();
  pVideoInputFilter->Release();
  pAudioInputFilter->Release();
  pControl->Release();
  pGraph->Release();
  pCaptureGraph->Release();
  CoUninitialize();
  return 0;
}
