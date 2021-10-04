#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>
#include "parser/sparql_parser.hpp"


namespace test {

namespace fs = boost::filesystem;

class SparqlParserTest : public testing::Test {
protected:
    std::string readSPARQLFromFile(const fs::path& filepath) {
        fs::ifstream infile(filepath, std::ios::in);
        fs::ifstream::sync_with_stdio(false);
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
};

TEST_F(SparqlParserTest, IndistinctSparql) {
    std::string sparql = "select ?x ?p where { ?x ?p <FullProfessor0>. }";
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_FALSE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x", "?p"};
    EXPECT_EQ(expect_variables, query_variables);
}

TEST_F(SparqlParserTest, DistinctSparql) {
    std::string sparql = "select distinct ?x ?p where { ?x ?p <FullProfessor0>. }";
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_TRUE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x", "?p"};
    EXPECT_EQ(expect_variables, query_variables);
}

TEST_F(SparqlParserTest, ParseSqlFromFile) {
    fs::path project_dir = fs::path(std::string(__FILE__)).parent_path().parent_path();

    fs::path sparql_file_path =
            project_dir.append("data").append("lubm").append("lubm_q1.sql");

    std::string sparql = readSPARQLFromFile(sparql_file_path);
    SparqlParser sparqlParser(sparql);

    auto distinct = sparqlParser.isDistinct();
    EXPECT_TRUE(distinct);

    auto query_variables = sparqlParser.getQueryVariables();
    auto expect_variables = std::vector<std::string> {"?x"};
    EXPECT_EQ(expect_variables, query_variables);

    auto query_triples_size = sparqlParser.getQueryTriples().size();
    EXPECT_EQ(6, query_triples_size);
}

} // namespace test
