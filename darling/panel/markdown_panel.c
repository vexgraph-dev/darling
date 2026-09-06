#include "darling/panel/markdown_panel.h"

#include "darling/label/label.h"
#include "darling/label/rich_label.h"
#include "darling/panel/panel.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "primitive/string.h"
#include "text/rich_text.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: MarkdownPanel (embeds Panel)
 * LEVEL: L2 — Behavior (markdown-fed document panel behavior API)
 * ============================================================================
 * Takes a markdown string, scans it with a zero-alloc line walker over the
 * vexspoke primitive/string block (index arithmetic only — no malloc in the
 * scan path), and builds an owned row list of Label/RichLabel children
 * inside an inner Panel container. v1 syntax: hash headings, dash/star
 * bullets, backtick inline code, fenced blocks, bold and italic spans via
 * RichText styles. Rebuild is detach-all + re-layout (cold path); row payloads use
 * Memory_alloc like the rest of the tree.
 *
 * Inline markup becomes RichLabel rows only when a Font is set (RichText
 * layout needs a font for quads); without one those lines fall back to
 * plain Labels with markers stripped. RichText span tags use two-digit
 * style ids ([00]/[01]/...) because the RichText parser reserves
 * single-char tokens for n/l/c/r/j. Rows live in a vertical ListPanel,
 * which owns Y positions; stackRow sizes rows and advances the height
 * cursor for the panel's own height accounting.
 *
 * STRUCT FIELDS (Mirroring darling/panel/markdown_panel.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                        // Inherited layout/tree/background state
 *   uint8_t *textBlock;                // Owned vexspoke string block (source)
 *   Font *font;                        // Aliased font for RichText rows; nullable
 *   uint32_t codeBackground;           // Fill color behind fenced code rows
 *   ListPanel *rows;                   // Owned vertical ListPanel of rows
 *   float rowSpacing;                  // Vertical gap between stacked rows
 *   struct MarkdownRowSlot *slots;     // Owned row records (see SLOT RECORD)
 *   size_t rowCount;                   // Active row record count
 *   size_t rowCapacity;                // Row record capacity
 *
 * SLOT RECORD (file-local, behaviorless; all behavior hangs off MarkdownPanel):
 * ----------------------------------------------------------------------------
 *   struct MarkdownRowSlot {
 *     Panel *panel;                    // Row base (Label* or RichLabel* payload)
 *     RichText *model;                 // Owned RichText model, or nullptr for Labels
 *     uint8_t isRich;                  // 1 = RichLabel row, 0 = Label row
 *   }
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - MarkdownPanel_0(void)
 *   - MarkdownPanel_1(text)
 *
 * Core Functions:
 *   - MarkdownPanel_free(s)
 *
 * Setters:
 *   - MarkdownPanel_setText(s, text)
 *   - MarkdownPanel_setFont(s, font)
 *   - MarkdownPanel_setCodeBackground(s, color)
 *   - MarkdownPanel_setRowSpacing(s, spacing)
 *   - MarkdownPanel_setLocation(s, x, y)
 *   - MarkdownPanel_setSize(s, w, h)
 *   - MarkdownPanel_setBackgroundColor(s, color)
 *
 * Getters:
 *   - MarkdownPanel_getText(s)
 *   - MarkdownPanel_getFont(s)
 *   - MarkdownPanel_getCodeBackground(s)
 *   - MarkdownPanel_getRows(s)
 *   - MarkdownPanel_getRowSpacing(s)
 *   - MarkdownPanel_getRowCount(s)
 *   - MarkdownPanel_getRow(s, index)
 * ============================================================================
 */

struct MarkdownRowSlot {
    Panel *panel;
    RichText *model;
    uint8_t isRich;
};

#define MARKDOWN_BASE_SIZE 14.0f
#define MARKDOWN_H1_SIZE 28.0f
#define MARKDOWN_H2_SIZE 22.0f
#define MARKDOWN_H3_SIZE 17.0f
#define MARKDOWN_CODE_SIZE 12.0f
#define MARKDOWN_LINE_FACTOR 1.35f
#define MARKDOWN_TEXT_COLOR 0xFFFFFFFFu
#define MARKDOWN_CODE_BACKGROUND 0xFF222222u
#define MARKDOWN_DEFAULT_SPACING 4.0f

typedef enum {
    LINE_BLANK,
    LINE_FENCE,
    LINE_HEADING,
    LINE_BULLET,
    LINE_PARA
} LineKind;

// CONSTRUCTORS
// ============================================================================

MarkdownPanel *MarkdownPanel_0(void) {
    MarkdownPanel *s = (MarkdownPanel*) Memory_alloc(TYPE_MARKDOWN_PANEL_SINGLETON, sizeof(MarkdownPanel));
    if (!s)
        return nullptr;
    Panel *b = Panel_0();
    if (!b) {
        Memory_free(s);
        return nullptr;
    }
    (*s).base = (*b);
    Memory_free(b);
    ListPanel *box = ListPanel_0();
    if (!box) {
        Memory_free(s);
        return nullptr;
    }
    (*s).textBlock = nullptr;
    (*s).font = nullptr;
    (*s).codeBackground = MARKDOWN_CODE_BACKGROUND;
    (*s).rows = box;
    (*s).rowSpacing = MARKDOWN_DEFAULT_SPACING;
    ListPanel_setSpacing(box, MARKDOWN_DEFAULT_SPACING);
    (*s).slots = nullptr;
    (*s).rowCount = 0;
    (*s).rowCapacity = 0;
    Panel *self = &(*s).base;
    Panel_addContainer(self, &(*box).base);
    return s;
}

MarkdownPanel *MarkdownPanel_1(const char *text) {
    MarkdownPanel *s = MarkdownPanel_0();
    if (s)
        MarkdownPanel_setText(s, text);
    return s;
}

// CORE FUNCTIONS
// ============================================================================

static void markDirty(MarkdownPanel *s) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    Container *c = &(*b).base;
    Container_markDirty(c);
}

