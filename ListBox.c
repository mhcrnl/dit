
#include "Prototypes.h"
//#needs Object RichString

#include <math.h>
#include <sys/param.h>
#include <stdbool.h>

#include <curses.h>
//#link ncurses

/*{

typedef enum HandlerResult_ {
   HANDLED,
   IGNORED,
   BREAK_LOOP
} HandlerResult;

typedef HandlerResult(*ListBox_eventHandler)(ListBox*, int);

struct ListBox_ {
   Object super;
   int x, y, w, h;
   WINDOW* window;
   List* items;
   int selected;
   int scrollV, scrollH;
   int oldSelected;
   bool needsRedraw;
   RichString header;
   ListBox_eventHandler eventHandler;
   bool highlightBar;
   int cursorX;
   int displaying;
   int color;
};

extern char* LISTBOX_CLASS;

}*/

/* private property */
char* LISTBOX_CLASS = "ListBox";

ListBox* ListBox_new(int x, int y, int w, int h, int color, char* type, bool owner) {
   ListBox* this = (ListBox*) malloc(sizeof(ListBox));
   ListBox_init(this, x, y, w, h, color, type, owner);
   return this;
}

void ListBox_delete(Object* cast) {
   ListBox* this = (ListBox*)cast;
   ListBox_done(this);
   free(this);
}

void ListBox_init(ListBox* this, int x, int y, int w, int h, int color, char* type, bool owner) {
   Object* super = (Object*) this;
   super->class = LISTBOX_CLASS;
   super->delete = ListBox_delete;
   this->x = x;
   this->y = y;
   this->w = w;
   this->h = h;
   this->color = color;
   this->eventHandler = NULL;
   this->items = List_new(type);
   this->scrollV = 0;
   this->scrollH = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->needsRedraw = true;
   this->highlightBar = false;
   this->cursorX = 0;
   this->displaying = 0;
   RichString_prune(&(this->header));
}

void ListBox_done(ListBox* this) {
   assert (this != NULL);
   RichString_delete(this->header);
   List_delete(this->items);
}

void ListBox_setHeader(ListBox* this, RichString header) {
   assert (this != NULL);

   if (this->header.len > 0) {
      RichString_delete(this->header);
   }
   this->header = header;
   this->needsRedraw = true;
}

void ListBox_move(ListBox* this, int x, int y) {
   assert (this != NULL);

   this->x = x;
   this->y = y;
   this->needsRedraw = true;
}

void ListBox_resize(ListBox* this, int w, int h) {
   assert (this != NULL);

   if (this->header.len > 0)
      h--;
   this->w = w;
   this->h = h;
   this->needsRedraw = true;
}

void ListBox_prune(ListBox* this) {
   assert (this != NULL);

   List_prune(this->items);
   this->scrollV = 0;
   this->selected = 0;
   this->oldSelected = 0;
   this->needsRedraw = true;
}

void ListBox_add(ListBox* this, ListItem* l) {
   assert (this != NULL);

   List_add(this->items, l);
   this->needsRedraw = true;
}

void ListBox_set(ListBox* this, int i, ListItem* l) {
   assert (this != NULL);

   List_set(this->items, i, l);
}

ListItem* ListBox_get(ListBox* this, int i) {
   assert (this != NULL);

   return List_get(this->items, i);
}

Object* ListBox_remove(ListBox* this, int i) {
   assert (this != NULL);

   this->needsRedraw = true;
   Object* removed = NULL;
   List_remove(this->items, i);
   if (this->selected > 0 && this->selected >= List_size(this->items))
      this->selected--;
   return removed;
}

ListItem* ListBox_getSelected(ListBox* this) {
   assert (this != NULL);

   return List_get(this->items, this->selected);
}

/*
void ListBox_moveSelectedUp(ListBox* this) {
   assert (this != NULL);

   List_moveUp(this->items, this->selected);
   if (this->selected > 0)
      this->selected--;
}

void ListBox_moveSelectedDown(ListBox* this) {
   assert (this != NULL);

   List_moveDown(this->items, this->selected);
   if (this->selected + 1 < List_size(this->items))
      this->selected++;
}
*/

int ListBox_getSelectedIndex(ListBox* this) {
   assert (this != NULL);

   return this->selected;
}

int ListBox_size(ListBox* this) {
   assert (this != NULL);

   return List_size(this->items);
}

void ListBox_setSelected(ListBox* this, int selected) {
   assert (this != NULL);

   selected = MAX(0, MIN(List_size(this->items) - 1, selected));
   this->selected = selected;
}

