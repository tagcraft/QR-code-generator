#include "DataEncoder.h"
#include <cctype>
#include <stdexcept>

// Alphanumeric character set for QR codes
const std::string ALPHANUMERIC_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

EncodingMode DataEncoder::determineMode(const std::string& data) {
    if (isNumeric(data)) {
        return EncodingMode::NUMERIC;
    } else if (isAlphanumeric(data)) {
        return EncodingMode::ALPHANUMERIC;
    }
    return EncodingMode::BYTE;
}

bool DataEncoder::isNumeric(const std::string& data) {
    for (char c : data) {
        if (!isdigit(c)) return false;
    }
    return !data.empty();
}

bool DataEncoder::isAlphanumeric(const std::string& data) {
    for (char c : data) {
        if (ALPHANUMERIC_CHARS.find(c) == std::string::npos) {
            return false;
        }
    }
    return !data.empty();
}

std::vector<int> DataEncoder::getModeIndicator(EncodingMode mode) {
    switch (mode) {
        case EncodingMode::NUMERIC:      return {0, 0, 0, 1};
        case EncodingMode::ALPHANUMERIC: return {0, 0, 1, 0};
        case EncodingMode::BYTE:         return {0, 1, 0, 0};
        case EncodingMode::KANJI:        return {1, 0, 0, 0};
    }
    return {0, 1, 0, 0}; // Default to BYTE
}

std::vector<int> DataEncoder::getCharacterCountIndicator(int count, EncodingMode mode, int version) {
    int bitCount = 8; // Default for BYTE mode (versions 1-9)
    
    if (version <= 9) {
        switch (mode) {
            case EncodingMode::NUMERIC:      bitCount = 10; break;
            case EncodingMode::ALPHANUMERIC: bitCount = 9;  break;
            case EncodingMode::BYTE:         bitCount = 8;  break;
            case EncodingMode::KANJI:        bitCount = 8;  break;
        }
    }
    
    return intToBits(count, bitCount);
}

std::vector<int> DataEncoder::intToBits(int value, int bitCount) {
    std::vector<int> bits;
    for (int i = bitCount - 1; i >= 0; i--) {
        bits.push_back((value >> i) & 1);
    }
    return bits;
}

std::vector<int> DataEncoder::encodeNumeric(const std::string& data) {
    std::vector<int> bits;
    
    // Process groups of 3 digits
    for (size_t i = 0; i < data.length(); i += 3) {
        int groupSize = std::min(3, (int)(data.length() - i));
        int value = 0;
        
        for (int j = 0; j < groupSize; j++) {
            value = value * 10 + (data[i + j] - '0');
        }
        
        int bitCount = (groupSize == 3) ? 10 : (groupSize == 2) ? 7 : 4;
        auto groupBits = intToBits(value, bitCount);
        bits.insert(bits.end(), groupBits.begin(), groupBits.end());
    }
    
    return bits;
}

std::vector<int> DataEncoder::encodeAlphanumeric(const std::string& data) {
    std::vector<int> bits;
    
    // Process pairs of characters
    for (size_t i = 0; i < data.length(); i += 2) {
        if (i + 1 < data.length()) {
            // Pair of characters
            int val1 = ALPHANUMERIC_CHARS.find(data[i]);
            int val2 = ALPHANUMERIC_CHARS.find(data[i + 1]);
            int value = val1 * 45 + val2;
            auto pairBits = intToBits(value, 11);
            bits.insert(bits.end(), pairBits.begin(), pairBits.end());
        } else {
            // Single character
            int value = ALPHANUMERIC_CHARS.find(data[i]);
            auto charBits = intToBits(value, 6);
            bits.insert(bits.end(), charBits.begin(), charBits.end());
        }
    }
    
    return bits;
}

std::vector<int> DataEncoder::encodeByte(const std::string& data) {
    std::vector<int> bits;
    
    for (char c : data) {
        auto byteBits = intToBits((unsigned char)c, 8);
        bits.insert(bits.end(), byteBits.begin(), byteBits.end());
    }
    
    return bits;
}

int DataEncoder::getCapacity(int version, ErrorCorrectionLevel ecLevel) {
    // Capacity for version 4 in codewords (multiply by 8 for bits)
    if (version == 4) {
        switch (ecLevel) {
            case ErrorCorrectionLevel::L: return 80 * 8;
            case ErrorCorrectionLevel::M: return 64 * 8;
            case ErrorCorrectionLevel::Q: return 48 * 8;
            case ErrorCorrectionLevel::H: return 36 * 8;
        }
    }
    return 64 * 8; // Default to M level
}

void DataEncoder::addPadding(std::vector<int>& bits, int targetCapacity) {
    // Add terminator (up to 4 zeros)
    int terminatorSize = std::min(4, targetCapacity - (int)bits.size());
    for (int i = 0; i < terminatorSize; i++) {
        bits.push_back(0);
    }
    
    // Pad to make complete bytes
   while (bits.size() % 8 != 0 && static_cast<int>(bits.size()) < targetCapacity) {
        bits.push_back(0);
    }
    
    // Add padding bytes (0xEC and 0x11 alternating)
    bool useEC = true;
    while (static_cast<int>(bits.size()) < targetCapacity){
        int padByte = useEC ? 0xEC : 0x11;
        auto padBits = intToBits(padByte, 8);
        bits.insert(bits.end(), padBits.begin(), padBits.end());
        useEC = !useEC;
    }
}

std::vector<int> DataEncoder::encodeData(const std::string& data, 
                                         EncodingMode mode, 
                                         ErrorCorrectionLevel ecLevel,
                                         int version) {
    std::vector<int> bits;
    
    // 1. Add mode indicator
    auto modeIndicator = getModeIndicator(mode);
    bits.insert(bits.end(), modeIndicator.begin(), modeIndicator.end());
    
    // 2. Add character count indicator
    auto countIndicator = getCharacterCountIndicator(data.length(), mode, version);
    bits.insert(bits.end(), countIndicator.begin(), countIndicator.end());
    
    // 3. Encode the data
    std::vector<int> dataBits;
    switch (mode) {
        case EncodingMode::NUMERIC:
            dataBits = encodeNumeric(data);
            break;
        case EncodingMode::ALPHANUMERIC:
            dataBits = encodeAlphanumeric(data);
            break;
        case EncodingMode::BYTE:
            dataBits = encodeByte(data);
            break;
        default:
            throw std::runtime_error("Unsupported encoding mode");
    }
    bits.insert(bits.end(), dataBits.begin(), dataBits.end());
    
    // 4. Add padding
    int capacity = getCapacity(version, ecLevel);
    addPadding(bits, capacity);
    
    return bits;
}