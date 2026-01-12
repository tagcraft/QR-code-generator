# QR-code-generator# QR Code Generator (C++)

This project is a **QR Code generator implemented from scratch in C++** without using any external QR or image libraries.  
The goal of the project is to understand **how QR codes work internally**, rather than just generating images using existing libraries.

---

## What This Project Does

- Takes a text input from the user
- Encodes the data into QR-compatible binary format
- Adds error correction bits
- Places data into a QR matrix using zig-zag traversal
- Applies masking and selects the best mask
- Displays the QR code in the terminal
- Saves the QR code to a text file

The implementation uses **QR Version 4 (33×33)**.

---

## How the QR Code Is Generated (Working)

1. **Mode Detection**
   - Determines whether input data is:
     - Numeric
     - Alphanumeric
     - Byte

2. **Data Encoding**
   - Adds:
     - Mode indicator
     - Character count indicator
     - Encoded data bits
   - Pads the data to fit QR capacity

3. **Error Correction**
   - Error correction codewords are added using a **simplified Reed–Solomon structure**
   - Helps the QR code tolerate minor data loss

4. **QR Matrix Construction**
   - Creates a 33×33 matrix
   - Adds:
     - Finder patterns
     - Separators
     - Timing patterns
     - Dark module

5. **Data Placement**
   - Places bits into the matrix using **zig-zag traversal** (bottom-right to top-left)

6. **Masking**
   - Applies all 8 QR mask patterns
   - Calculates penalty for each
   - Selects the mask with the lowest penalty

7. **Output**
   - Prints QR code in terminal
   - Saves QR structure to a text file

---


### File Descriptions

- **main.cpp**
  - Entry point of the program
  - Takes user input and generates the QR code

- **QRCode.h / QRCode.cpp**
  - Handles QR matrix creation
  - Adds patterns (finder, timing, mask)
  - Places data bits
  - Applies masking and penalty calculation
  - Prints and saves output

- **DataEncoder.h / DataEncoder.cpp**
  - Detects encoding mode
  - Converts input data into bit stream
  - Adds padding and capacity handling

- **ErrorCorrection.h / ErrorCorrection.cpp**
  - Adds error correction bits
  - Uses a simplified Reed–Solomon style approach

---

## Time Complexity

Let **n = number of bits** and **m = QR matrix size (33×33)**

- Data encoding: **O(n)**
- Error correction: **O(n)**
- Matrix placement: **O(m²)**
- Mask evaluation (8 masks): **O(8 × m²) ≈ O(m²)**

### Overall Time Complexity: O(n + m²)

---

## Space Complexity

- Data bits storage: **O(n)**
- QR matrix storage: **O(m²)**

### Overall Space Complexity: O(n + m²)


---

## Limitations

- QR version is fixed to Version 4
- Error correction is simplified (not full GF(256))
- Output is text-based (no PNG/image generation)

These limitations are intentional to focus on **core QR logic and learning**.

---

## Why This Project Is Useful

- Demonstrates low-level understanding of QR code standards
- Shows bit manipulation and matrix traversal skills
- Avoids black-box library usage


---

## How to Build and Run

```bash
make
./qrcode

