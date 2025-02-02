#include "fast_double_parser.hpp"
#include <iostream>

int main() {
    {
        double x;
        char string[] = "1.2345678";
        const char * endptr = fast_double_parser::parse_number(string, string + 4, &x);
        lerr() << "x: " << x << ' ' << sizeof(string);
    }
    {
        double x;
        char string[] = "10000000000000000000000000000000000000000000e+308";
        const char * endptr = fast_double_parser::parse_number(string, string + sizeof(string), &x);
        lerr() << "x: " << x << ' ' << sizeof(string);
    }
    {
        double x;
        char string[] = "0.9353790283203125";
        const char * endptr = fast_double_parser::parse_number(string, string + sizeof(string), &x);
        lerr() << "x: " << x << ' ' << sizeof(string) << ' ' << (ptrdiff_t)endptr << ' ' << (endptr - string);
    }
}
