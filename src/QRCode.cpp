#include "QRCode.h"
#include "DataEncoder.h"
#include "ErrorCorrection.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>

QRCode::QRCode(const std::string& data, ErrorCorrectionLevel level) 
    : version(4), size(33), ecLevel(level), inputData(data) {
    // Version 4 is 33x33 (21 + 4*3)
    mode = DataEncoder::determineMode(data);
    matrix.resize(size, std::vector<int>(size, -1)); // -1 means unset
}

void QRCode::initializeMatrix() {
    // Clear matrix
    for (auto& row : matrix) {
        std::fill(row.begin(), row.end(), -1);
    }
}

void QRCode::addFinderPatterns() {
    // Finder pattern is 7x7
    auto drawFinder = [this](int row, int col) {
        // Outer 7x7 black border
        for (int i = 0; i < 7; i++) {
            for (int j = 0; j < 7; j++) {
                bool isBlack = (i == 0 || i == 6 || j == 0 || j == 6 ||
                               (i >= 2 && i <= 4 && j >= 2 && j <= 4));
                matrix[row + i][col + j] = isBlack ? 1 : 0;
            }
        }
    };
    
    // Top-left
    drawFinder(0, 0);
    // Top-right
    drawFinder(0, size - 7);
    // Bottom-left
    drawFinder(size - 7, 0);
}

void QRCode::addSeparators() {
    // White separators around finder patterns
    auto drawSeparator = [this](int row, int col, int width, int height) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (row + i >= 0 && row + i < size && 
                    col + j >= 0 && col + j < size) {
                    matrix[row + i][col + j] = 0;
                }
            }
        }
    };
    
    // Top-left separator
    drawSeparator(7, 0, 8, 1);
    drawSeparator(0, 7, 1, 8);
    
    // Top-right separator
    drawSeparator(7, size - 8, 8, 1);
    drawSeparator(0, size - 8, 1, 8);
    
    // Bottom-left separator
    drawSeparator(size - 8, 0, 8, 1);
    drawSeparator(size - 8, 7, 1, 8);
}

void QRCode::addTimingPatterns() {
    // Horizontal timing pattern
    for (int i = 8; i < size - 8; i++) {
        matrix[6][i] = (i % 2 == 0) ? 1 : 0;
    }
    
    // Vertical timing pattern
    for (int i = 8; i < size - 8; i++) {
        matrix[i][6] = (i % 2 == 0) ? 1 : 0;
    }
}

void QRCode::addDarkModule() {
    // Always placed at (4*version + 9, 8)
    matrix[4 * version + 9][8] = 1;
}

void QRCode::placeDataBits(const std::vector<int>& bits) {
    int bitIndex = 0;
    bool up = true;
    
    // Start from bottom-right, move in zigzag
    for (int col = size - 1; col > 0; col -= 2) {
        if (col == 6) col--; // Skip timing column
        
        for (int i = 0; i < size; i++) {
            int row = up ? (size - 1 - i) : i;
            
            // Place in right column of pair
            if (matrix[row][col] == -1 && bitIndex < static_cast<int>(bits.size())) {
 
                matrix[row][col] = bits[bitIndex++];
            }
            
            // Place in left column of pair
            if (matrix[row][col] == -1 && bitIndex < static_cast<int>(bits.size())) {

                matrix[row][col - 1] = bits[bitIndex++];
            }
        }
        
        up = !up;
    }
    
    // Fill any remaining cells with 0
    for (auto& row : matrix) {
        for (auto& cell : row) {
            if (cell == -1) cell = 0;
        }
    }
}

