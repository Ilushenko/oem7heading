# OEM7 Heading Library

Library for obtaining heading and positionfrom NovAtel OEM7 GNSS dual-antenna receiver by ESP32, Arduino or PC via serial port

See: [OEM7 Receiver User Documentation](https://docs.novatel.com/OEM7/Content/Home.htm)

Tested on **OEM718D** connected via **UART** to **ESP WROOM-32U**

Tested on **OEM718D** connected via **Serial** to **Win32**

### Compute:

- UTC Time
- Best Position
- Heading information with multiple rovers

### Check:

- Receiver Status
- Antennas Status
- RTK State
- Jamming & Spoofing detection

## Pinout

| ESP32 PIN | OEM718D PIN |
| --------- | ----------- |
| 3V3       | 3V3         |
| GND       | GND         |
| 18        | 11 TXD1     |
| 5         | 12 RXD1     |

![Scheme](./scheme.png)

## Debug Logs

To output debug logs by **Arduino** or **ESP32**, declare macro **DEBUGLOG** in **platforio.ini** and rebuild sketch

```
[env:esp32dev]
...
build_flags = -Wall -D DEBUGLOG
...
```

To output debug logs by **Win32** or **POSIX**, declare macro **DEBUGLOG** in compiller options or MS VS projects properties

## PC dependency

For **Win32** or **POSIX** platform need use [Serialib](https://github.com/imabot2/serialib/tree/master)
