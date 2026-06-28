#include <iostream>
#include "BPlusTree.h"
#include "Item.h"
// #include "ItemInterface.h"
// #include "NCItem.h"
#include <string.h>
#include <functional>








using namespace std;


#ifndef COLUMN
#define COLUMN
class Column {
    private:
        std::shared_ptr<void> tree;
        std::string typeName;

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
            : tree(tree), typeName(typeid(T).name())
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

            getColumnNameFn = [tree]() -> string {
                return string(tree->getColumnName().data());
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
        int columnCount;

    public:
        string getTableName() {return tableName;}
    
    
        // DUAL PURPOSE CONSTRUCTOR
        Table(const string& tableName, int numColumns, const vector<string>& columnFileNames, int branchFactor)
            : tableName(tableName), columnCount(numColumns) {
            



            // PK
            auto maintree = createBPlusTree<int>(branchFactor, 0, columnCount, tableName, columnFileNames[0], nullptr);
            string realColName;
            if (columnFileNames[0].length() < 7 || columnFileNames[0].substr(columnFileNames[0].length() - 7) != ".bptree") {
                realColName = columnFileNames[0] + ".bptree";
            }
            Column* mainColumn = new Column(maintree, realColName); // okay to pass the auto here?
            clusteredIndex = mainColumn;






            // Columns
            for (int i = 1; i < columnFileNames.size(); i++)
            {
                auto otherTree = createBPlusTree<AttributeType>(branchFactor, i, columnCount, tableName, columnFileNames[i], maintree);
                string realNonColName;
                if (columnFileNames[i].length() < 7 || columnFileNames[i].substr(columnFileNames[i].length() - 7) != ".bptree") {
                    realNonColName = columnFileNames[i] + ".bptree";
                }
                Column* otherColumn = new Column(otherTree, realNonColName);
                nonclusteredIndices.push_back(otherColumn);
            }
        }





        // DESTRUCTOR
        ~Table() {
            for (Column* col : nonclusteredIndices)
            {
                delete col; // should call delete on the B+ tree... anything else?
            }
            delete clusteredIndex;
            cout << tableName << " shut down.";
        }






        


        template <typename T>
        ItemInterface* whereSingle(string columnName, T equals) {
            auto itr = nonclusteredIndices.begin();
            while ((*itr)->getColumnName() != columnName && itr != nonclusteredIndices.end())
            {
                itr++;
            }

            if (itr == nonclusteredIndices.end())
            {
                cout << "Column " << columnName << "not found in " << tableName << "." << endl;
                return nullptr;
            }

            ItemInterface* result = (*itr)->singleKeySearchFn(&equals);
            return result;
        }



        template <typename T>
        ItemInterface* delWhereSingle(string columnName, T equals) {
            auto itr = nonclusteredIndices.begin();
            while ((*itr)->getColumnName() != columnName && itr != nonclusteredIndices.end())
            {
                itr++;
            }

            if (itr == nonclusteredIndices.end())
            {
                cout << "Column " << columnName << "not found in " << tableName << "." << endl;
                return nullptr;
            }

            ItemInterface* result = (*itr)->removeFn(&equals);
            return result;
        }



        
};
#endif