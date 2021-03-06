
#define _GNU_SOURCE
#include <stdlib.h>

#include "Prototypes.h"

/*{

struct Text_ {
   char* data;
   int dataSize;
   int bytes;
   int chars;
};

#define Text_isSet(this) ((this).data)
#define Text_chars(this) ((this).chars)
#define Text_bytes(this) ((this).bytes)
#define Text_toString(this) ((this).data)

}*/

bool UTF8_isValid(const char* text) {
   while (*text) {
      int extra = 0;
      unsigned char c = *text;
      text++;
      if (c >> 7 == 0) {
         continue;
      } else if (c >> 5 == 0x06) {
         extra = 1;
      } else if (c >> 4 == 0x0e) {
         extra = 2;
      } else if (c >> 3 == 0x1e) {
         extra = 3;
      } else {
         return false;
      }
      for (; extra; extra--) {
         unsigned char c = *text;
         if (c >> 6 != 0x02)
            return false;
         text++;
      }
   }
   return true;
}

int UTF8_bytes(const char sc) {
   const unsigned char c = (const unsigned char) sc;
   if (c >> 7 == 0)         return 1;
   else if (c >> 5 == 0x06) return 2;
   else if (c >> 4 == 0x0e) return 3;
   else if (c >> 3 == 0x1e) return 4;
   // TODO handle invalid UTF-8 sequences
   return 1;
}

static int UTF8_chars(const char* s) {
   int i = 0;
   unsigned char* c = (unsigned char*) s;
   while (*c) {
      if (*c >> 6 != 2)
         i++;
      c++;
   }
   return i;
}

int UTF8_copyChar(char* sdest, const char* ssrc) {
   unsigned char* dest = (unsigned char*) sdest;
   const unsigned char* src = (const unsigned char*) ssrc;
   int offset = 1;
   *dest = *src;
   return offset;
   src++;
   while (*src >> 6 == 2) {
      dest++;
      *dest = *src;
      src++;
      offset++;
   } 
   return offset;
}

wchar_t UTF8_stringToCodePoint(const char* ss) {
   const unsigned char* s = (const unsigned char*) ss;
   if (*s >> 7 == 0)
      return *s;
   if (*s >> 5 == 0x06)
      return ((*s & 0x1f) << 6) | (*(s+1) & 0x3f);
   if (*s >> 4 == 0x0e)
      return ((*s & 0x1f) << 12) | ((*(s+1) & 0x3f) << 6) | (*(s+2) & 0x3f);
   if (*s >> 3 == 0x1e)
      return ((*s & 0x1f) << 18) | ((*(s+1) & 0x3f) << 12) | ((*(s+2) & 0x3f) << 6) | (*(s+3) & 0x3f);
   return 0;
}

static int UTF8_codePointToString(char* sdest, wchar_t c) {
   unsigned char* dest = (unsigned char*) sdest;
   if (c >> 7 == 0) {
      dest[0] = c; dest[1] = '\0';
      return 1;
   } else if (c >> 11 == 0) {
      dest[0] = c >> 6 | 0xc0;
      dest[1] = (c & 0x3f) | 0x80;
      dest[2] = '\0';
      return 2;
   } else if (c >> 16 == 0) {
      dest[0] = c >> 12 | 0xe0;
      dest[1] = ((c >> 6) & 0x3f) | 0x80;
      dest[2] = (c & 0x3f) | 0x80;
      dest[3] = '\0';
      return 3;
   } else if (c >> 21 == 0) {
      dest[0] = c >> 18 | 0xf0;
      dest[1] = ((c >> 12) & 0x3f) | 0xe0;
      dest[2] = ((c >> 6) & 0x3f) | 0x80;
      dest[3] = (c & 0x3f) | 0x80;
      dest[4] = '\0';
      return 4;
   }
   // TODO handle invalid UTF-8 sequences
   return 1;
}

static const char* UTF8_backward(const char* ss) {
   unsigned char* c = (unsigned char*) ss;
   c--;
   while (*c >> 6 == 2)
      c--;
   return (const char*) c;
}

const char* UTF8_forward(const char* ss, int n) {
   int i = 0;
   unsigned char* c = (unsigned char*) ss;
   while (*c) {
      if (i == n)
         return (const char*) c;
      c++;
      while (*c >> 6 == 2)
         c++;
      i++;
   }
   return (const char*) c;
}