static bool pushSlot(MarkdownPanel *s, Panel *row, RichText *model, uint8_t isRich) {
    if ((*s).rowCount >= (*s).rowCapacity) {
        size_t grown = (*s).rowCapacity == 0 ? 8 : (*s).rowCapacity * 2;
        struct MarkdownRowSlot *next = (struct MarkdownRowSlot*) Memory_realloc((*s).slots, grown * sizeof(struct MarkdownRowSlot));
        if (!next)
            return false;
        (*s).slots = next;
        (*s).rowCapacity = grown;
    }
    struct MarkdownRowSlot *slots = (*s).slots;
    size_t at = (*s).rowCount;
    slots[at].panel = row;
    slots[at].model = model;
    slots[at].isRich = isRich;
    (*s).rowCount = at + 1;
    return true;
}

static void clearRows(MarkdownPanel *s) {
    if (!s)
        return;
    ListPanel *box = (*s).rows;
    if (box) {
        while (ListPanel_count(box) > 0)
            ListPanel_remove(box, 0);
    }
    struct MarkdownRowSlot *slots = (*s).slots;
    size_t n = (*s).rowCount;
    for (size_t i = 0; i < n; i++) {
        struct MarkdownRowSlot *slot = &slots[i];
        RichText *model = (*slot).model;
        Panel *row = (*slot).panel;
        uint8_t rich = (*slot).isRich;
        if (model)
            RichText_free(model);
        if (row) {
            if (rich)
                Memory_free(row);
            else {
                Label *lbl = (Label*) row;
                Label_free(lbl);
            }
        }
    }
    if (slots)
        Memory_free(slots);
    (*s).slots = nullptr;
    (*s).rowCount = 0;
    (*s).rowCapacity = 0;
}

