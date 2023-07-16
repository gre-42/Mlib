/*
Can load easier and more indepth with https://github.com/Hydroque/DDSLoader

Because a lot of crappy, weird DDS file loader files were found online. The resources are actually VERY VERY limited.

Written in C, can very easily port to C++ through casting mallocs (ensure your imports are correct), goto can be replaced.

https://www.gamedev.net/forums/topic/637377-loading-dds-textures-in-opengl-black-texture-showing/
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/
^ Two examples of terrible code

https://gist.github.com/Hydroque/d1a8a46853dea160dc86aa48618be6f9
^ My first look and clean up 'get it working'

https://ideone.com/WoGThC
^ Improvement details

File Structure:

  Section     Length
  ///////////////////
  FILECODE    4
  HEADER      124
  HEADER_DX10* 20  (https://msdn.microsoft.com/en-us/library/bb943983(v=vs.85).aspx)
  PIXELS      fseek(f, 0, SEEK_END); (ftell(f) - 128) - (fourCC == "DX10" ? 17 or 20 : 0)

* the link tells you that this section isn't written unless its a DX10 file


Supports DXT1, DXT3, DXT5.
The problem with supporting DX10 is you need to know what it is used for and how opengl would use it.

File Byte Order:

typedef unsigned int DWORD;           // 32bits little endian

  type   index    attribute           // description
///////////////////////////////////////////////////////////////////////////////////////////////
  DWORD  0        file_code;          //. always `DDS `, or 0x20534444
  DWORD  4        size;               //. size of the header, always 124 (includes PIXELFORMAT)
  DWORD  8        flags;              //. bitflags that tells you if data is present in the file
                                      //      CAPS         0x1
                                      //      HEIGHT       0x2
                                      //      WIDTH        0x4
                                      //      PITCH        0x8
                                      //      PIXELFORMAT  0x1000
                                      //      MIPMAPCOUNT  0x20000
                                      //      LINEARSIZE   0x80000
                                      //      DEPTH        0x800000
  DWORD  12       height;             //. height of the base image (biggest mipmap)
  DWORD  16       width;              //. width of the base image (biggest mipmap)
  DWORD  20       pitchOrLinearSize;  //. bytes per scan line in an uncompressed texture, or bytes in the top level texture for a compressed texture
                                      //     D3DX11.lib and other similar libraries unreliably or inconsistently provide the pitch, convert with
                                      //     DX* && BC*: max( 1, ((width+3)/4) ) * block-size
                                      //     *8*8_*8*8 && UYVY && YUY2: ((width+1) >> 1) * 4
                                      //     (width * bits-per-pixel + 7)/8 (divide by 8 for byte alignment, whatever that means)
  DWORD  24       depth;              //. Depth of a volume texture (in pixels), garbage if no volume data
  DWORD  28       mipMapCount;        //. number of mipmaps, garbage if no pixel data
  DWORD  32       reserved1[11];      //. unused
  DWORD  76       Size;               //. size of the following 32 bytes (PIXELFORMAT)
  DWORD  80       Flags;              //. bitflags that tells you if data is present in the file for following 28 bytes
                                      //      ALPHAPIXELS  0x1
                                      //      ALPHA        0x2
                                      //      FOURCC       0x4
                                      //      RGB          0x40
                                      //      YUV          0x200
                                      //      LUMINANCE    0x20000
  DWORD  84       FourCC;             //. File format: DXT1, DXT2, DXT3, DXT4, DXT5, DX10. 
  DWORD  88       RGBBitCount;        //. Bits per pixel
  DWORD  92       RBitMask;           //. Bit mask for R channel
  DWORD  96       GBitMask;           //. Bit mask for G channel
  DWORD  100      BBitMask;           //. Bit mask for B channel
  DWORD  104      ABitMask;           //. Bit mask for A channel
  DWORD  108      caps;               //. 0x1000 for a texture w/o mipmaps
                                      //      0x401008 for a texture w/ mipmaps
                                      //      0x1008 for a cube map
  DWORD  112      caps2;              //. bitflags that tells you if data is present in the file
                                      //      CUBEMAP           0x200     Required for a cube map.
                                      //      CUBEMAP_POSITIVEX 0x400     Required when these surfaces are stored in a cube map.
                                      //      CUBEMAP_NEGATIVEX 0x800     ^
                                      //      CUBEMAP_POSITIVEY 0x1000    ^
                                      //      CUBEMAP_NEGATIVEY 0x2000    ^
                                      //      CUBEMAP_POSITIVEZ 0x4000    ^
                                      //      CUBEMAP_NEGATIVEZ 0x8000    ^
                                      //      VOLUME            0x200000  Required for a volume texture.
  DWORD  114      caps3;              //. unused
  DWORD  116      caps4;              //. unused
  DWORD  120      reserved2;          //. unused
*/