void QRCode::applyMask(int maskPattern) {
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            // Don't mask function patterns
            if ((row < 9 && col < 9) || 
                (row < 9 && col >= size - 8) ||
                (row >= size - 8 && col < 9) ||
                row == 6 || col == 6) {
                continue;
            }
            
            bool shouldFlip = false;
            switch (maskPattern) {
                case 0: shouldFlip = ((row + col) % 2 == 0); break;
                case 1: shouldFlip = (row % 2 == 0); break;
                case 2: shouldFlip = (col % 3 == 0); break;
                case 3: shouldFlip = ((row + col) % 3 == 0); break;
                case 4: shouldFlip = ((row / 2 + col / 3) % 2 == 0); break;
                case 5: shouldFlip = ((row * col) % 2 + (row * col) % 3 == 0); break;
                case 6: shouldFlip = (((row * col) % 2 + (row * col) % 3) % 2 == 0); break;
                case 7: shouldFlip = (((row + col) % 2 + (row * col) % 3) % 2 == 0); break;
            }
            
            if (shouldFlip) {
                matrix[row][col] ^= 1;
            }
        }
    }
}

int QRCode::calculatePenalty() {
    // Simplified penalty calculation
    int penalty = 0;
    
    // Rule 1: Consecutive same-colored modules in row/column
    for (int i = 0; i < size; i++) {
        int rowCount = 1, colCount = 1;
        for (int j = 1; j < size; j++) {
            if (matrix[i][j] == matrix[i][j-1]) rowCount++;
            else { if (rowCount >= 5) penalty += (rowCount - 2); rowCount = 1; }
            
            if (matrix[j][i] == matrix[j-1][i]) colCount++;
            else { if (colCount >= 5) penalty += (colCount - 2); colCount = 1; }
        }
        if (rowCount >= 5) penalty += (rowCount - 2);
        if (colCount >= 5) penalty += (colCount - 2);
    }
    
    return penalty;
}

int QRCode::selectBestMask() {
    int bestMask = 0;
    int lowestPenalty = INT32_MAX;
    
    for (int mask = 0; mask < 8; mask++) {
        // Create a copy and apply mask
        auto original = matrix;
        applyMask(mask);
        int penalty = calculatePenalty();
        
        if (penalty < lowestPenalty) {
            lowestPenalty = penalty;
            bestMask = mask;
        }
        
        // Restore original
        matrix = original;
    }
    
    return bestMask;
}

void QRCode::generate() {
    // Step 1: Initialize matrix
    initializeMatrix();
    
    // Step 2: Add function patterns
    addFinderPatterns();
    addSeparators();
    addTimingPatterns();
    addDarkModule();
    
    // Step 3: Encode data
    auto encodedBits = DataEncoder::encodeData(inputData, mode, ecLevel, version);
    
    // Step 4: Add error correction
    auto finalBits = ErrorCorrection::addErrorCorrection(encodedBits, ecLevel, version);
    
    // Step 5: Place data bits
    placeDataBits(finalBits);
    
    // Step 6: Apply best mask
    int bestMask = selectBestMask();
    applyMask(bestMask);
}

void QRCode::printToConsole() {
    std::cout << "\nQR Code (Version " << version << ", " << size << "x" << size << "):\n";
    std::cout << "Mode: ";
    switch (mode) {
        case EncodingMode::NUMERIC: std::cout << "NUMERIC"; break;
        case EncodingMode::ALPHANUMERIC: std::cout << "ALPHANUMERIC"; break;
        case EncodingMode::BYTE: std::cout << "BYTE"; break;
        case EncodingMode::KANJI: std::cout << "KANJI"; break;
    }
    std::cout << "\n\n";
    
    // Add quiet zone (4 modules)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size + 8; j++) {
            std::cout << "  ";
        }
        std::cout << "\n";
    }
    
    // Print QR code
    for (int i = 0; i < size; i++) {
        std::cout << "        "; // Left quiet zone
        for (int j = 0; j < size; j++) {
            std::cout << (matrix[i][j] == 1 ? "██" : "  ");
        }
        std::cout << "\n";
    }
    
    // Bottom quiet zone
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size + 8; j++) {
            std::cout << "  ";
        }
        std::cout << "\n";
    }
}

void QRCode::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return;
    }
    
    // Save as text representation
    file << "QR Code - Version " << version << "\n";
    file << "Data: " << inputData << "\n\n";
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            file << (matrix[i][j] == 1 ? "█" : " ");
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "QR Code saved to " << filename << "\n";
}