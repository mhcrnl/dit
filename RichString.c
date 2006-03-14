
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <sys/param.h>

#include "Prototypes.h"

/*{

#define RICHSTRING_SIZE 300

struct RichString_ {
   int len;
   chtype chstr[RICHSTRING_SIZE+1];
};

}*/

void RichString_init(RichString* this) {
   this->len = 0;
}

void RichString_delete(RichString this) {
}

void RichString_prune(RichString* this) {
   this->len = 0;
}

void RichString_write(RichString* this, int attrs, unsigned char* data) {
   this->len = 0;
   RichString_append(this, attrs, data, strlen((char*)data));
}

inline void RichString_append(RichString* this, int attrs, unsigned char* data, int len) {
   // TODO: 8-bit clean
   // int len = strlen((char*)data);
   int maxToWrite = (RICHSTRING_SIZE - 1) - this->len;
   int wrote = MIN(maxToWrite, len);
   for (int i = 0; i < wrote; i++) {
      this->chstr[this->len + i] = data[i] | attrs;
   }
   this->len += wrote;
   this->chstr[this->len] = 0;
}

inline void RichString_appendChar(RichString* this, int attrs, char data) {
   this->chstr[this->len++] = data | attrs;
   this->chstr[this->len] = 0;
}

void RichString_setAttr(RichString *this, int attrs) {
   for (int i = 0; i < this->len; i++) {
      unsigned char c = this->chstr[i] & 0xff;
      this->chstr[i] = c | attrs;
   }
}

void RichString_setAttrs(RichString *this, int* attrs) {
   for (int i = 0; i < this->len; i++) {
      unsigned char c = this->chstr[i];
      this->chstr[i] = c | attrs[i];
   }
}

RichString RichString_quickString(int attrs, unsigned char* data) {
   RichString str; RichString_init(&str);
   RichString_write(&str, attrs, data);
   return str;
}