// Zero-alloc line classifier: index arithmetic over (line, len) only.
static LineKind classifyLine(const char *line, size_t len, size_t *contentStart, size_t *contentLen, float *size) {
    size_t i = 0;
    while (i < len && (line[i] == ' ' || line[i] == '\t'))
        i++;
    if (i >= len) {
        (*contentStart) = len;
        (*contentLen) = 0;
        (*size) = MARKDOWN_BASE_SIZE;
        return LINE_BLANK;
    }
    if (len - i >= 3 && line[i] == '`' && line[i + 1] == '`' && line[i + 2] == '`') {
        (*contentStart) = len;
        (*contentLen) = 0;
        (*size) = MARKDOWN_CODE_SIZE;
        return LINE_FENCE;
    }
    if (line[i] == '#') {
        size_t hashes = 0;
        while (i + hashes < len && line[i + hashes] == '#' && hashes < 3)
            hashes++;
        size_t after = i + hashes;
        if (after >= len || line[after] == ' ' || line[after] == '\t') {
            size_t start = after;
            while (start < len && (line[start] == ' ' || line[start] == '\t'))
                start++;
            (*contentStart) = start;
            (*contentLen) = len - start;
            if (hashes == 1)
                (*size) = MARKDOWN_H1_SIZE;
            else if (hashes == 2)
                (*size) = MARKDOWN_H2_SIZE;
            else
                (*size) = MARKDOWN_H3_SIZE;
            return LINE_HEADING;
        }
    }
    if ((line[i] == '-' || line[i] == '*') && i + 1 < len && line[i + 1] == ' ') {
        size_t start = i + 2;
        while (start < len && (line[start] == ' ' || line[start] == '\t'))
            start++;
        (*contentStart) = start;
        (*contentLen) = len - start;
        (*size) = MARKDOWN_BASE_SIZE;
        return LINE_BULLET;
    }
    (*contentStart) = i;
    (*contentLen) = len - i;
    (*size) = MARKDOWN_BASE_SIZE;
    return LINE_PARA;
}

// Zero-alloc inline-markup probe: true when a converter pass is worthwhile.
static bool hasInline(const char *line, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char ch = line[i];
        if (ch == '`')
            return true;
        if (ch == '*' && i + 1 < len && line[i + 1] == '*')
            return true;
        if (ch == '*' && i > 0 && i + 1 < len)
            return true;
    }
    return false;
}

// Emits the combined style tag for the active set: [00] reset + [01|02|03].
static void emitActive(char *dest, size_t *at, bool bold, bool italic, bool code) {
    const char *reset = "[00]";
    size_t k = 0;
    while (reset[k] != '\0') {
        dest[(*at)] = reset[k];
        (*at)++;
        k++;
    }
    if (!bold && !italic && !code)
        return;
    dest[(*at)] = '[';
    (*at)++;
    bool first = true;
    if (bold) {
        dest[(*at)] = '0';
        (*at)++;
        dest[(*at)] = '1';
        (*at)++;
        first = false;
    }
    if (italic) {
        if (!first) {
            dest[(*at)] = '|';
            (*at)++;
        }
        dest[(*at)] = '0';
        (*at)++;
        dest[(*at)] = '2';
        (*at)++;
        first = false;
    }
    if (code) {
        if (!first) {
            dest[(*at)] = '|';
            (*at)++;
        }
        dest[(*at)] = '0';
        (*at)++;
        dest[(*at)] = '3';
        (*at)++;
    }
    dest[(*at)] = ']';
    (*at)++;
}

// Measures the tagged conversion length (counting pass, no output).
static size_t measureTagged(const char *line, size_t len) {
    size_t need = len + 1;
    for (size_t i = 0; i < len; i++) {
        char ch = line[i];
        if (ch == '[')
            need += 1;
        if (ch == '*' && i + 1 < len && line[i + 1] == '*') {
            need += 4;
            i++;
        } else if (ch == '*' || ch == '`') {
            need += 4;
        }
    }
    need += 16;
    return need;
}