void ListBox_draw(ListBox* this, bool focus) {
   assert (this != NULL);

   int cursorY = 0;
   int first, last;
   int itemCount = List_size(this->items);
   int scrollH = this->scrollH;
   int y = this->y; int x = this->x;
   first = this->scrollV;

   if (this->h > itemCount) {
      last = itemCount;
   } else {
      last = MIN(itemCount, this->scrollV + this->h);
   }
   if (this->selected < first) {
      first = this->selected;
      this->scrollV = first;
      this->needsRedraw = true;
   }
   if (this->selected >= last) {
      last = MIN(itemCount, this->selected + 1);
      first = MAX(0, last - this->h);
      this->scrollV = first;
      this->needsRedraw = true;
   }
   assert(first >= 0);
   assert(last <= itemCount);

   if (this->header.len > 0) {
      int attr = HEADER_PAIR;
      attron(attr);
      mvhline(y, x, ' ', this->w);
      attroff(attr);
      if (scrollH < this->header.len) {
         assert(this->header.len > 0);
         mvaddchnstr(y, x, this->header.chstr + scrollH,
                     MIN(this->header.len - scrollH, this->w));
      }
      y++;
   }

   scrollH = 0;
   
   int highlight;
   if (focus) {
      highlight = FOCUS_HIGHLIGHT_PAIR;
   } else {
      highlight = UNFOCUS_HIGHLIGHT_PAIR;
   }

   attrset(this->color);
   if (this->needsRedraw) {
      for(int i = first, j = 0; j < this->h && i < last; i++, j++) {
         Object* itemObj = (Object*) List_get(this->items, i);
         assert(itemObj);
         RichString itemRef; RichString_init(&itemRef);
         this->displaying = i;
         itemObj->display(itemObj, &itemRef);
         int amt = MIN(itemRef.len - scrollH, this->w);
         if (i == this->selected) {
            if (this->highlightBar) {
               attron(highlight);
               RichString_setAttr(&itemRef, highlight);
            }
            cursorY = y + j;
            mvhline(cursorY, x+amt, ' ', this->w-amt);
            if (amt > 0)
               mvaddchnstr(y+j, x+0, itemRef.chstr + scrollH, amt);
            if (this->highlightBar)
               attroff(highlight);
         } else {
            mvhline(y+j, x+amt, ' ', this->w-amt);
            if (amt > 0)
               mvaddchnstr(y+j, x+0, itemRef.chstr + scrollH, amt);
         }
      }
      for (int i = y + (last - first); i < y + this->h; i++)
         mvhline(i, x+0, ' ', this->w);
      this->needsRedraw = false;

   } else {
      Object* oldObj = (Object*) List_get(this->items, this->oldSelected);
      RichString oldRef; RichString_init(&oldRef);
      this->displaying = this->oldSelected;
      oldObj->display(oldObj, &oldRef);
      Object* newObj = (Object*) List_get(this->items, this->selected);
      RichString newRef; RichString_init(&newRef);
      this->displaying = this->selected;
      newObj->display(newObj, &newRef);
      mvhline(y+ this->oldSelected - this->scrollV, x+0, ' ', this->w);
      if (scrollH < oldRef.len)
         mvaddchnstr(y+ this->oldSelected - this->scrollV, x+0, oldRef.chstr + scrollH, MIN(oldRef.len - scrollH, this->w));
      if (this->highlightBar)
         attron(highlight);
      cursorY = y+this->selected - this->scrollV;
      mvhline(cursorY, x+0, ' ', this->w);
      if (this->highlightBar)
         RichString_setAttr(&newRef, highlight);
      if (scrollH < newRef.len)
         mvaddchnstr(y+this->selected - this->scrollV, x+0, newRef.chstr + scrollH, MIN(newRef.len - scrollH, this->w));
      if (this->highlightBar)
         attroff(highlight);
   }
 //   attroff(this->color);
   this->oldSelected = this->selected;
   move(cursorY, this->cursorX);
}

bool ListBox_onKey(ListBox* this, int key) {
   assert (this != NULL);
   switch (key) {
   case KEY_DOWN:
      if (this->selected + 1 < List_size(this->items))
         this->selected++;
      return true;
   case KEY_UP:
      if (this->selected > 0)
         this->selected--;
      return true;
   case KEY_C_DOWN:
      if (this->selected + 1 < List_size(this->items)) {
         this->selected++;
         if (this->scrollV < List_size(this->items) - this->h) {
            this->scrollV++;
            this->needsRedraw = true;
         }
      }
      return true;
   case KEY_C_UP:
      if (this->selected > 0) {
         this->selected--;
         if (this->scrollV > 0) {
            this->scrollV--;
            this->needsRedraw = true;
         }
      }
      return true;
   case KEY_LEFT:
      if (this->scrollH > 0) {
         this->scrollH -= 5;
         this->needsRedraw = true;
      }
      return true;
   case KEY_RIGHT:
      this->scrollH += 5;
      this->needsRedraw = true;
      return true;
   case KEY_PPAGE:
      this->selected -= (this->h - 1);
      this->scrollV -= (this->h - 1);
      if (this->selected < 0)
         this->selected = 0;
      if (this->scrollV < 0)
         this->scrollV = 0;
      this->needsRedraw = true;
      return true;
   case KEY_NPAGE:
      this->selected += (this->h - 1);
      int size = List_size(this->items);
      if (this->selected >= size)
         this->selected = size - 1;
      this->scrollV += (this->h - 1);
      if (this->scrollV >= MAX(0, size - this->h))
         this->scrollV = MAX(0, size - this->h - 1);
      this->needsRedraw = true;
      return true;
   case KEY_HOME:
      this->selected = 0;
      return true;
   case KEY_END:
      this->selected = List_size(this->items) - 1;
      return true;
   }
   return false;
}