#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <cstring>
#include <iostream>
#include <vector>

using namespace Mlib;

#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

GLuint texture_loadDDS(std::istream& istr) {
  // lay out variables to be used
  
  unsigned int width;
  unsigned int height;
  unsigned int mipMapCount;
  
  unsigned int blockSize;
  unsigned int format;
  
  unsigned int w;
  unsigned int h;
  
  GLuint tid = 0;
  
  // open the DDS file for binary reading and get file size
  istr.seekg(0, std::ios_base::end);
  long file_size = istr.tellg();
  istr.seekg(0);
  if (file_size < 128)
    throw std::runtime_error("File too small (" + std::to_string(file_size) + ')');
  
  // allocate new unsigned char space with 4 (file code) + 124 (header size) bytes
  // read in 128 bytes from the file
  std::vector<unsigned char> header(128);
  istr.read((char*)header.data(), 128);
  
  // compare the `DDS ` signature
  if(memcmp(header.data(), "DDS ", 4) != 0)
    throw std::runtime_error("Signature mismatch");
  
  // extract height, width, and amount of mipmaps - yes it is stored height then width
  height = (unsigned int)((header[12]) | (header[13] << 8) | (header[14] << 16) | (header[15] << 24));
  width = (unsigned int)((header[16]) | (header[17] << 8) | (header[18] << 16) | (header[19] << 24));
  mipMapCount = (unsigned int)((header[28]) | (header[29] << 8) | (header[30] << 16) | (header[31] << 24));
  
  // figure out what format to use for what fourCC file type it is
  // block size is about physical chunk storage of compressed data in file (important)
  if(header[84] == 'D') {
    switch(header[87]) {
      case '1': // DXT1
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        blockSize = 8;
        break;
      case '3': // DXT3
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        blockSize = 16;
        break;
      case '5': // DXT5
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        blockSize = 16;
        break;
      case '0': // DX10
        // unsupported, else will error
        // as it adds sizeof(struct DDS_HEADER_DXT10) between pixels
        // so, buffer = malloc((file_size - 128) - sizeof(struct DDS_HEADER_DXT10));
      default:
	  	  throw std::runtime_error("Unsupported data type (0)");
    }
  } else // BC4U/BC4S/ATI2/BC55/R8G8_B8G8/G8R8_G8B8/UYVY-packed/YUY2-packed unsupported
    throw std::runtime_error("Unsupported data type (1)");
  
  // allocate new unsigned char space with file_size - (file_code + header_size) magnitude
  // read rest of file
  std::vector<unsigned char> buffer(integral_cast<size_t>(file_size) - 128);
  istr.read((char*)buffer.data(), file_size - 128);
  
  // prepare new incomplete texture
  CHK(glGenTextures(1, &tid));
  
  // bind the texture
  // make it complete by specifying all needed parameters and ensuring all mipmaps are filled
  CHK(glBindTexture(GL_TEXTURE_2D, tid));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, integral_cast<GLint>(mipMapCount)-1)); // opengl likes array length of mipmaps
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)); // don't forget to enable mipmaping
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    
    // prepare some variables
    unsigned int offset = 0;
    unsigned int size = 0;
    w = width;
    h = height;
    
    // loop through sending block at a time with the magic formula
    // upload to opengl properly, note the offset transverses the pointer
    // assumes each mipmap is 1/2 the size of the previous mipmap
    for (unsigned int i=0; i<mipMapCount; i++) {
      if(w == 0 || h == 0) { // discard any odd mipmaps 0x1 0x2 resolutions
        mipMapCount--;
        continue;
      }
      size = ((w+3)/4) * ((h+3)/4) * blockSize;
      CHK(glCompressedTexImage2D(
        GL_TEXTURE_2D,
        integral_cast<GLint>(i),
        format,
        integral_cast<GLsizei>(w),
        integral_cast<GLsizei>(h),
        0,
        integral_cast<GLsizei>(size),
        buffer.data() + offset));
      offset += size;
      w /= 2;
      h /= 2;
    }
      // discard any odd mipmaps, ensure a complete texture
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, integral_cast<GLint>(mipMapCount)-1));
    // unbind
  CHK(glBindTexture(GL_TEXTURE_2D, 0));
  
  // easy macro to get out quick and uniform (minus like 15 lines of bulk)
  return tid;
}
