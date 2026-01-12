#ifndef ERRORCORRECTION_H
#define ERRORCORRECTION_H

#include <vector>
#include "QRCode.h"

class ErrorCorrection {
public:
    // Add Reed-Solomon error correction codes
    static std::vector<int> addErrorCorrection(const std::vector<int>& dataBits,
                                               ErrorCorrectionLevel ecLevel,
                                               int version);
    
private:
    // Convert bits to bytes
    static std::vector<int> bitsToBytes(const std::vector<int>& bits);
    
    // Convert bytes to bits
    static std::vector<int> bytesToBits(const std::vector<int>& bytes);
    
    // Simplified Reed-Solomon encoding (polynomial division)
    static std::vector<int> generateErrorCorrectionCodewords(
        const std::vector<int>& dataCodewords,
        int numECCodewords);
    
    // Galois Field operations
    static int gfMul(int a, int b);
    static int gfDiv(int a, int b);
    static int gfPow(int a, int power);
    
    // Generator polynomial
    static std::vector<int> getGeneratorPolynomial(int degree);
};

#endif // ERRORCORRECTION_H