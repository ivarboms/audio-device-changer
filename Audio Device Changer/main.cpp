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

std::wstring getDeviceFriendlyName(IMMDevice *pDevice)
{
	LPWSTR wstrID = NULL;
	std::wstring friendlyName;
	HRESULT hr = pDevice->GetId(&wstrID);
	if (SUCCEEDED(hr))
	{
		IPropertyStore *pStore;
		hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
		if (SUCCEEDED(hr))
		{
			PROPVARIANT friendlyNameProp;
			PropVariantInit(&friendlyNameProp);
			hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyNameProp);
			if (SUCCEEDED(hr))
			{
				friendlyName = friendlyNameProp.pwszVal;

				PropVariantClear(&friendlyNameProp);
			}
			pStore->Release();
		}
	}
	pDevice->Release();

	return friendlyName;
}

std::wstring getDefaultAudioDevice()
{
	std::wstring deviceName;
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		IMMDeviceEnumerator *pEnum = NULL;
		// Create a multimedia device enumerator.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
		if (SUCCEEDED(hr))
		{
			IMMDevice* pDevice;
			hr = pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
			if (SUCCEEDED(hr))
			{
				deviceName = getDeviceFriendlyName(pDevice);
			}
		}
		pEnum->Release();
	}

	return deviceName;
}

//Returns available audio devices in a map<friendly name, device ID>.
//Friendly name is the format that is shown in the Windows Sound dialog.
std::unordered_map<std::wstring, std::wstring> getAudioOutputDevices()
{
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

void printAudioDevices(const std::unordered_map<std::wstring, std::wstring>& audioDevices)
{
	for (const auto& audioDevice : audioDevices)
	{
		wprintf(L"%s\n", audioDevice.first.c_str());
	}
}

bool contains(const std::vector<std::wstring>& container, const std::wstring& item)
{
	return std::find(std::begin(container), std::end(container), item)
		!= std::end(container);
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc == 1)
	{
		printf("Usage: adc [-l] [-d] [AudioDevice]\n\n");
		printf("Options:\n");
		printf("    -l           List available audio devices.\n");
		printf("    -d           Show the current default audio device.\n");
		printf("    AudioDevice  Set the specified audio device as the new primary audio output device.\n");
		return 1;
	}

	std::vector<std::wstring> args(argv, argv + argc);

	const bool listDevices = contains(args, L"-l");
	const bool showDefault  = contains(args, L"-d");

	const auto devices = getAudioOutputDevices();

	if (listDevices)
	{
		printAudioDevices(devices);
	}
	else if (showDefault)
	{
		const std::wstring defaultDevice(getDefaultAudioDevice());
		wprintf(L"%s", defaultDevice.c_str());
	}
	else
	{
		const auto& device = devices.find(args[1]);
		if (device != std::end(devices))
		{
				wprintf(L"Setting '%s' as active audio device.\n", argv[1]);
				SetDefaultAudioPlaybackDevice(device->second.c_str());
				return 0;
		}
		wprintf(L"Failed to find audio device '%s'\n", argv[1]);
		return 1;
	}

	return 0;
}