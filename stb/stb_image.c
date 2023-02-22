#include <Mlib/Features.hpp>
#define STB_IMAGE_IMPLEMENTATION
#ifdef WITHOUT_THREAD_LOCAL
#define STBI_NO_THREAD_LOCALS
#endif
#include "stb_image_bpc.h"

static void *stbi__load_and_postprocess_bpc(stbi__context *s, int *x, int *y, int *comp, int req_comp, int *bits_per_channel)
{
   stbi__result_info ri;
   void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 8);

   if (result == NULL)
      return NULL;

   if (stbi__vertically_flip_on_load) {
      int channels = req_comp ? req_comp : *comp;
      if ((ri.bits_per_channel != 8) &&
          (ri.bits_per_channel != 16))
      {
         abort();
      }
      stbi__vertical_flip(result, *x, *y, channels * (ri.bits_per_channel / 8));
   }

   *bits_per_channel = ri.bits_per_channel;
   return result;
}

STBIDEF void *stbi_load_from_memory_bpc(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp, int *bits_per_channel)
{
   stbi__context s;
   stbi__start_mem(&s,buffer,len);
   return stbi__load_and_postprocess_bpc(&s,x,y,comp,req_comp,bits_per_channel);
}

STBIDEF void *stbi_load_from_file_bpc(FILE *f, int *x, int *y, int *comp, int req_comp, int *bits_per_channel)
{
   unsigned char *result;
   stbi__context s;
   stbi__start_file(&s,f);
   result = stbi__load_and_postprocess_bpc(&s,x,y,comp,req_comp,bits_per_channel);
   if (result) {
      // need to 'unget' all the characters in the IO buffer
      fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
   }
   return result;
}

STBIDEF void *stbi_load_bpc(char const *filename, int *x, int *y, int *comp, int req_comp, int *bits_per_channel)
{
   FILE *f = stbi__fopen(filename, "rb");
   unsigned char *result;
   if (!f) return stbi__errpuc("can't fopen", "Unable to open file");
   result = stbi_load_from_file_bpc(f,x,y,comp,req_comp,bits_per_channel);
   fclose(f);
   return result;
}
