#include "annotation/overview.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/button/switch.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: SwitchTest (darling/button/switch_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Switch live click controls (Pkg 2): pointer up toggle,
 * inside/outside bounds, state flip, callback firing.
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static void onSwitchChange(void *ctx) {
    (*(bool*) ctx) = true;
}

int main(void) {
    printf("=== Running Switch Test Suite ===\n");

    // §1 Null safety
    Switch_handlePointer(nullptr, PTR_DOWN, 10.0f, 10.0f);
    Switch_handlePointer(nullptr, PTR_UP, 10.0f, 10.0f);
    Switch_toggle(nullptr);

    // §2 Instantiate widget + set callback with context pointer
    Switch *s = Switch_0();
    assert(s != nullptr);
    assert(Switch_isOn(s) == false);

    bool fired = false;
    Switch_setOnChange(s, onSwitchChange, &fired);

    // §3 Call _handlePointer with PTR_DOWN then PTR_UP inside bounds
    // Default bounds are 44x24 when unset
    Switch_handlePointer(s, PTR_DOWN, 20.0f, 12.0f);
    assert(fired == false);
    assert(Switch_isOn(s) == false);

    Switch_handlePointer(s, PTR_UP, 20.0f, 12.0f);
    assert(fired == true);
    assert(Switch_isOn(s) == true);

    // §4 Call _handlePointer with PTR_UP outside bounds and assert callback did NOT fire
    fired = false;
    Switch_handlePointer(s, PTR_UP, 100.0f, 100.0f);
    assert(fired == false);
    assert(Switch_isOn(s) == true);

    // PTR_DOWN inside then PTR_UP outside
    fired = false;
    Switch_handlePointer(s, PTR_DOWN, 20.0f, 12.0f);
    Switch_handlePointer(s, PTR_UP, 100.0f, 100.0f);
    assert(fired == false);
    assert(Switch_isOn(s) == true);

    // §5 Second toggle inside flips back to off
    fired = false;
    Switch_handlePointer(s, PTR_DOWN, 20.0f, 12.0f);
    Switch_handlePointer(s, PTR_UP, 20.0f, 12.0f);
    assert(fired == true);
    assert(Switch_isOn(s) == false);

    Memory_free(s);
    printf("=== Switch Test PASS ===\n");
    return 0;
}
