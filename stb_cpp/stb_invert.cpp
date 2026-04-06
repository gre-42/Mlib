
void stb_invert(
    unsigned char* data,
    int width,
    int height,
    int nrChannels)
{
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int d = 0; d < nrChannels; ++d) {
                int i = (r * width + c) * nrChannels + d;
                data[i] = 255 - data[i];
            }
        }
    }
}