// Fills dest with the RichText-tagged conversion; dest has measureTagged bytes.
static void fillTagged(const char *line, size_t len, char *dest) {
    size_t at = 0;
    bool bold = false;
    bool italic = false;
    bool code = false;
    for (size_t i = 0; i < len;) {
        char ch = line[i];
        if (ch == '*' && i + 1 < len && line[i + 1] == '*') {
            bold = !bold;
            emitActive(dest, &at, bold, italic, code);
            i += 2;
        } else if (ch == '*') {
            italic = !italic;
            emitActive(dest, &at, bold, italic, code);
            i++;
        } else if (ch == '`') {
            code = !code;
            emitActive(dest, &at, bold, italic, code);
            i++;
        } else {
            if (ch == '[') {
                dest[at] = '\\';
                at++;
            }
            dest[at] = ch;
            at++;
            i++;
        }
    }
    if (bold || italic || code)
        emitActive(dest, &at, false, false, false);
    dest[at] = '\0';
}

// Copies a line slice, stripping markdown markers for the plain Label path.
static void fillStripped(const char *line, size_t len, char *dest) {
    size_t at = 0;
    for (size_t i = 0; i < len;) {
        char ch = line[i];
        if (ch == '*' && i + 1 < len && line[i + 1] == '*') {
            i += 2;
        } else if (ch == '*' || ch == '`') {
            i++;
        } else {
            dest[at] = ch;
            at++;
            i++;
        }
    }
    dest[at] = '\0';
}

static float stackRow(MarkdownPanel *s, Panel *row, float cursor, float height) {
    // Y positions belong to the ListPanel now; rows only take their size.
    Panel *b = &(*s).base;
    Container *c = &(*b).base;
    float w = Container_getWidth(c);
    if (w < 0.0f)
        w = 0.0f;
    Panel_setSize(row, w, height);
    return cursor + height + (*s).rowSpacing;
}

static void addLabelRow(MarkdownPanel *s, const char *line, size_t len, bool bullet, float size, uint32_t color, uint32_t bg, float *cursor) {
    ListPanel *box = (*s).rows;
    if (!box || !cursor)
        return;
    size_t extra = bullet ? 4 : 0;
    char *tmp = (char*) Memory_alloc(TYPE_ARRAY, len + extra + 1);
    if (!tmp)
        return;
    size_t at = 0;
    if (bullet) {
        tmp[0] = 0xE2;
        tmp[1] = 0x80;
        tmp[2] = 0xA2;
        tmp[3] = ' ';
        at = 4;
    }
    for (size_t i = 0; i < len; i++) {
        tmp[at] = line[i];
        at++;
    }
    tmp[at] = '\0';
    Label *lbl = Label_1((const char*) tmp);
    Memory_free(tmp);
    if (!lbl)
        return;
    Label_setFontSize(lbl, size);
    Label_setTextColor(lbl, color);
    if (bg != 0u)
        Label_setBackgroundColor(lbl, bg);
    Panel *row = &(*lbl).base;
    ListPanel_add(box, row);
    if (!pushSlot(s, row, nullptr, 0)) {
        size_t n = ListPanel_count(box);
        if (n > 0)
            ListPanel_remove(box, (int32_t)(n - 1));
        Label_free(lbl);
        return;
    }
    (*cursor) = stackRow(s, row, (*cursor), size * MARKDOWN_LINE_FACTOR);
}

