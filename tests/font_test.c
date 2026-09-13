#include "font.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void assert_text_is_supported(const char *text)
{
   for (size_t i = 0; text[i] != '\0'; i++) {
      assert(font_get_glyph(text[i]) != NULL);
   }
}

int main(void)
{
   for (char lowercase = 'a'; lowercase <= 'z'; lowercase++) {
      char uppercase = lowercase - 'a' + 'A';

      assert(memcmp(font_get_glyph(lowercase), font_get_glyph(uppercase),
                    GLYPH_WIDTH) == 0);
   }

   assert_text_is_supported("DEVLOG");
   assert_text_is_supported("Analysis complete");
   assert_text_is_supported(
      "The analysis found relevant architectural evidence.");
   assert_text_is_supported("ABC XYZ");
   assert_text_is_supported("abc xyz");
   assert_text_is_supported("Mixed Case Text");
   assert_text_is_supported("0123456789");
   assert_text_is_supported(" :-./");

   return 0;
}
