#include "annotation/overview.h"

#include <stdio.h>
#include <string.h>

#include "darling/field/radiogroup.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: RadioGroupTest (darling/field/radiogroup_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for RadioGroup option storage, mutual exclusion, onSelect
 * callbacks, and pointer handling (Pkg 2).
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static int g_failures = 0;

#define CHECK(name, cond) do { \
    if (cond) { printf("[radiogroup_test] PASS %s\n", name); } \
    else { printf("[radiogroup_test] FAIL %s\n", name); g_failures++; } \
} while (0)

static void onSelectCb(void *ctx) {
    (*(int*) ctx)++;
}

int main(void) {
    printf("=== Running RadioGroup Test Suite ===\n");

    // §1 Primary specification flow: create, add options, select, pointer hit
    {
        RadioGroup *g = RadioGroup_0();
        CHECK("RadioGroup_0 created", g != nullptr);

        RadioGroup_addOption(g, "Option A");
        RadioGroup_addOption(g, "Option B");
        RadioGroup_addOption(g, "Option C");

        CHECK("RadioGroup_optionCount == 3", RadioGroup_optionCount(g) == 3);
        CHECK("RadioGroup_getOption(g, 1) == Option B",
              RadioGroup_getOption(g, 1) != nullptr && strcmp(RadioGroup_getOption(g, 1), "Option B") == 0);

        int callbackFired = 0;
        RadioGroup_setOnSelect(g, onSelectCb);
        RadioGroup_setCtx(g, &callbackFired);
        CHECK("RadioGroup callback getter round-trip",
              RadioGroup_getOnSelect(g) == onSelectCb && RadioGroup_getCtx(g) == &callbackFired);

        RadioGroup_select(g, 1);
        CHECK("RadioGroup_select(g, 1) fired callback", callbackFired == 1);
        CHECK("RadioGroup_getSelected(g) == 1", RadioGroup_getSelected(g) == 1);

        // Selecting same index again must not re-fire
        RadioGroup_select(g, 1);
        CHECK("select same index does not refire", callbackFired == 1);

        // Simulate RadioGroup_handlePointer(g, PTR_UP, 10.0f, 50.0f) to select option 2
        // Default height is 60.0f, 3 items -> rows of 20.0f each.
        // y = 50.0f falls into index 2 (40.0f .. 60.0f).
        RadioGroup_handlePointer(g, PTR_UP, 10.0f, 50.0f);
        CHECK("handlePointer PTR_UP selected option 2", RadioGroup_getSelected(g) == 2);
        CHECK("handlePointer PTR_UP fired callback", callbackFired == 2);

        RadioGroup_free(g);
    }

    // §2 Horizontal orientation and pointer handling
    {
        RadioGroup *g = RadioGroup_0();
        RadioGroup_setOrientation(g, RADIOGROUP_HORIZONTAL);
        CHECK("orientation horizontal", RadioGroup_getOrientation(g) == RADIOGROUP_HORIZONTAL);

        RadioGroup_addOption(g, "First");
        RadioGroup_addOption(g, "Second");
        RadioGroup_addOption(g, "Third");

        int fired = 0;
        RadioGroup_setOnSelect(g, onSelectCb);
        RadioGroup_setCtx(g, &fired);

        // Default width 120.0f, 3 items -> cols of 40.0f each.
        // x = 10.0f falls into index 0.
        RadioGroup_handlePointer(g, PTR_UP, 10.0f, 10.0f);
        CHECK("horizontal select index 0", RadioGroup_getSelected(g) == 0 && fired == 1);

        // x = 50.0f falls into index 1.
        RadioGroup_handlePointer(g, PTR_UP, 50.0f, 10.0f);
        CHECK("horizontal select index 1", RadioGroup_getSelected(g) == 1 && fired == 2);

        // Pointer outside bounds should NOT select
        RadioGroup_handlePointer(g, PTR_UP, 200.0f, 10.0f);
        CHECK("pointer outside does not change selection", RadioGroup_getSelected(g) == 1 && fired == 2);

        // Non-UP pointer event should NOT select
        RadioGroup_handlePointer(g, PTR_DOWN, 90.0f, 10.0f);
        CHECK("PTR_DOWN does not select", RadioGroup_getSelected(g) == 1 && fired == 2);

        RadioGroup_free(g);
    }

    // §3 Option clearing, out-of-bounds, and setSelected forwarding
    {
        RadioGroup *g = RadioGroup_0();
        RadioGroup_addOption(g, "X");
        RadioGroup_addOption(g, "Y");
        CHECK("initial count 2", RadioGroup_optionCount(g) == 2);

        int fired = 0;
        RadioGroup_setOnSelect(g, onSelectCb);
        RadioGroup_setCtx(g, &fired);

        RadioGroup_setSelected(g, 0);
        CHECK("setSelected forwarded to select", RadioGroup_getSelected(g) == 0 && fired == 1);

        // Out of bounds selection is no-op
        RadioGroup_select(g, -1);
        CHECK("negative index ignored", RadioGroup_getSelected(g) == 0 && fired == 1);
        RadioGroup_select(g, 99);
        CHECK("excess index ignored", RadioGroup_getSelected(g) == 0 && fired == 1);

        RadioGroup_clear(g);
        CHECK("count after clear is 0", RadioGroup_optionCount(g) == 0);
        CHECK("selected after clear is -1", RadioGroup_getSelected(g) == -1);
        CHECK("getOption after clear is nullptr", RadioGroup_getOption(g, 0) == nullptr);

        // Re-adding after clear works
        RadioGroup_addOption(g, "Z");
        CHECK("count after re-add is 1", RadioGroup_optionCount(g) == 1);
        CHECK("getOption 0 is Z", strcmp(RadioGroup_getOption(g, 0), "Z") == 0);

        RadioGroup_free(g);
    }

    // §4 Null safety
    {
        RadioGroup_addOption(nullptr, "Option");
        RadioGroup_clear(nullptr);
        RadioGroup_select(nullptr, 0);
        RadioGroup_setSelected(nullptr, 0);
        RadioGroup_setOrientation(nullptr, 1);
        RadioGroup_setOnSelect(nullptr, nullptr);
        RadioGroup_setCtx(nullptr, nullptr);
        RadioGroup_handlePointer(nullptr, PTR_UP, 0.0f, 0.0f);
        RadioGroup_free(nullptr);

        CHECK("null getters safe", RadioGroup_getSelected(nullptr) == -1 &&
                                  RadioGroup_getOrientation(nullptr) == RADIOGROUP_VERTICAL &&
                                  RadioGroup_optionCount(nullptr) == 0 &&
                                  RadioGroup_getOption(nullptr, 0) == nullptr &&
                                  RadioGroup_getOnSelect(nullptr) == nullptr &&
                                  RadioGroup_getCtx(nullptr) == nullptr);
    }

    printf("\n=== RadioGroup Test Summary: %d failures ===\n", g_failures);
    return g_failures > 0 ? 1 : 0;
}
