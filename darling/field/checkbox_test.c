#include "annotation/overview.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/field/checkbox.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: CheckboxTest (darling/field/checkbox_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Checkbox live click controls (Pkg 2): pointer up toggle,
 * inside/outside bounds, indeterminate clearing, callback firing.
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static void onCheckboxChange(void *ctx) {
    (*(bool*) ctx) = true;
}

int main(void) {
    printf("=== Running Checkbox Test Suite ===\n");

    // §1 Null safety
    Checkbox_handlePointer(nullptr, PTR_DOWN, 5.0f, 5.0f);
    Checkbox_handlePointer(nullptr, PTR_UP, 5.0f, 5.0f);
    Checkbox_toggle(nullptr);

    // §2 Instantiate widget + set callback with context pointer
    Checkbox *c = Checkbox_0();
    assert(c != nullptr);
    assert(Checkbox_isChecked(c) == false);

    bool fired = false;
    Checkbox_setOnChange(c, onCheckboxChange);
    Checkbox_setCtx(c, &fired);

    // §3 Call _handlePointer with PTR_DOWN then PTR_UP inside bounds
    // Default bounds are 20x20 when unset
    Checkbox_handlePointer(c, PTR_DOWN, 10.0f, 10.0f);
    assert(fired == false);
    assert(Checkbox_isChecked(c) == false);

    Checkbox_handlePointer(c, PTR_UP, 10.0f, 10.0f);
    assert(fired == true);
    assert(Checkbox_isChecked(c) == true);

    // §4 Call _handlePointer with PTR_UP outside bounds and assert callback did NOT fire
    fired = false;
    Checkbox_handlePointer(c, PTR_UP, 50.0f, 50.0f);
    assert(fired == false);
    assert(Checkbox_isChecked(c) == true);

    // PTR_DOWN inside then PTR_UP outside
    fired = false;
    Checkbox_handlePointer(c, PTR_DOWN, 10.0f, 10.0f);
    Checkbox_handlePointer(c, PTR_UP, 50.0f, 50.0f);
    assert(fired == false);
    assert(Checkbox_isChecked(c) == true);

    // §5 Toggle clears indeterminate
    Checkbox_setIndeterminate(c, true);
    assert(Checkbox_isIndeterminate(c) == true);
    fired = false;
    Checkbox_handlePointer(c, PTR_DOWN, 10.0f, 10.0f);
    Checkbox_handlePointer(c, PTR_UP, 10.0f, 10.0f);
    assert(fired == true);
    assert(Checkbox_isIndeterminate(c) == false);
    assert(Checkbox_isChecked(c) == false);

    Memory_free(c);
    printf("=== Checkbox Test PASS ===\n");
    return 0;
}