static void addRichRow(MarkdownPanel *s, const char *line, size_t len, bool bullet, float *cursor) {
    ListPanel *box = (*s).rows;
    Font *font = (*s).font;
    if (!box || !font || !cursor)
        return;
    size_t rawLen = len + (bullet ? 2 : 0);
    char *raw = (char*) Memory_alloc(TYPE_ARRAY, rawLen + 1);
    if (!raw)
        return;
    size_t at = 0;
    if (bullet) {
        raw[0] = 0xE2;
        raw[1] = 0x80;
        raw[2] = 0xA2;
        raw[3] = ' ';
        at = 4;
    }
    for (size_t i = 0; i < len; i++) {
        raw[at] = line[i];
        at++;
    }
    raw[at] = '\0';
    size_t need = measureTagged(raw, at);
    char *tagged = (char*) Memory_alloc(TYPE_ARRAY, need);
    if (!tagged) {
        Memory_free(raw);
        return;
    }
    fillTagged(raw, at, tagged);
    Memory_free(raw);
    RichText *rt = RichText_new();
    if (!rt) {
        Memory_free(tagged);
        return;
    }
    RichText_setStyle(rt, 0, font, MARKDOWN_BASE_SIZE, MARKDOWN_TEXT_COLOR, false, DECOR_NONE);
    RichText_setStyle(rt, 1, font, MARKDOWN_BASE_SIZE, MARKDOWN_TEXT_COLOR, true, DECOR_NONE);
    RichText_setStyle(rt, 2, font, MARKDOWN_BASE_SIZE, MARKDOWN_TEXT_COLOR, false, DECOR_LINE);
    RichText_setStyle(rt, 3, font, MARKDOWN_CODE_SIZE, MARKDOWN_TEXT_COLOR, false, DECOR_NONE);
    RichText_setString(rt, tagged);
    Memory_free(tagged);
    RichText_layout(rt, 0.0f);
    RichLabel *rl = RichLabel_0();
    if (!rl) {
        RichText_free(rt);
        return;
    }
    RichLabel_setTextModel(rl, rt);
    Panel *row = &(*rl).base;
    ListPanel_add(box, row);
    if (!pushSlot(s, row, rt, 1)) {
        size_t n = ListPanel_count(box);
        if (n > 0)
            ListPanel_remove(box, (int32_t)(n - 1));
        RichText_free(rt);
        Memory_free(rl);
        return;
    }
    float height = (*rt).layoutHeight;
    if (height <= 0.0f)
        height = MARKDOWN_BASE_SIZE * MARKDOWN_LINE_FACTOR;
    (*cursor) = stackRow(s, row, (*cursor), height);
}

static void rebuild(MarkdownPanel *s) {
    if (!s)
        return;
    clearRows(s);
    ListPanel *box = (*s).rows;
    if (!box)
        return;
    uint8_t *block = (*s).textBlock;
    const char *src = block ? string_get(block) : nullptr;
    size_t total = block ? string_length(block) : 0;
    if (!src || total == 0) {
        Panel_setSize(&(*box).base, 0.0f, 0.0f);
        markDirty(s);
        return;
    }
    float cursor = 0.0f;
    bool inFence = false;
    size_t start = 0;
    for (size_t i = 0; i <= total; i++) {
        bool edge = i == total || src[i] == '\n';
        if (!edge)
            continue;
        size_t len = i - start;
        const char *line = &src[start];
        start = i + 1;
        size_t contentStart = 0;
        size_t contentLen = 0;
        float size = MARKDOWN_BASE_SIZE;
        LineKind kind = classifyLine(line, len, &contentStart, &contentLen, &size);
        if (kind == LINE_FENCE) {
            inFence = !inFence;
            continue;
        }
        if (kind == LINE_BLANK)
            continue;
        if (inFence) {
            addLabelRow(s, line, len, false, MARKDOWN_CODE_SIZE, MARKDOWN_TEXT_COLOR, (*s).codeBackground, &cursor);
            continue;
        }
        const char *content = &line[contentStart];
        if (kind == LINE_HEADING) {
            addLabelRow(s, content, contentLen, false, size, MARKDOWN_TEXT_COLOR, 0u, &cursor);
        } else if (kind == LINE_BULLET) {
            Font *font = (*s).font;
            if (font && hasInline(content, contentLen))
                addRichRow(s, content, contentLen, true, &cursor);
            else {
                size_t stripped = contentLen + 1;
                char *tmp = (char*) Memory_alloc(TYPE_ARRAY, stripped);
                if (tmp) {
                    fillStripped(content, contentLen, tmp);
                    addLabelRow(s, tmp, strlen(tmp), true, size, MARKDOWN_TEXT_COLOR, 0u, &cursor);
                    Memory_free(tmp);
                }
            }
        } else {
            Font *font = (*s).font;
            if (font && hasInline(content, contentLen)) {
                addRichRow(s, content, contentLen, false, &cursor);
            } else {
                size_t stripped = contentLen + 1;
                char *tmp = (char*) Memory_alloc(TYPE_ARRAY, stripped);
                if (tmp) {
                    fillStripped(content, contentLen, tmp);
                    addLabelRow(s, tmp, strlen(tmp), false, size, MARKDOWN_TEXT_COLOR, 0u, &cursor);
                    Memory_free(tmp);
                }
            }
        }
    }
    if (cursor > 0.0f)
        cursor -= (*s).rowSpacing;
    Panel *b = &(*s).base;
    Container *c = &(*b).base;
    float w = Container_getWidth(c);
    if (w < 0.0f)
        w = 0.0f;
    Panel_setSize(&(*box).base, w, cursor);
    // Authoritative pass: rows were sized after their per-add layouts ran,
    // so re-stack once with final heights (cold path, documents only).
    ListPanel_layout(box);
    markDirty(s);
}

