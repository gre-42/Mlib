#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <iostream>

using namespace Mlib;

void pgm_test() {
    auto im = PgmImage::from_float(uniform_random_array<float>(ArrayShape{5, 6}, 1));
    im.save_to_file("TestOut/rpgm.pgm");
    auto ld = PgmImage::load_from_file("TestOut/rpgm.pgm");
    assert_allequal(im, ld);
}

int main(int argc, char **argv) {
    pgm_test();
    return 0;
}
