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

std::string readSPARQLFromFile(const std::string& filepath) {
    std::ifstream infile(filepath, std::ios::in);
    std::ifstream::sync_with_stdio(false);
    std::ostringstream buf;
    std::string sparql;
    char ch;
    if (infile.is_open()) {
        while (buf && infile.get(ch)) {
            buf.put(ch);
        }
        sparql = buf.str();
    } else {
        std::cerr << "cannot open file: " << filepath << std::endl;
    }
    infile.close();
    return sparql;
}

TEST(SparqlParserTest, ParseSqlFile) {
    std::string filename = R"(../../data/lubm/lubm_q1.sql)";
    std::string sparql = readSPARQLFromFile(filename);
    std::string expect_sparql = "select distinct ?x where\n"
                                "{\n"
                                "?x\t<rdf:type>\t<ub:GraduateStudent>.\n"
                                "?y\t<rdf:type>\t<ub:University>.\n"
                                "?z\t<rdf:type>\t<ub:Department>.\n"
                                "?x\t<ub:memberOf>\t?z.\n"
                                "?z\t<ub:subOrganizationOf>\t?y.\n"
                                "?x\t<ub:undergraduateDegreeFrom>\t?y.\n"
                                "}\n";
    EXPECT_EQ(expect_sparql, sparql);
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_TRUE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x"};
    EXPECT_EQ(expect_variables, query_variables);

    auto query_triples_size = sparqlParser.getQueryTriples().size();
    EXPECT_EQ(6, query_triples_size);
}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
