// This class will handle SQL parsing and query planning.

#include <iostream>
#include "Table.h"

using namespace std;

class Database {
    private:
        vector<Table*> tables;

    public:
        auto findTable (string tableName) {
                        auto itr = tables.begin();
            while ((*itr)->getTableName() != tableName)
            {
                itr++;
            }
    
            return *itr;
        }


        // these are testing methods not meant to replace a real query planner.
        void selectAllFromTableWhere(string tableName, string columnName, string equals)
        {
            Table* table = findTable(tableName);
            ItemInterface* selection = table->whereSingle(columnName, equals);
            selection->print();
        }


        void selectAllFromTable(string tableName) {
            // Use a modification of ripPrint to print every item in a table
        }


        void removeAllFromTableWhere(string tableName, string columnName, string equals) {
            Table* table = findTable(tableName);
            table->delWhereSingle(columnName, equals);
        }
};