#include <iostream>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <gtest/gtest.h>
#include "parser/sparql_parser.hpp"

TEST(SparqlParserTest, IndistinctSparql) {
    std::string sparql = "select ?x ?p where { ?x ?p <FullProfessor0>. }";
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_FALSE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x", "?p"};
    EXPECT_EQ(expect_variables, query_variables);
}


//void test2() {
//    std::string filename = R"(../data/p3.sql)";
//    std::ifstream infile(filename, std::ios::in);
//    std::ostringstream buf;
//    char ch;
//    if (infile.is_open()) {
//        while (buf && infile.get(ch)) {
//            buf.put(ch);
//        }
//        std::string sparql = buf.str();
//        testSPARQLParser(sparql);
//    } else {
//        std::cout << "cannot open file: " << std::endl;
//    }
//    infile.close();
//}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
