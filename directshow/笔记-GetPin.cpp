// This code allows us to find a pin (input or output) on a filter.
IPin *GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
  BOOL bFound = FALSE;
  IEnumPins *pEnum;
  IPin *pPin;
  // Begin by enumerating all the pins on a filter
  HRESULT hr = pFilter->EnumPins(&pEnum);
  if (FAILED(hr))
  {
    return NULL;
  }
  // Now look for a pin that matches the direction characteristic.
  // When we've found it, we'll return with it.
  while(pEnum->Next(1, &pPin, 0) == S_OK)
  {
    PIN_DIRECTION PinDirThis;
    pPin->QueryDirection(&PinDirThis);
    if (bFound = (PinDir == PinDirThis))
    break;
    pPin->Release();
  }
  pEnum->Release();
  return (bFound ? pPin : NULL);
}