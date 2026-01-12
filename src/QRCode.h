#ifndef QRCODE_H
#define QRCODE_H

#include <string>
#include <vector>

enum class EncodingMode {
    NUMERIC,
    ALPHANUMERIC,
    BYTE,
    KANJI
};

enum class ErrorCorrectionLevel {
    L,  // 7% recovery
    M,  // 15% recovery
    Q,  // 25% recovery
    H   // 30% recovery
};

class QRCode {
private:
    int version;  // We'll use version 4 (33x33)
    int size;     // Size of the QR code matrix
    ErrorCorrectionLevel ecLevel;
    EncodingMode mode;
    std::vector<std::vector<int>> matrix;
    std::string inputData;

    // Helper functions
    void initializeMatrix();
    void addFinderPatterns();
    void addSeparators();
    void addTimingPatterns();
    void addDarkModule();
    void placeDataBits(const std::vector<int>& bits);
    void applyMask(int maskPattern);
    int calculatePenalty();
    int selectBestMask();

public:
    QRCode(const std::string& data, ErrorCorrectionLevel level = ErrorCorrectionLevel::M);
    
    void generate();
    void printToConsole();
    void saveToFile(const std::string& filename);
    
    EncodingMode getMode() const { return mode; }
    int getSize() const { return size; }
};

#endif // QRCODE_H