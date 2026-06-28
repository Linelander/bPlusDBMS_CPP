#include <cassert>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include "../BPLeaf.h"
#include "../Table.h"

using namespace std;

static int passed = 0;
static int failed = 0;

static void check(bool cond, const string& name) {
    if (cond) {
        cout << "  [PASS] " << name << "\n";
        ++passed;
    } else {
        cout << "  [FAIL] " << name << "\n";
        ++failed;
    }
}

static void cleanup(const string& dir) {
    if (filesystem::exists(dir)) {
        filesystem::remove_all(dir);
    }
}

// Helper: make an Item with integer primary key and one attribute
static ItemInterface* makeItem(int pk, const string& attr) {
    AttributeType a{};
    size_t len = min(attr.size(), (size_t)(COLUMN_LENGTH - 1));
    copy(attr.begin(), attr.begin() + len, a.begin());
    return new Item(pk, {a});
}

// ──────────────────────────────────────────────────────────────
// Test 1: Basic insert + search on clustered (int) tree
// ──────────────────────────────────────────────────────────────
static int testInsertSearch() {
    cout << "Test: insert & search\n";
    cleanup("instest");
    auto tree = createBPlusTree<int>(3, 0, 1, "instest", "pk", nullptr);

    for (int i = 0; i < 30; i++) {
        tree->insert(makeItem(i, "v" + to_string(i)));
    }

    bool allFound = true;
    for (int i = 0; i < 30; i++) {
        ItemInterface* r = tree->singleKeySearch(i);
        if (r == nullptr || r->getPrimaryKey() != i) {
            allFound = false;
            break;
        }
    }
    check(allFound, "all 30 inserted keys found");

    ItemInterface* missing = tree->singleKeySearch(999);
    check(missing == nullptr, "search miss returns nullptr");

    cleanup("instest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 2: Ascending insert, then ripPrint ordering is ascending
// ──────────────────────────────────────────────────────────────
static int testAscendingOrder() {
    cout << "Test: ascending insert order\n";
    cleanup("asctest");
    auto tree = createBPlusTree<int>(3, 0, 1, "asctest", "pk", nullptr);

    for (int i = 0; i < 25; i++) {
        tree->insert(makeItem(i, ""));
    }

    // Verify every key from 0..24 is searchable
    bool ok = true;
    for (int i = 0; i < 25; i++) {
        if (tree->singleKeySearch(i) == nullptr) { ok = false; break; }
    }
    check(ok, "all keys searchable after ascending insert");
    cleanup("asctest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 3: Descending insert
// ──────────────────────────────────────────────────────────────
static int testDescendingOrder() {
    cout << "Test: descending insert order\n";
    cleanup("desctest");
    auto tree = createBPlusTree<int>(3, 0, 1, "desctest", "pk", nullptr);

    for (int i = 24; i >= 0; i--) {
        tree->insert(makeItem(i, ""));
    }

    bool ok = true;
    for (int i = 0; i < 25; i++) {
        if (tree->singleKeySearch(i) == nullptr) { ok = false; break; }
    }
    check(ok, "all keys searchable after descending insert");
    cleanup("desctest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 4: Duplicate primary key is rejected
// ──────────────────────────────────────────────────────────────
static int testDuplicateKey() {
    cout << "Test: duplicate primary key rejection\n";
    cleanup("duptest");
    auto tree = createBPlusTree<int>(3, 0, 1, "duptest", "pk", nullptr);

    tree->insert(makeItem(5, "first"));
    tree->insert(makeItem(5, "second")); // should be silently rejected

    // Only one record with PK=5 should exist
    ItemInterface* r = tree->singleKeySearch(5);
    check(r != nullptr && r->getPrimaryKey() == 5, "PK=5 still retrievable after dup insert");
    cleanup("duptest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 5: Remove simple (no rebalancing)
// ──────────────────────────────────────────────────────────────
static int testSimpleRemove() {
    cout << "Test: simple remove\n";
    cleanup("rmtest");
    auto tree = createBPlusTree<int>(5, 0, 1, "rmtest", "pk", nullptr);

    for (int i = 0; i < 10; i++) tree->insert(makeItem(i, ""));

    tree->remove(5);
    check(tree->singleKeySearch(5) == nullptr, "removed key not found");
    check(tree->singleKeySearch(4) != nullptr, "neighbours still present (4)");
    check(tree->singleKeySearch(6) != nullptr, "neighbours still present (6)");
    cleanup("rmtest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 6: Remove with rebalancing (many removals forcing merges)
// ──────────────────────────────────────────────────────────────
static int testRebalancingRemove() {
    cout << "Test: remove with rebalancing\n";
    cleanup("rebaltest");
    auto tree = createBPlusTree<int>(3, 0, 1, "rebaltest", "pk", nullptr);

    for (int i = 0; i < 25; i++) tree->insert(makeItem(i, ""));

    // Remove in a pattern that forces steals and merges
    vector<int> toRemove = {6, 8, 10, 12, 4, 5, 3, 24, 23, 17, 13, 14};
    for (int k : toRemove) {
        tree->remove(k);
    }

    bool removedGone = true;
    for (int k : toRemove) {
        if (tree->singleKeySearch(k) != nullptr) { removedGone = false; break; }
    }
    check(removedGone, "all removed keys are gone");

    // Spot-check that a few remaining keys still exist
    check(tree->singleKeySearch(0)  != nullptr, "key 0 still present");
    check(tree->singleKeySearch(7)  != nullptr, "key 7 still present");
    check(tree->singleKeySearch(22) != nullptr, "key 22 still present");
    cleanup("rebaltest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 7: Table constructor / destructor (smoke test)
// ──────────────────────────────────────────────────────────────
static int testTable() {
    cout << "Test: table create / destroy\n";
    cleanup("tbl");
    {
        Table* t = new Table("tbl", 2, {"pk", "name"}, 5);
        check(t->getTableName() == "tbl", "table name");
        delete t;
    }
    cleanup("tbl");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 8: Larger branching factor (way=8) with 100 inserts
// ──────────────────────────────────────────────────────────────
static int testLargerWay() {
    cout << "Test: way=8, 100 inserts\n";
    cleanup("bigtest");
    auto tree = createBPlusTree<int>(8, 0, 1, "bigtest", "pk", nullptr);

    for (int i = 0; i < 100; i++) tree->insert(makeItem(i, "x"));

    bool ok = true;
    for (int i = 0; i < 100; i++) {
        if (tree->singleKeySearch(i) == nullptr) { ok = false; break; }
    }
    check(ok, "all 100 keys found with way=8");
    cleanup("bigtest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 9: Random-order insert + search (small page, forces splits)
// ──────────────────────────────────────────────────────────────
static int testRandomOrder() {
    cout << "Test: random-order insert (small page)\n";
    cleanup("randtest");
    auto tree = createBPlusTree<int>(3, 0, 1, "randtest", "pk", nullptr, 300);

    vector<int> keys = {17,3,42,8,31,55,1,29,14,47,6,38,22,50,11,
                        44,27,5,33,19,48,2,36,13,41,9,25,52,16,39};
    for (int k : keys) tree->insert(makeItem(k, ""));

    bool ok = true;
    for (int k : keys)
        if (tree->singleKeySearch(k) == nullptr) { ok = false; break; }
    check(ok, "all random-order keys found");
    check(tree->singleKeySearch(99) == nullptr, "missing key returns null");

    cleanup("randtest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 10: Interleaved inserts and removes (small page)
// ──────────────────────────────────────────────────────────────
static int testInterleaved() {
    cout << "Test: interleaved insert/remove (small page)\n";
    cleanup("intertest");
    auto tree = createBPlusTree<int>(3, 0, 1, "intertest", "pk", nullptr, 300);

    for (int i = 0; i < 20; i++) tree->insert(makeItem(i, ""));
    tree->remove(5); tree->remove(6); tree->remove(7);
    for (int i = 20; i < 35; i++) tree->insert(makeItem(i, ""));
    tree->remove(10); tree->remove(11); tree->remove(12);
    for (int i = 35; i < 45; i++) tree->insert(makeItem(i, ""));

    set<int> removed = {5,6,7,10,11,12};
    bool ok = true;
    for (int i = 0; i < 45; i++) {
        bool should = removed.count(i) == 0;
        bool found  = tree->singleKeySearch(i) != nullptr;
        if (should != found) { ok = false; break; }
    }
    check(ok, "all keys correct after interleaved ops");

    cleanup("intertest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 11: Delete everything — tree should handle full drain
// ──────────────────────────────────────────────────────────────
static int testFullDelete() {
    cout << "Test: delete all keys\n";
    cleanup("deltest");
    auto tree = createBPlusTree<int>(3, 0, 1, "deltest", "pk", nullptr, 300);

    for (int i = 0; i < 25; i++) tree->insert(makeItem(i, ""));
    for (int i = 0; i < 25; i++) tree->remove(i);

    bool allGone = true;
    for (int i = 0; i < 25; i++)
        if (tree->singleKeySearch(i) != nullptr) { allGone = false; break; }
    check(allGone, "all keys gone after full delete");

    cleanup("deltest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 12: Persistence — write, close, reopen, verify
// ──────────────────────────────────────────────────────────────
static int testPersistence() {
    cout << "Test: persistence (write, close, reopen)\n";
    cleanup("perstest");

    // Phase 1: write data and let the tree go out of scope (triggers flush)
    {
        auto tree = createBPlusTree<int>(3, 0, 1, "perstest", "pk", nullptr, 300);
        for (int i = 0; i < 30; i++) tree->insert(makeItem(i, "val"));
        tree->remove(5);
        tree->remove(15);
        tree->remove(25);
        // tree destructor writes header + flushes all pages
    }

    // Phase 2: reopen from the same file and verify
    {
        auto tree = createBPlusTree<int>(3, 0, 1, "perstest", "pk", nullptr, 300);

        bool ok = true;
        for (int i = 0; i < 30; i++) {
            bool should = (i != 5 && i != 15 && i != 25);
            bool found  = tree->singleKeySearch(i) != nullptr;
            if (should != found) { ok = false; break; }
        }
        check(ok, "data survives close and reopen");
    }

    cleanup("perstest");
    return 0;
}

// ──────────────────────────────────────────────────────────────
// Test 13: Visual merge/rebalance walk
//   Builds a 4-level tree (30 items, way=3, 300-byte pages), then
//   removes items in a sequence that forces leaf steals and merges,
//   printing the full tree after every step.
// ──────────────────────────────────────────────────────────────
static void printTree(const string& label,
                      shared_ptr<BPlusTreeBase<int>> tree) {
    cout << "\n=== " << label << " ===\n";
    cout << "[ leaf layer ]\n";
    tree->ripPrint();
    cout << "\n[ tree structure (rotated 90°, root in centre) ]\n";
    tree->print();
    cout << "\n";
}

static int testVisualMerge() {
    cout << "Test: visual merge/rebalance\n";
    cleanup("mergevis");

    auto tree = createBPlusTree<int>(3, 0, 1, "mergevis", "pk", nullptr, 300);
    for (int i = 0; i < 30; i++) {
        AttributeType a{};
        tree->insert(new Item(i, {a}));
    }

    printTree("initial (30 keys, way=3, 300-byte pages)", tree);

    // Each group removes one leaf's worth of items, forcing progressive
    // merges up through the internal nodes.
    vector<pair<string, vector<int>>> steps = {
        {"remove 28, 29 — right leaf becomes underfull, steals from neighbour",  {28, 29}},
        {"remove 24, 25 — triggers a leaf merge",                                {24, 25}},
        {"remove 18, 19, 20 — cascades into internal-node rebalance",            {18, 19, 20}},
        {"remove 0, 1, 2 — collapses left subtree",                              {0, 1, 2}},
        {"remove 10, 11 — approaches single-level tree",                         {10, 11}},
    };

    set<int> removed;
    for (auto& [label, keys] : steps) {
        for (int k : keys) {
            tree->remove(k);
            removed.insert(k);
        }
        printTree(label, tree);
    }

    // Verify correctness
    bool ok = true;
    for (int i = 0; i < 30; i++) {
        bool should = removed.count(i) == 0;
        bool found  = tree->singleKeySearch(i) != nullptr;
        if (should != found) { ok = false; break; }
    }
    check(ok, "remaining keys correct after all merges");

    cleanup("mergevis");
    return 0;
}

// ──────────────────────────────────────────────────────────────

int main() {
    cout << "=== B+ Tree Tests ===\n\n";

    testInsertSearch();
    testAscendingOrder();
    testDescendingOrder();
    testDuplicateKey();
    testSimpleRemove();
    testRebalancingRemove();
    testTable();
    testLargerWay();
    testRandomOrder();
    testInterleaved();
    testFullDelete();
    testPersistence();
    testVisualMerge();

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
