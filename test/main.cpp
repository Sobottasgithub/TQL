#include <tablog.h>

#include "query_engine.h"

#include <iostream>
#include <string>
#include <memory>

using namespace tql;

int main() {
    std::string query;
    QueryEngine queryEngine;

    while (true) {
        std::cout << "Please enter a query: ";
        std::getline(std::cin, query);

        queryEngine.execute(query);
    }
    
    return 0;
}
