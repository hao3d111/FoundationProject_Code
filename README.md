# Huffman File Compressor

A lossless file compression and decompression tool built on **Huffman coding**, written in C++17 for Windows. It offers two ways to use it: a **graphical user interface (GUI)** and a **command-line interface (CLI)**.

> **Team:** Nam Luu, Huu Nguyen, Duong Hoang · **Version:** 1.0

---

## Features

- **Compress and decompress any file type** (text, images, executables, etc.), since the algorithm works at the byte level. Compressed files use the `.huff` extension.
- **Native Win32 GUI** with three tabs:
  - **Compress:** pick a source file and a `.huff` destination, then watch the progress bar and metrics (original size, compressed size, space saved %, elapsed time).
  - **Decompress:** pick a `.huff` file; a restored file name is suggested automatically (e.g. `image.bmp.huff` → `image_restored.bmp`).
  - **Benchmark:** run compression over a whole directory (default `test_dataset/`) and see per-file results plus an overall summary.
- **Drag and drop** files from Explorer into the window.
- **Background worker thread** (`std::thread`), so the UI stays responsive on large files.
- **High-DPI support** and Common Controls v6 via an application manifest.
- **Menu-driven CLI** for terminal users.

---

## Requirements

- Windows 10 / 11
- MinGW-w64 with `g++` (C++17) and `windres`

The source uses `windows.h`, so it **does not build on Linux or macOS** (the Huffman core and BitIO are standard C++, though).

---

## Building

**Option 1: batch script**

```bat
build.bat
```

**Option 2: Make**

```bash
make          # build
make clean    # remove HuffmanTool.exe and resource.o
```

**Option 3: manual build**

```bash
windres resources/resource.rc -O coff -o resources/resource.o
g++ -O2 -std=c++17 src/main.cpp src/GUI.cpp src/UI.cpp resources/resource.o -o HuffmanTool.exe -mwindows -lcomctl32 -lcomdlg32 -lshell32 -lole32
```

---

## Usage

| Command | Effect |
|---|---|
| `HuffmanTool.exe` | Launch the GUI (default) |
| `HuffmanTool.exe --cli` (or `-c`) | Launch the CLI |
| `HuffmanTool.exe --help` (or `-h`) | Show help |

The CLI shows this menu:

```
[1] Compress File (.huff)
[2] Decompress File (.huff)
[3] Run Benchmark Test Suite
[0] Exit Application
```

Because the executable is built with `-mwindows`, CLI mode must be launched **from a terminal** (PowerShell or CMD). The program attaches to that terminal's console.

---

## How it works

### Compression

1. Read the file in 64 KB chunks and count the frequency of each byte value (0–255).
2. Build the Huffman tree with a hand-written **min-heap**: repeatedly take the two lowest-frequency nodes and merge them under a parent until one root remains.
3. Walk the tree to generate prefix codes (left branch = `0`, right branch = `1`).
4. Write the header, then the encoded bit stream through `BitWriter`. The final byte is padded with `0` bits up to 8 bits.

### Decompression

1. Read the header, rebuild the frequency table, and rebuild **the same Huffman tree**.
2. Read bit by bit and walk down the tree (`0` goes left, `1` goes right). On reaching a leaf, emit its byte and return to the root.
3. Stop once `total_bytes` bytes have been decoded, so the trailing padding bits are ignored correctly.

### Edge cases

| Case | Handling |
|---|---|
| Empty file | Only the header is written (`distinct_symbols = 0`, `total_bytes = 0`) |
| File with a single distinct byte | The tree has one leaf whose code is `0` (1 bit per byte); decompression repeats that byte `total_bytes` times |

### `.huff` file format

All integers are stored **little-endian** (native x86 layout at build time).

```
Offset   Size         Content
0        2 bytes      distinct_symbols  (uint16)  number of distinct byte values, N
2        8 bytes      total_bytes       (uint64)  size of the original file
10       9 × N bytes  Frequency table: N entries, each
                        1 byte  byte value (uint8)
                        8 bytes frequency  (uint64)
10+9N    remainder    Huffman-encoded data (bit stream, last byte zero-padded)
```