static int UTF8_offset(const char* ss, const char* ssubstr) {
   if (!ss || !ssubstr) return 0;
   unsigned char* c = (unsigned char*) ss;
   const unsigned char* substr = (const unsigned char*) ssubstr;
   int i = 0;
   while (*c) {
      if (c == substr)
         return i;
      c++;
      while (*c >> 6 == 2)
         c++;
      i++;
   }
   return i;
}

Text Text_new(char* data) {
   Text t;
   t.data = data;
   t.bytes = strlen(data);
   t.dataSize = t.bytes + 1;
   t.chars = UTF8_chars(data);
   return t;
}

Text* Text_replace(Text* t, char* data, int bytes) {
   free(t->data);
   t->data = data;
   t->dataSize = bytes + 1;
   t->bytes = bytes;
   t->chars = UTF8_chars(data);
   return t;
}

Text Text_copy(Text this) {
   Text t;
   t.data = strdup(this.data);
   t.dataSize = this.dataSize;
   t.bytes = this.bytes;
   t.chars = this.chars;
   return t;
}

Text Text_null() {
   Text t;
   t.data = NULL;
   t.dataSize = 0;
   t.bytes = 0;
   t.chars = 0;
   return t;
}

void Text_prune(Text* this) {
   free(this->data);
   this->data = NULL;
   this->dataSize = 0;
   this->bytes = 0;
   this->chars = 0;
}

bool Text_hasChar(Text t, int c) {
   if (!t.data) return false;
   if (c >> 7 == 0)
      return strchr(t.data, c);
   char seq[5];
   UTF8_codePointToString(seq, c);
   return strstr(t.data, seq);
}

wchar_t Text_at(Text t, int n) {
   return UTF8_stringToCodePoint(UTF8_forward(t.data, n));
}

const char* Text_stringAt(Text t, int n) {
   return UTF8_forward(t.data, n);
}

int Text_bytesUntil(Text t, int n) {
   const char* offset = UTF8_forward(t.data, n);
   return offset - t.data;
}

const Text Text_textAt(Text t, int n) {
   Text s;
   s.data = t.data ? (char*) UTF8_forward(t.data, n) : NULL;
   s.bytes = t.bytes - (s.data - t.data);
   s.chars = t.chars - n;
   return s;
}

int Text_forwardWord(Text this, int cursor) {
   const char* s = UTF8_forward(this.data, cursor);
   wchar_t curr = UTF8_stringToCodePoint(s);
   if (iswalnum(curr)) {
      while (iswalnum(curr) && cursor < this.chars) {
         s = UTF8_forward(s, 1);
         curr = UTF8_stringToCodePoint(s);
         cursor++;
      }
   } else if (!iswalnum(curr) && !iswblank(curr)) {
      while (!iswalnum(curr) && !iswblank(curr) && cursor < this.chars) {
         s = UTF8_forward(s, 1);
         curr = UTF8_stringToCodePoint(s);
         cursor++;
      }
   } else {
      while (iswblank(curr) && cursor < this.chars) {
         s = UTF8_forward(s, 1);
         curr = UTF8_stringToCodePoint(s);
         cursor++;
      }
   }
   return cursor;
}

int Text_backwardWord(Text this, int cursor) {
   if (cursor == 0) return 0;
   cursor--;
   const char* s = UTF8_forward(this.data, cursor);
   wchar_t curr = UTF8_stringToCodePoint(s);
   if (iswalnum(curr)) {
      for (; cursor > 0; cursor--) {
         s = UTF8_backward(s);
         curr = UTF8_stringToCodePoint(s);
         if (!iswalnum(curr)) break;
      }
   } else if (!iswalnum(curr) && !iswblank(curr)) {
      for (; cursor > 0; cursor--) {
         s = UTF8_backward(s);
         curr = UTF8_stringToCodePoint(s);
         if (iswalnum(curr) || iswblank(curr)) break;
      }
   } else {
      for (; cursor > 0; cursor--) {
         s = UTF8_backward(s);
         curr = UTF8_stringToCodePoint(s);
         if (!iswblank(curr)) break;
      }
   }
   return cursor;
}

Text Text_wordAt(Text this, int cursor) {
   if (!this.data) {
      return Text_new(strdup(""));
   }
   const char* start = UTF8_forward(this.data, cursor);
   const char* end = start;

   int scursor = cursor;
   while (scursor > 0) {
      const char* prev = UTF8_backward(start);
      if (iswword(UTF8_stringToCodePoint(prev))) {
         start = prev;
         scursor--;
      }
   }
   
   int ecursor = cursor;
   while (ecursor < this.chars && iswword(UTF8_stringToCodePoint(end))) {
      end = UTF8_forward(end, 1);
      ecursor++;
   }
   
   int bytes = end - start + 1;
   char* word = malloc(bytes + 1);
   memcpy(word, start, bytes);
   word[bytes] = '\0';

   Text t;
   t.data = word;
   t.bytes = bytes;
   t.chars = ecursor - scursor + 1;
   return t;
}

