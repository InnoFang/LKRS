#include <iostream>

#include "parser/SPARQLParser.hpp"

std::string readSPARQLFromFile(const std::string& filepath) const {
//    std::string filename = R"(../data/p3.sql)";
    std::ifstream infile(filepath, std::ios::in);
    std::ostringstream buf;
    std::string sparql = "";
    char ch;
    if (infile.is_open()) {
        while (buf && infile.get(ch)) {
            buf.put(ch);
        }
        sparql = buf.str();
    } else {
        std::cout << "cannot open file: " << std::endl;
    }
    infile.close();
    return sparql;
}

int main() {
    std::string sparql = readSPARQLFromFile("../data/p1.sql");
    std::cout << sparql << std::endl;
    return 0;
}