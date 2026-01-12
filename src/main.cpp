#include "QRCode.h"
#include <iostream>

int main() {
    std::string input;

    std::cout << "Enter text to encode in QR: ";
    std::getline(std::cin, input);

    if (input.empty()) {
        std::cerr << "Error: Input cannot be empty\n";
        return 1;
    }

    QRCode qr(input, ErrorCorrectionLevel::M);
    qr.generate();

    qr.printToConsole();
    qr.saveToFile("qrcode_output.txt");

    return 0;
}
