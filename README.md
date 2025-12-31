# OpenD-Star

Open Source D-Star AMBE Vocoder Library

## Overview

OpenD-Star is a lightweight C/C++ library for decoding D-Star AMBE (3600x2400) voice frames to PCM audio. It provides a simple API for integrating D-Star voice decoding into amateur radio applications.

## Features

- **D-Star AMBE Decoding**: Decode D-Star AMBE 3600x2400 frames to 16-bit PCM
- **Simple API**: Clean C API with opaque decoder state
- **Static and Shared Libraries**: Build as `.a` or `.so`/`.dylib`
- **Command-line Tool**: Included `dstar_codec` utility for testing
- **Cross-platform**: Tested on Linux and macOS

## Building

```bash
make
```

This produces:
- `libopendstar.a` - Static library
- `libopendstar.dylib` (macOS) or `libopendstar.so` (Linux) - Shared library
- `dstar_codec` - Command-line tool

## Installation

```bash
sudo make install
```

Default installation prefix is `/usr/local`. Override with:
```bash
sudo make install PREFIX=/opt/opendstar
```

## Usage

### API Example

```c
#include "opendstar.h"

// Create decoder
opendstar_decoder_t *dec = opendstar_decoder_create();

// Decode frame (9 bytes AMBE -> 160 samples PCM)
uint8_t ambe[9];    // Input: D-Star AMBE frame
int16_t pcm[160];   // Output: 16-bit PCM samples
int errors;

opendstar_decode(dec, ambe, pcm, &errors);

// Clean up
opendstar_decoder_destroy(dec);
```

### Command-line Tool

```bash
# Decode AMBE file to raw PCM
./dstar_codec decode input.ambe output.raw

# Show library info
./dstar_codec info

# Convert raw PCM to WAV (using sox)
sox -t raw -r 8000 -e signed -b 16 -c 1 output.raw output.wav
```

## Technical Details

### D-Star AMBE 3600x2400 Format

| Parameter | Value |
|-----------|-------|
| Total bit rate | 3600 bps |
| Voice data rate | 2400 bps |
| FEC overhead | 1200 bps |
| Frame size | 72 bits (9 bytes) |
| Frame duration | 20 ms |
| Frame rate | 50 fps |

### Audio Output

| Parameter | Value |
|-----------|-------|
| Sample rate | 8000 Hz |
| Bit depth | 16-bit signed |
| Channels | Mono |
| Samples per frame | 160 |

## Project Structure

```
OpenD-Star/
├── opendstar.h        # Public API header
├── opendstar.cpp      # Library implementation
├── dstar_codec.cpp    # Command-line tool
├── Makefile           # Build system
├── decoder/           # AMBE decoder (from mbelib-neo)
│   ├── mbelib.c/h     # Core MBE vocoder
│   ├── ambe3600x2400.c # D-Star AMBE demodulation
│   ├── mbe_adaptive.c # Adaptive spectral enhancement
│   ├── pffft.c/h      # FFT implementation
│   └── ...
├── LICENSE            # GPL v2
└── README.md
```

## Performance

Typical performance on modern hardware:
- **Decode**: ~0.5ms per frame (3000+ real-time)
- **Memory**: ~50KB per decoder instance

## Upstream Sources

This library integrates the vocoder implementation from mbelib-neo, a
long-standing open-source project. See `decoder/CREDITS` for full attribution.

## License

This project is licensed under the GNU General Public License v2.0 (GPL-2.0).
