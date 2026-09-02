#include <tablog.h>

#include "query_engine.h"

#include <iostream>
#include <chrono>
#include <memory>
#include <arrow/table.h>
#include <readline/readline.h>
#include <readline/history.h>

using namespace tql;

int main() {
    char* query;
    QueryEngine queryEngine;

    while (true) {
        query = readline("\033[106;97m TQL \033[0m> ");

        try {
            std::chrono::time_point start = std::chrono::steady_clock::now();
            std::shared_ptr<arrow::Table> resultTable = queryEngine.execute(query);
            std::chrono::time_point end = std::chrono::steady_clock::now();

            std::cout << "The query was executed in: "
                      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "µs "
                      << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() << "ns " << std::endl;
            std::cout << "Result table: \n" << resultTable->ToString() << std::endl;

            add_history(query);
            std::free(query);
        } catch (const std::invalid_argument& invalidArgument) {
            std::cout << "Error: " << invalidArgument.what() << std::endl;
        }
    }
    
    return 0;
}
