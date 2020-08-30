// Show the property pages for a filter.
// This is stolen from the DX9 SDK.
HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter) {
  /* Obtain the filter's IBaseFilter interface. (Not shown) */
  ISpecifyPropertyPages *pProp;
  HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages,
                                       (void **)&pProp);
                                       
  if (SUCCEEDED(hr))
  {
    // Get the filter's name and IUnknown pointer.
    FILTER_INFO FilterInfo;
    hr = pFilter->QueryFilterInfo(&FilterInfo);
    IUnknown *pFilterUnk;
    pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

    // Show the page.
    CAUUID caGUID;
    pProp->GetPages(&caGUID);
    pProp->Release();
    OleCreatePropertyFrame(
		NULL,                         // Parent window
		0, 0,                         // Reserved
		FilterInfo.achName,           // Caption for the dialog box
		1,                            // # of objects (just the filter)
		&pFilterUnk,                  // Array of object pointers.
		caGUID.cElems,                // Number of property pages
		caGUID.pElems,                // Array of property page CLSIDs
		0,                            // Locale identifier
		0, NULL                       // Reserved
    );

    // Clean up.
    pFilterUnk->Release();
    FilterInfo.pGraph->Release();
    CoTaskMemFree(caGUID.pElems);
  }
  return hr;
}