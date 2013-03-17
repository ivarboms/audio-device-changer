#include "stdafx.h"

#include "PolicyConfig.h"


//These two functions are mostly taken from 
//http://www.daveamenta.com/2011-05/programmatically-or-command-line-change-the-default-sound-playback-device-in-windows-7/

HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{
  IPolicyConfigVista *pPolicyConfig;
  ERole reserved = eConsole;
  
  HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
    NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
  if (SUCCEEDED(hr))
  {
    hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
    pPolicyConfig->Release();
  }
  return hr;
}

std::unordered_map<std::wstring, std::wstring> getAudioOutputDevices()
{
  //friendly name : device ID
  std::unordered_map<std::wstring, std::wstring> devices;

  HRESULT hr = CoInitialize(NULL);
  if (SUCCEEDED(hr))
  {
    IMMDeviceEnumerator *pEnum = NULL;
    // Create a multimedia device enumerator.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
      CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (SUCCEEDED(hr))
    {
      IMMDeviceCollection *pDevices;
      // Enumerate the output devices.
      hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
      if (SUCCEEDED(hr))
      {
        UINT count;
        pDevices->GetCount(&count);
        if (SUCCEEDED(hr))
        {
          for (unsigned int i = 0; i < count; i++)
          {
            IMMDevice *pDevice;
            hr = pDevices->Item(i, &pDevice);
            if (SUCCEEDED(hr))
            {
              LPWSTR wstrID = NULL;
              hr = pDevice->GetId(&wstrID);
              if (SUCCEEDED(hr))
              {
                IPropertyStore *pStore;
                hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
                if (SUCCEEDED(hr))
                {
                  PROPVARIANT friendlyName;
                  PropVariantInit(&friendlyName);
                  hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
                  if (SUCCEEDED(hr))
                  {
                    devices[friendlyName.pwszVal] = wstrID;

                    PropVariantClear(&friendlyName);
                  }
                  pStore->Release();
                }
              }
              pDevice->Release();
            }
          }
        }
        pDevices->Release();
      }
      pEnum->Release();
    }
  }

  return devices;
}

int wmain(int argc, wchar_t* argv[])
{
  if (argc != 2)
  {
    printf("Need exactly 1 argument.\n");
    return 1;
  }

  //device friendly name : device ID
  std::unordered_map<std::wstring, std::wstring> devices(getAudioOutputDevices());

  if (!devices.empty())
  {
    const std::wstring vengeance2000Name(L"Headphones (Vengeance 2000)");
    const std::wstring motherboardName(L"Speakers (High Definition Audio Device)");

    for (const auto& device : devices)
    {
      if ((device.first == vengeance2000Name && wcsstr(argv[1], L"v") != nullptr)
        || (device.first == motherboardName && wcsstr(argv[1], L"x") != nullptr))
      {
        SetDefaultAudioPlaybackDevice(device.second.c_str());
        wprintf(L"Audio device set to '%s'\n", device.first.c_str());

        return 0;
      }
    }
  }

  wprintf(L"Could not set audio device to '%s'\n", argv[1]);

  return 0;
}