void MarkdownPanel_free(MarkdownPanel *s) {
    if (!s)
        return;
    clearRows(s);
    if ((*s).textBlock) {
        string_free((*s).textBlock);
        (*s).textBlock = nullptr;
    }
    if ((*s).rows) {
        Memory_free((*s).rows);
        (*s).rows = nullptr;
    }
    (*s).font = nullptr;
    Memory_free(s);
}

// SETTERS
// ============================================================================

void MarkdownPanel_setText(MarkdownPanel *s, const char *text) {
    if (!s)
        return;
    if ((*s).textBlock) {
        string_free((*s).textBlock);
        (*s).textBlock = nullptr;
    }
    if (text)
        (*s).textBlock = string_allocate(text);
    rebuild(s);
}

void MarkdownPanel_setFont(MarkdownPanel *s, Font *font) {
    if (!s)
        return;
    (*s).font = font;
    rebuild(s);
}

void MarkdownPanel_setCodeBackground(MarkdownPanel *s, uint32_t color) {
    if (!s)
        return;
    (*s).codeBackground = color;
    rebuild(s);
}

void MarkdownPanel_setRowSpacing(MarkdownPanel *s, float spacing) {
    if (!s)
        return;
    if (spacing < 0.0f)
        spacing = 0.0f;
    (*s).rowSpacing = spacing;
    if ((*s).rows)
        ListPanel_setSpacing((*s).rows, spacing);
    rebuild(s);
}

void MarkdownPanel_setLocation(MarkdownPanel *s, float x, float y) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    Panel_setLocation(b, x, y);
}

void MarkdownPanel_setSize(MarkdownPanel *s, float w, float h) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    Panel_setSize(b, w, h);
}

void MarkdownPanel_setBackgroundColor(MarkdownPanel *s, uint32_t color) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    Panel_setBackgroundColor(b, color);
}

// GETTERS
// ============================================================================

const char *MarkdownPanel_getText(const MarkdownPanel *s) {
    if (!s || !(*s).textBlock)
        return nullptr;
    uint8_t *block = (*s).textBlock;
    return string_get(block);
}

Font *MarkdownPanel_getFont(const MarkdownPanel *s) {
    return s ? (*s).font : nullptr;
}

uint32_t MarkdownPanel_getCodeBackground(const MarkdownPanel *s) {
    return s ? (*s).codeBackground : 0u;
}

Panel *MarkdownPanel_getRows(const MarkdownPanel *s) {
    ListPanel *rows = s ? (*s).rows : nullptr;
    return rows ? &(*rows).base : nullptr;
}

float MarkdownPanel_getRowSpacing(const MarkdownPanel *s) {
    return s ? (*s).rowSpacing : 0.0f;
}

size_t MarkdownPanel_getRowCount(const MarkdownPanel *s) {
    return s ? (*s).rowCount : 0;
}

Panel *MarkdownPanel_getRow(const MarkdownPanel *s, size_t index) {
    if (!s)
        return nullptr;
    if (index >= (*s).rowCount)
        return nullptr;
    struct MarkdownRowSlot *slots = (*s).slots;
    struct MarkdownRowSlot *slot = &slots[index];
    return (*slot).panel;
}
