#include "annotation/overview.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/container.h"
#include "darling/panel/panel.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: PanelDirtyTest (darling/panel/panel_dirty_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for recursive panel tree dirtiness and present-on-demand
 * support: null safety, single-node dirtiness, multi-level hierarchy
 * propagation, leaf-level mutations, and tree-wide clearing.
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

int main(void) {
    printf("=== Running Panel Dirty Test Suite ===\n");

    // §1 Null safety
    assert(Panel_isTreeDirty(nullptr) == false);
    Panel_clearTreeDirty(nullptr);

    // §2 Single panel clean and dirty transitions
    Panel *root = Panel_0();
    assert(root != nullptr);
    Panel_clearTreeDirty(root);
    assert(Panel_isTreeDirty(root) == false);

    Container *rootCont = &(*root).base;
    Container_markDirty(rootCont);
    assert(Panel_isTreeDirty(root) == true);

    Panel_clearTreeDirty(root);
    assert(Panel_isTreeDirty(root) == false);

    // §3 Multi-level hierarchy propagation:
    // root -> child1 -> grandchild
    //      -> child2
    Panel *child1 = Panel_0();
    Panel *child2 = Panel_0();
    Panel *grandchild = Panel_0();
    assert(child1 != nullptr && child2 != nullptr && grandchild != nullptr);

    Panel_addContainer(root, child1);
    Panel_addContainer(root, child2);
    Panel_addContainer(child1, grandchild);

    // Settle all nodes
    Panel_clearTreeDirty(root);
    assert(Panel_isTreeDirty(root) == false);
    assert(Panel_isTreeDirty(child1) == false);
    assert(Panel_isTreeDirty(child2) == false);
    assert(Panel_isTreeDirty(grandchild) == false);

    // Dirty only grandchild (deepest leaf)
    Container *gcCont = &(*grandchild).base;
    Container_markDirty(gcCont);

    // Assert that root and parent observe dirtiness, while sibling stays clean
    assert(Panel_isTreeDirty(grandchild) == true);
    assert(Panel_isTreeDirty(child1) == true);
    assert(Panel_isTreeDirty(child2) == false);
    assert(Panel_isTreeDirty(root) == true);

    // Clearing root clears all descendants
    Panel_clearTreeDirty(root);
    assert(Panel_isTreeDirty(grandchild) == false);
    assert(Panel_isTreeDirty(child1) == false);
    assert(Panel_isTreeDirty(child2) == false);
    assert(Panel_isTreeDirty(root) == false);

    // §4 Container mutation sets dirty and is reflected at root
    Container *c2Cont = &(*child2).base;
    Container_setOpacity(c2Cont, 0.75f);
    assert(Panel_isTreeDirty(child2) == true);
    assert(Panel_isTreeDirty(root) == true);

    Panel_clearTreeDirty(root);
    assert(Panel_isTreeDirty(root) == false);

    printf("=== Panel Dirty Test Suite: ALL PASS ===\n");
    return 0;
}
