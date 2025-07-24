/*
The goal of this class is to allow for operations on secdonary keys and combinations of keys.

This will be achieved by having multiple B+ trees for different non-primary keys and non-primary key combinations.

This class will wrap a pointer to a physical item in the primary B+ tree (the one sorted by the integer primary key).

It will implement all the same methods of the regular item class by referencing the physical record, 
allowing for all the same types of comparisons without wasting more space on copies of key fields.

Different comparator methods will be provided for different keys and combinations of keys.

The B+ tree nodes' insert methods will be refactored to take as a parameter an instruction
telling it which comparator to use for B+ tree operations.
*/