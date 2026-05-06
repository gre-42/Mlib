#include "Binary.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void Mlib::print_char(char c) {
    char v = (c >= ' ') && (c <= '~') ? c : '.';
    linfo() << "Read: " << std::hex << "0x" << std::setfill('0') << std::setw(2) << (uint32_t)(uint8_t)c << " - " << v;
}

void Mlib::print_chars(std::span<char> span, const char* msg) {
    if (msg != nullptr) {
        linfo() << "Msg: " << msg;
    }
    for (char c : span) {
        print_char(c);
    }
}

std::vector<std::byte> Mlib::read_all_vector(std::istream& istr, const char* msg, IoVerbosity verbosity) {
    istr.seekg(0, std::istream::end);
    std::streamoff file_size = istr.tellg();
    istr.seekg(0, std::istream::beg);
    std::vector<std::byte> res(integral_cast<size_t>(file_size));
    read_vector(istr, res, msg, verbosity);
    return res;
}

std::string Mlib::read_string(std::istream& istr, size_t length, const char* msg, IoVerbosity verbosity) {
    if (length > 1'000) {
        throw std::runtime_error("String too large: " + std::string(msg));
    }
    std::string s(length, '?');
    read_vector(istr, s, msg, verbosity);
    return s;
}

void Mlib::seek_relative_positive(std::istream& istr, std::streamoff amount, IoVerbosity verbosity) {
    if (amount < 0) {
        throw std::runtime_error("Seek in negative direction");
    }
    if (any(verbosity & IoVerbosity::DATA)) {
        for (std::streamoff i = 0; i < amount; ++i) {
            auto c = istr.get();
            if (c == EOF) {
                throw std::runtime_error("Could not read char");
            }
            print_char((char)c);
        }
    } else {
        istr.seekg(amount, std::ios::cur);
        if (istr.fail()) {
            throw std::runtime_error("Seekg failed");
        }
    }
}
