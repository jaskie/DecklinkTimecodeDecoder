# DecklinkTimecodeDecoder
A simple video timecode decoder (overlay) using Blackmagic Design Decklink card(s)

![obraz](https://user-images.githubusercontent.com/1919742/135086939-04092220-a5a7-4095-b6ee-ae530695544b.png)

It can be used on Windows platform only.

Requires:
  - Windows 7, Server 2008R2 or newer,
  - VC2019 runtime
  - .NET Framework 4.5
  - Blackmagic Desktop Video driers 10.6.6 or newer

Supports decoding timecode using Blackmagic card, and outputs the timecode on video overlay using internal keyer of the card.

It renders timecode on frame using Gdiplus.

Can operate as service or as console application. To install as service, run the application from adminstrator command prompt with `--install` parameter.
