HRESULT RunFile(LPTSTR pszSrcFile, LPTSTR pszDestFile)
{
  HRESULT hr;
  USES_CONVERSION; // For TCHAR -> WCHAR conversion macros
  CComPtr<IGraphBuilder> pGraph; // Filter Graph Manager
  CComPtr<IBaseFilter> pGrabF; // Sample grabber
  CComPtr<IBaseFilter> pMux; // AVI Mux
  CComPtr<IBaseFilter> pSrc; // Source filter
  CComPtr<IBaseFilter> pDVEnc; // DV Encoder
  CComPtr<ICaptureGraphBuilder2> pBuild;
  
  // Create the Filter Graph Manager.
  hr = pGraph.CoCreateInstance(CLSID_FilterGraph);
  if (FAILED(hr))
  {
    printf("Could not create the Filter Graph Manager (hr = 0x%X.)\n",
    hr);
    return hr;
  }
  
  // Create the Capture Graph Builder.
  hr = pBuild.CoCreateInstance(CLSID_CaptureGraphBuilder2);
  if (FAILED(hr))
  {
    printf("Could not create the Capture Graph Builder (hr = 0x%X.)\n",
    hr);
    return hr;
  }
  pBuild->SetFiltergraph(pGraph);
  
  // Build the file-writing portion of the graph.
  hr = pBuild->SetOutputFileName(&MEDIASUBTYPE_Avi, T2W(pszDestFile),

  if (FAILED(hr))
  {
    printf("Could not hook up the AVI Mux / File Writer (hr = 0x%X.)\n",
    hr);
    return hr;
  }
  
  // Add the source filter for the input file.
  hr = pGraph->AddSourceFilter(T2W(pszSrcFile), L"Source", &pSrc);
  if (FAILED(hr))
  {
    printf("Could not add the source filter (hr = 0x%X.)\n", hr);
    return hr;
  }
  
  // Create some filters and add them to the graph.
  // DV Video Encoder
  hr = AddFilterByCLSID(pGraph, CLSID_DVVideoEnc, L"DV Encoder", &pDVEnc);
  if (FAILED(hr))
  {
    printf("Could not add the DV video encoder filter (hr = 0x%X.)\n", hr);
    return hr;
  }
  
  CComQIPtr<ISampleGrabber> pGrabber(pGrabF);
	if (!pGrabF)
	{
	  return E_NOINTERFACE;
	}

	// Configure the Sample Grabber.
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_UYVY;
	mt.formattype = FORMAT_VideoInfo;

	// Note: I don't expect the next few methods to fail...
	hr = pGrabber->SetMediaType(&mt); // Set the media type
	_ASSERTE(SUCCEEDED(hr));
	hr = pGrabber->SetOneShot(FALSE); // Disable "one-shot" mode
	_ASSERTE(SUCCEEDED(hr));

	hr = pGrabber->SetBufferSamples(FALSE); // Disable sample buffering
	_ASSERTE(SUCCEEDED(hr));
	hr = pGrabber->SetCallback(&grabberCallback, 0); // Set our callback
	// '0' means 'use the SampleCB callback'
	_ASSERTE(SUCCEEDED(hr));
	
	// Build the graph.
	// First connect the source to the DV Encoder,
	// through the Sample Grabber.
	// This should connect the video stream.
	hr = pBuild->RenderStream(0, 0, pSrc, pGrabF, pDVEnc);
	if (SUCCEEDED(hr))
	{
	  // Next, connect the DV Encoder to the AVI Mux.
	  hr = pBuild->RenderStream(0, 0, pDVEnc, 0, pMux);
	  if (SUCCEEDED(hr))
	  {
		// Maybe we have an audio stream.
		// If so, connect it the AVI Mux.
		// But don't fail if we don't...
		HRESULT hrTmp = pBuild->RenderStream(0, 0, pSrc, 0, pMux);
		SaveGraphFile(pGraph, L"C:\\Grabber.grf");
	  }
	}

	if (FAILED(hr))
	{
	  printf("Error building the graph (hr = 0x%X.)\n", hr);
	  return hr;
	}

	// Find out the exact video format.
	hr = pGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr))
	{
	  printf("Could not get the video format. (hr = 0x%X.)\n", hr);
	  return hr;
	}

	VIDEOINFOHEADER *pVih;
	if ((mt.subtype == MEDIASUBTYPE_UYVY) && (mt.formattype == FORMAT_VideoInfo))
	{
	  pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	}
	else
	{
	  // This is not the format we 
	  CoTaskMemFree(mt.pbFormat);
	  return VFW_E_INVALIDMEDIATYPE;
	}
	
	g_stretch.SetFormat(*pVih);
	CoTaskMemFree(mt.pbFormat);

	// Turn off the graph clock.
	CComQIPtr<IMediaFilter> pMF(pGraph);
	pMF->SetSyncSource(NULL);

	// Run the graph to completion.
	CComQIPtr<IMediaControl> pControl(pGraph);

	CComQIPtr<IMediaEvent> pEvent(pGraph);
	long evCode = 0;
	printf("Processing the video file... ");
	pControl->Run();
	pEvent->WaitForCompletion(INFINITE, &evCode);
	pControl->Stop();
	printf("Done.\n");
	return hr;
}