The header is between 10 and 2314 bytes, depending only on how many distinct byte values occur, not on the file size.

The format does **not store the original file name or extension**. The restored name shown in the GUI is derived from the `.huff` file name.

---

## Sample result

In `test_dataset/`, `codetest.exe` (87,023 bytes) compresses to `codetest.exe.huff` (58,011 bytes), about **66.7%** of the original size (roughly 33.3% saved).

Results depend on the byte distribution: text and skewed data usually compress well, while already-compressed data (ZIP, JPEG, MP4, ...) barely compresses and can even grow slightly because of the header.

---

## Testing

**BitIO unit tests** (bit-level and multi-byte write/read):

```bash
g++ -std=c++17 tests/test_bit_io.cpp -o test_bit_io
./test_bit_io
```

**Sample data** in `test_dataset/`:

| File | Purpose |
|---|---|
| `sample_text_english.txt` | Ordinary text |
| `single_char.txt` | Single distinct byte |
| `edge_case_empty.txt` | Empty file (0 bytes) |
| `image_test.bmp` | Bitmap image (~2.2 MB) |
| `codetest.exe`, `abc.exe` | Binary files |

Run them all from the **Benchmark** tab (or option `[3]` in the CLI). The temporary `.test.huff` files it creates can be removed with the *Clean Temp Files* button.

---

## Project structure

```
HuffmanCompressor/
├── build.bat                 # One-click Windows build script
├── Makefile
├── README.md
├── HuffmanTool.exe           # Prebuilt executable (GUI + CLI)
├── include/
│   ├── BitIO.hpp             # BitWriter / BitReader (bit-level I/O)
│   ├── Benchmark.hpp         # Timer and compression statistics
│   ├── GUI.hpp               # Win32 GUI class declaration
│   ├── HuffmanCore.hpp       # HuffmanNode, MinHeap, frequency counting
│   ├── HuffmanTree.hpp       # Tree construction and code table generation
│   ├── HuffmanEncoder.hpp    # Compression
│   ├── HuffmanDecoder.hpp    # Decompression
│   ├── HeaderWriter.hpp      # (legacy) header writer with a padding byte
│   ├── HeaderReader.hpp      # (legacy) matching header reader
│   ├── HuffmanTestCoder.cpp  # Small test program using the encoder/decoder
│   └── UI.hpp                # CLI declaration
├── src/
│   ├── main.cpp              # Entry point: GUI by default, CLI via --cli
│   ├── GUI.cpp               # GUI implementation and worker threads
│   └── UI.cpp                # CLI implementation
├── resources/
│   ├── manifest.xml          # Common Controls v6 + DPI awareness
│   ├── resource.h
│   └── resource.rc           # Manifest and version info
├── tests/
│   └── test_bit_io.cpp
└── test_dataset/             # Sample data
```

> `HeaderWriter.hpp` and `HeaderReader.hpp` use a **different** header layout (with a `padding` byte) and are not used by `HuffmanEncoder` / `HuffmanDecoder`. The format actually in use is the one described above.

---

## Known limitations

- There is no checksum or magic number, so feeding a non-`.huff` file to the decompressor may produce garbage or an error with no early detection.
- The header stores an 8-byte frequency per distinct byte value, which is **costly for very small files**; storing code lengths instead would shrink it.
- The decoder rebuilds the tree from the same frequency table with the same algorithm, so a **compressed file can only be decoded with this exact tree-building behaviour**. Changing how `MinHeap` breaks ties between equal frequencies would make existing `.huff` files decode incorrectly.
- The decoder opens files with plain string paths, while the encoder uses `std::filesystem::u8path`. Paths containing Unicode characters have not been thoroughly tested at decompression time.
- Only `BitIO` has unit tests. Compression and decompression are checked through the benchmark and sample data, with no automated test that the decompressed output matches the original byte for byte.

---

## License

Copyright © 2026 Nam Luu, Huu Nguyen, Duong Hoang.