int Text_indexOf(Text haystack, Text needle) {
   char* found = strstr(haystack.data, needle.data);
   if (found)
      return UTF8_offset(haystack.data, found);
   return -1;
}

int Text_indexOfi(Text haystack, Text needle) {
   char* found = strcasestr(haystack.data, needle.data);
   if (found)
      return UTF8_offset(haystack.data, found);
   return -1;
}

int Text_strncmp(Text haystack, Text needle) {
   return strncmp(haystack.data, needle.data, needle.bytes);
}

int Text_strncasecmp(Text haystack, Text needle) {
   return strncasecmp(haystack.data, needle.data, needle.bytes);
}

Text* Text_strcat(Text* dest, Text src) {
   int newSize = dest->bytes + src.bytes + 1;
   if (dest->dataSize < newSize) {
      dest->data = realloc(dest->data, newSize);
      dest->dataSize = newSize;
   }
   memcpy(dest->data + dest->bytes, src.data, src.bytes);
   dest->bytes += src.bytes;
   dest->chars += src.chars;
   dest->data[dest->bytes] = '\0';
   return dest;
}

int Text_cellsUntil(Text t, int n, int tabSize) {
   int width = 0;
   n = MIN(n, t.chars);
   int offset = 0;
   for (int i = 0; i < n; i++) {
      char curr = t.data[offset];
      if (curr == '\t') {
         width += tabSize - (width % tabSize);
         offset++;
      } else {
         offset += UTF8_bytes(curr);
         width++;
      }
   }
   return width;
}

void Text_deleteChar(Text* t, int at) {
   if (t->chars == 0 || at >= t->chars)
      return;
   char* s = (char*) UTF8_forward(t->data, at);
   int offset = UTF8_bytes(*s);
   for(; *s; s++) {
      *s = *(s + offset);
   }
   t->bytes -= offset;
   t->chars--;
}

void Text_deleteChars(Text* t, int at, int n) {
   if (t->chars == 0 || at >= t->chars)
      return;
   n = MIN(n, t->chars - at);
   char* s = (char*) UTF8_forward(t->data, at);
   int offset = UTF8_forward(s, n) - s;
   for (;;) {
      *s = *(s + offset);
      if (!*s) break;
      s++;
   }
   t->bytes -= offset;
   t->chars -= n;
}

void Text_clear(Text* t) {
   t->bytes = 0;
   t->chars = 0;
}

static inline void insert(Text* t, int at, const char* data, int bytes, int chars) {
   if (t->bytes + bytes >= t->dataSize) {
      t->dataSize += MAX(bytes, t->dataSize) + 1;
      t->data = realloc(t->data, t->dataSize);
      t->data[t->bytes] = '\0';
   }
   char* s = (char*) UTF8_forward(t->data, at);
   for (int i = t->bytes; t->data + i >= s; i--) {
      t->data[i+bytes] = t->data[i];
   }
   memcpy(s, data, bytes);
   t->bytes += bytes;
   t->chars += chars;
   t->data[t->bytes] = '\0';
}

void Text_insertChar(Text* t, int at, wchar_t ch) {
   assert(at >= 0 && at <= t->chars);
   char seq[5];
   int offset = UTF8_codePointToString(seq, ch);
   insert(t, at, seq, offset, 1);
}

void Text_insert(Text* t, int at, Text new) {
   assert(at >= 0 && at <= t->chars);
   insert(t, at, new.data, new.bytes, new.chars);
}

void Text_insertString(Text* t, int at, const char* s, int bytes) {
   assert(at >= 0 && at <= t->chars);
   insert(t, at, s, bytes, UTF8_offset(s, s + bytes));
}

Text Text_breakIndenting(Text* t, int at, int indent) {
   Text new = Text_null();
   int atBytes = at ? Text_bytesUntil(*t, at) : 0;
   int restBytes = t->bytes - atBytes;

   if (indent)
      Text_insertString(&new, 0, t->data, indent);
   Text_insertString(&new, indent, t->data + atBytes, restBytes);
   t->data[atBytes] = '\0';
   t->chars = at;
   t->bytes = atBytes;
   return new;
}
