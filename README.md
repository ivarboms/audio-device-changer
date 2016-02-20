Audio Device Changer
====================
Simple tool to make changing default audio device easier.
This is typically done by opening the Sound settings, selecting the desired playback device and pressing the Set Default button.


```
>adc
Usage: adc [-l] [-d] [AudioDevice]

Options:
    -l           List available audio devices.
    -d           Show the current default audio device.
    AudioDevice  Set the specified audio device as the new primary audio output device.

>adc -l
Speakers (Sennheiser 3D G4ME1)
Digital Audio (S/PDIF) (High Definition Audio Device)

>adc -d
Speakers (Sennheiser 3D G4ME1)
>adc "Digital Audio (S/PDIF) (High Definition Audio Device)"
Setting 'Digital Audio (S/PDIF) (High Definition Audio Device)' as active audio device.

>adc "Speakers (Sennheiser 3D G4ME1)"
Setting 'Speakers (Sennheiser 3D G4ME1)' as active audio device.
```
