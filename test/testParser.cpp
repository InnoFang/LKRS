#include <iostream>
#include <fstream>
#include <string>
#include "parser/SPARQLParser.hpp"

void testSPARQLParser(std::string &sparql) {
    Parser parser;
    parser.parse(sparql);
    std::cout << "[parser] variables:" << std::endl;
    for (const auto &item : parser.getQueryVariables()) {
        std::cout << item << std::endl;
    }

    std::cout << "[parser] triples:" << std::endl;
    for (const Triple &triple : parser.getQueryTriples()) {
        std::cout << triple.s << " " << triple.p << " " << triple.o << " " << std::endl;
    }
}

void test1() {
    std::string sparql = "select ?x ?p where { ?x ?p <FullProfessor0>. }";
    testSPARQLParser(sparql);
}

void test2() {
    std::string filename = R"(../data/p3.sql)";
    std::ifstream infile(filename, std::ios::in);
    std::ostringstream buf;
    char ch;
    if (infile.is_open()) {
        while (buf && infile.get(ch)) {
            buf.put(ch);
        }
        std::string sparql = buf.str();
        testSPARQLParser(sparql);
    } else {
        std::cout << "cannot open file: " << std::endl;
    }
    infile.close();
}

int main() {
    test2();
    return 0;
}
