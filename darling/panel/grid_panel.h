#ifndef DARLING_GRID_PANEL_H
#define DARLING_GRID_PANEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// darling/panel/grid_panel.h — excel core (fixed or auto rows x cols of
// Panel* cell slots; empty cell = nullptr, skipped in layout).
// Cells live ONLY in the row-major slot array below, never in the embedded
// base's child list. Detach-only: overwriting or clearing a slot drops the
// old pointer without freeing it. The grid auto-grows on setCell past the
// edge from either constructor; GridPanel_2 presets the initial extent.
// Selection/editing lives above (a GridView controller later), not here.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef ID_GRID_PANEL
#define ID_GRID_PANEL 0x006Cu
#endif
#ifndef TYPE_GRID_PANEL_SINGLETON
#define TYPE_GRID_PANEL_SINGLETON (FORM_SINGLETON | ID_GRID_PANEL)
#endif

typedef struct GridPanel {
    Panel base;
    Panel **cells;
    float *rowHeights;
    int32_t rows;
    int32_t cols;
    float gapX;
    float gapY;
    int32_t headerRows;
    int32_t headerCols;
} GridPanel;

// Constructors:
//   GridPanel()            — empty auto-grow grid (0 x 0)
//   GridPanel(rows, cols)  — preset extent, all cells empty, heights auto
GridPanel *GridPanel_0(void);
GridPanel *GridPanel_2(int32_t rows, int32_t cols);

#define GridPanel(...) CONSTRUCTOR_DISPATCH(GridPanel, __VA_ARGS__)

// Layout facade (same pattern as Panel_* shims: forward over the prefix).
static inline void GridPanel_setLocation(GridPanel *g, float x, float y)
    { if (g) Panel_setLocation(&(*g).base, x, y); }
static inline void GridPanel_setSize(GridPanel *g, float w, float h)
    { if (g) Panel_setSize(&(*g).base, w, h); }

// Core (cells are slots: nullptr clears; overwrite drops without freeing;
// every mutation re-runs the layout pass).
void GridPanel_setCell(GridPanel *g, int32_t row, int32_t col, Panel *cell);
Panel *GridPanel_getCell(const GridPanel *g, int32_t row, int32_t col);
void GridPanel_layout(GridPanel *g);

// Setters.
void GridPanel_setGap(GridPanel *g, float gx, float gy);
void GridPanel_setHeaderRows(GridPanel *g, int32_t count);
void GridPanel_setHeaderCols(GridPanel *g, int32_t count);
void GridPanel_setRowHeight(GridPanel *g, int32_t row, float h);

// Getters.
void GridPanel_getGap(const GridPanel *g, float *gx, float *gy);
int32_t GridPanel_getHeaderRows(const GridPanel *g);
int32_t GridPanel_getHeaderCols(const GridPanel *g);
float GridPanel_getRowHeight(const GridPanel *g, int32_t row);
int32_t GridPanel_rowCount(const GridPanel *g);
int32_t GridPanel_colCount(const GridPanel *g);

#endif
