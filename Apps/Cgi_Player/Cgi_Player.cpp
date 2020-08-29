#include <Mlib/Cpx/Convenience.hpp>
#include <fstream>
#include <iostream>
// #include <Mlib/Cpx/FileList.hpp>

using namespace Mlib::cpx;

void render(const std::list<std::string>& pathes, std::ostream& ostream) {
    auto contents_ = gen_renderables(
        pathes, [](const std::string& s){return _<div_>(s + "ff");});
    auto meta_ = _<meta>({});
    meta_->attr("charset", "utf-8");
    auto head_ = _<head>({meta_});
    auto body_ = _<body>(contents_);
    auto html_ = _<html>({head_, body_});
    ostream << "<!DOCTYPE html>" << std::endl;
    ostream << *html_;
}

int main(int argc, const char** argv) {
    std::ofstream of("index.html");

    render({"hello.mp3", "hello.m4a"}, of);
    return 0;
}
