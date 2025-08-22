#include <iostream>
#include "BPlusTree.h"
#include "Item.h"
#include "ItemInterface.h"
#include "NCItem.h"
#include <string.h>
#include <functional>


/*


NOTE: Thinking about scrapping the column class. Why not just store that info in BPlusTree?


*/


using namespace std;


#ifndef COLUMN
#define COLUMN
class Column {
    private:
        std::shared_ptr<void> tree_ptr;
        std::string type_name;

        // Function objects
        
    public:
        std::function<void(ItemInterface*)> insertFn;
        std::function<ItemInterface*(const void*)> removeFn;
        std::function<void()> printFn;
        std::function<string()> getColumnNameFn;
        std::function<void()> ripPrintFn;
        std::function<ItemInterface*(const void*)> singleKeySearchFn;

        // constructor
        template <typename T>
        Column(std::shared_ptr<BPlusTreeBase<T>> tree, string colName)
            : tree_ptr(tree), type_name(typeid(T).name())
        {
            insertFn = [tree](ItemInterface* item) {
                tree->insert(item);
            };

            removeFn = [tree](const void* deleteIt)->ItemInterface* {
                const T* typedValue = static_cast<const T*>(deleteIt);
                return tree->remove(*typedValue);
            };

            printFn = [tree]() {
                tree->print();
            };

            ripPrintFn = [tree]() {
                tree->ripPrint();
            };

            getColumnNameFn = [tree]() {
                tree->getColumnName();
            };

            singleKeySearchFn = [tree](const void* findIt) {
                const T* typedValue = static_cast<const T*>(findIt);
                return tree->singleKeySearch(*typedValue);
            };
        }

        string getColumnName() {return getColumnNameFn();}
};
#endif




#ifndef TABLE
#define TABLE
class Table {
    private:
        string tableName;
        Column* clusteredIndex;
        vector<Column*> nonclusteredIndices;

    public:
        string getTableName() {return tableName;}
    
    
        // Constructor for testing: Future constructor will take a SQL query and parse... I think. Or maybe the database class will parse the query and use this constructor.
        Table(string name, int numColumns, vector<string> columnNames, int branchFactor) : tableName(name) {
            
            // Clustered index on primary key
            auto mainTree = createBPlusTree<int>(??????);
            clusteredIndex = new Column(mainTree, "PK");
            
            // Nonclustered indices on attributes
            int i = 1;
            for (string name : columnNames){
                auto attTree = createBPlusTree<AttributeType>(branchFactor, i, name.c_str()); // why did I have to c_str that?
                Column* attColumn = new Column(attTree, name);
                columns.push_back(attColumn);
                i++;
            }
        }




        template <typename T>
        ItemInterface* whereSingle(string columnName, T equals) {
            auto itr = columns.begin();
            while ((*itr)->getColumnName() != columnName && itr != columns.end())
            {
                itr++;
            }

            if (itr == columns.end())
            {
                cout << "Column " << columnName << "not found in " << tableName << "." << endl;
                return nullptr;
            }

            ItemInterface* result = (*itr)->singleKeySearchFn(equals);
        }



        template <typename T>
        ItemInterface* delWhereSingle(string columnName, T equals) {
            auto itr = columns.begin();
            while ((*itr)->getColumnName() != columnName && itr != columns.end())
            {
                itr++;
            }

            if (itr == columns.end())
            {
                cout << "Column " << columnName << "not found in " << tableName << "." << endl;
                return nullptr;
            }

            ItemInterface* result = (*itr)->removeFn(equals);
        }
};
#endif