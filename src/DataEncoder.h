#ifndef DATAENCODER_H
#define DATAENCODER_H

#include <string>
#include <vector>
#include "QRCode.h"

class DataEncoder {
public:
    // Determine encoding mode for input data
    static EncodingMode determineMode(const std::string& data);
    
    // Encode data into bit string
    static std::vector<int> encodeData(const std::string& data, 
                                       EncodingMode mode, 
                                       ErrorCorrectionLevel ecLevel,
                                       int version);
    
private:
    // Check if data can be encoded in specific mode
    static bool isNumeric(const std::string& data);
    static bool isAlphanumeric(const std::string& data);
    
    // Encoding for different modes
    static std::vector<int> encodeNumeric(const std::string& data);
    static std::vector<int> encodeAlphanumeric(const std::string& data);
    static std::vector<int> encodeByte(const std::string& data);
    
    // Helper functions
    static std::vector<int> getModeIndicator(EncodingMode mode);
    static std::vector<int> getCharacterCountIndicator(int count, EncodingMode mode, int version);
    static std::vector<int> intToBits(int value, int bitCount);
    static void addPadding(std::vector<int>& bits, int targetCapacity);
    
    // Get capacity for version and error correction level
    static int getCapacity(int version, ErrorCorrectionLevel ecLevel);
};

#endif // DATAENCODER_H