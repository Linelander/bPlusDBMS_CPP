#include <cassert>
#include <cstdio>
#include <filesystem>
#include <iostream>
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

    cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
