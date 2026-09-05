#ifndef DARLING_FIELD_SELECT_H
#define DARLING_FIELD_SELECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "struct/list.h"

// Dropdown select: Panel layout plus an owned item list and popup state.
typedef void (*Select_SelectFn)(void *ctx);

typedef struct Select {
    Panel base;
    List *items;
    int32_t selected;
    int32_t open;
    char *filter;
    Select_SelectFn onSelect;
    void *ctx;
} Select;

Select *Select_0(void);
Select *Select_1(Panel *parent);

#define Select(...) CONSTRUCTOR_DISPATCH(Select, __VA_ARGS__)

// Core list edits (stubs: owned-string growth lands with the caret walker).
void Select_addItem(Select *sel, const char *item);
void Select_clear(Select *sel);

void Select_free(Select *sel);

void Select_setSelected(Select *sel, int32_t index);
void Select_setOpen(Select *sel, int32_t open);
void Select_setFilter(Select *sel, const char *filter);
void Select_setOnSelect(Select *sel, Select_SelectFn fn);
void Select_setCtx(Select *sel, void *ctx);

int32_t Select_getSelected(const Select *sel);
int32_t Select_getOpen(const Select *sel);
const char *Select_getFilter(const Select *sel);
Select_SelectFn Select_getOnSelect(const Select *sel);
void *Select_getCtx(const Select *sel);
size_t Select_itemCount(const Select *sel);
const char *Select_getItem(const Select *sel, size_t index);

#endif
