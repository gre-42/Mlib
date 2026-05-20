#include "Base64.hpp"
#include <stdexcept>

using namespace Mlib;

static int base64_char_value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return 0; // Padding
    throw std::runtime_error("Invalid base64 character: code=" + std::to_string((int)(unsigned char)c));
}

std::vector<std::byte> Mlib::decode_base64(const std::string& base64_str) {
    if (base64_str.empty()) return {};

    std::vector<std::byte> result;
    result.reserve((base64_str.length() / 4) * 3);

    int val = 0;
    int val_bits = -8;

    for (char c : base64_str) {
        if (c == '=') break; // Ignore trailing padding

        val = (val << 6) + base64_char_value(c);
        val_bits += 6;

        if (val_bits >= 0) {
            result.push_back(static_cast<std::byte>((val >> val_bits) & 0xFF));
            val_bits -= 8;
        }
    }
    return result;
}
