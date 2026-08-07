#include <tablog.h>

#include "query_engine.h"

#include <iostream>
#include <string>
#include <chrono>
#include <memory>
#include <arrow/table.h>

using namespace tql;

int main() {
    std::string query;
    QueryEngine queryEngine;

    while (true) {
        std::cout << "Please enter a query: ";
        std::getline(std::cin, query);

        std::chrono::time_point start = std::chrono::steady_clock::now();
        std::shared_ptr<arrow::Table> resultTable = queryEngine.execute(query);
        std::chrono::time_point end = std::chrono::steady_clock::now();

        std::cout << "The query was executed in: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "µs "
                  << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() << "ns " << std::endl;
        std::cout << "Result table: \n" << resultTable->ToString() << std::endl;
    }
    
    return 0;
}
