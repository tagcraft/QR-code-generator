#include "ErrorCorrection.h"
#include <stdexcept>

// Convert bits to bytes
std::vector<int> ErrorCorrection::bitsToBytes(const std::vector<int>& bits) {
    std::vector<int> bytes;
    for (size_t i = 0; i + 7 < bits.size(); i += 8) {
        int value = 0;
        for (int j = 0; j < 8; j++) {
            value = (value << 1) | bits[i + j];
        }
        bytes.push_back(value);
    }
    return bytes;
}

// Convert bytes back to bits
std::vector<int> ErrorCorrection::bytesToBits(const std::vector<int>& bytes) {
    std::vector<int> bits;
    for (int b : bytes) {
        for (int i = 7; i >= 0; i--) {
            bits.push_back((b >> i) & 1);
        }
    }
    return bits;
}

// Dummy Galois Field multiplication (simplified)
int ErrorCorrection::gfMul(int a, int b) {
    return (a * b) % 256;
}

int ErrorCorrection::gfDiv(int a, int b) {
    if (b == 0) throw std::runtime_error("GF divide by zero");
    return a / b;
}

int ErrorCorrection::gfPow(int a, int power) {
    int result = 1;
    for (int i = 0; i < power; i++) {
        result = gfMul(result, a);
    }
    return result;
}

// Generator polynomial placeholder
std::vector<int> ErrorCorrection::getGeneratorPolynomial(int degree) {
    std::vector<int> poly(degree + 1, 1);
    return poly;
}

// Fake Reed–Solomon ECC (structure-correct)
std::vector<int> ErrorCorrection::generateErrorCorrectionCodewords(
    const std::vector<int>& dataCodewords,
    int numECCodewords) {

    std::vector<int> ec(numECCodewords, 0);

    // Simple parity-like ECC
    for (size_t i = 0; i < dataCodewords.size(); i++) {
        ec[i % numECCodewords] ^= dataCodewords[i];
    }

    return ec;
}

// Public API
std::vector<int> ErrorCorrection::addErrorCorrection(
    const std::vector<int>& dataBits,
    ErrorCorrectionLevel ecLevel,
    int /* version */) {


    auto dataBytes = bitsToBytes(dataBits);

    int ecCodewords = 16; // Version 4, EC-M approx
    if (ecLevel == ErrorCorrectionLevel::L) ecCodewords = 10;
    if (ecLevel == ErrorCorrectionLevel::Q) ecCodewords = 24;
    if (ecLevel == ErrorCorrectionLevel::H) ecCodewords = 28;

    auto ecBytes = generateErrorCorrectionCodewords(dataBytes, ecCodewords);

    dataBytes.insert(dataBytes.end(), ecBytes.begin(), ecBytes.end());
    return bytesToBits(dataBytes);
}
