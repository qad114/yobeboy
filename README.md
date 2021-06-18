# yobeboy
A WIP Game Boy emulator for Linux (Windows and macOS are untested), with graphics powered by SDL2.

## Building
Run `make` to build for Linux. Windows and macOS instructions will be added later. (Note: SDL2 must be installed)

## Usage
`./yobeboy <path to ROM>`

## Status
### Blargg CPU instruction tests:
All individual tests pass except those using the SBC instruction (not sure why yet). The combined test doesn't work as STOP isn't implemented.
### Rendering:
All 3 layers (BG, Window, Objects) are implemented. The positioning of the layers is sometimes wrong (e.g. in Super Mario Land, Mario travels on top of the pipe rather than under it). Scanline-based rendering is only implemented for the Background, and is currently buggy (i.e flickering in some games).
### MBCs:
MBC1 is mostly implemented, and MBC3 is partially implemented (enough to run Pokemon Red/Blue).
### Games that are known to be working:
- Dr. Mario (pill doesn't render correctly)
- Kirby's Dream Land
- Metroid 2
- Pokemon Red
- Tetris
- Tetris DX

## Screenshots
Note: These may be outdated.

![Super Mario Land](https://i.imgur.com/5iYuXus.png)
![Tetris](https://i.imgur.com/fOClhNR.png)
![Kirby's Dream Land](https://i.imgur.com/nq2AaKC.png)