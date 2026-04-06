
void stb_saturate(
    unsigned char* grayscale,
    unsigned char* colored,
    int width,
    int height,
    int nrChannels)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int i0 = r * width + c;
            for (int d = 0; d < nrChannels; ++d) {
                int i1 = i0 * nrChannels + d;
                colored[i1] = grayscale[i0];
            }
        }
    }
}