/*
NCItem: A nonclustered item. Wraps a regular item pointer and implements the same getters, but has no fields.

The goal of this class is to allow for operations on secondary keys and combinations of keys.

This will be achieved by having multiple B+ trees for different non-primary keys and non-primary key combinations.

This class will wrap a pointer to a physical item in the primary B+ tree (the one sorted by the integer primary key).

It will implement all the same methods of the regular item class by referencing the physical record, 
allowing for all the same types of comparisons without wasting more space on copies of key fields.

Different comparator methods will be provided for different keys and combinations of keys.

The B+ tree nodes' insert methods will be refactored to take as a parameter an instruction
telling it which comparator to use for B+ tree operations.

Those methods rely on returning null to tell their parents when they haven't split, so that approach has to be preserved.
They will be refactored to return a wrapping class that holds a BPNode pointer and an item pointer. 
    The first slot in the array will serve the same purpose of the original return value.
    The second slot will be a pointer to the item that was just inserted.
        After insertion into the primary B+ tree, the returned item pointer will be wrapped in this class and inserted into the secondary clustered indexes.
*/