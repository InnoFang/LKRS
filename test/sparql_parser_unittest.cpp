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

TEST(SparqlParserTest, DistinctSparql) {
    std::string sparql = "select distinct ?x ?p where { ?x ?p <FullProfessor0>. }";
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_TRUE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x", "?p"};
    EXPECT_EQ(expect_variables, query_variables);
}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
