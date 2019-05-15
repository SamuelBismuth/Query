/* Samuel Bismuth 342533064 */
/* Matan Zilka 307949438 */

#include "Query.h"
#include "TextQuery.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>

using namespace std;
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string &s) {
  // Regexes to match with all the format
  regex regexAndOrNum("\\s*([\\w']+)\\s+(AND|OR|\\d+)\\s+([\\w']+)\\s*");
  regex regexNot("\\s*NOT\\s+([\\w']+)\\s*");
  regex regexWord("\\s*([\\w']+)\\s*");
  smatch matchThreeWords, matchTwoWords, matchOneWord;
  // We're now testing the string, and get the match group.
  if (regex_match(s, matchThreeWords, regexAndOrNum)) {
    string leftWord = matchThreeWords.str(1);
    string logic = matchThreeWords.str(2);
    string rightWord = matchThreeWords.str(3);
    if (logic == "AND") {
      return std::shared_ptr<QueryBase>(new AndQuery(leftWord, rightWord));
    } else if (logic == "OR") {
      return std::shared_ptr<QueryBase>(new OrQuery(leftWord, rightWord));
    } else {
      return std::shared_ptr<QueryBase>(
          new NQuery(leftWord, rightWord, stoi(logic)));
    }
  } else if (regex_match(s, matchTwoWords, regexNot)) {
    return std::shared_ptr<QueryBase>(new NotQuery(matchTwoWords.str(1)));
  } else if (regex_match(s, matchOneWord, regexWord)) {
    return std::shared_ptr<QueryBase>(new WordQuery(matchOneWord.str(1)));
  } else {
    throw invalid_argument("Unrecognized search");
  }
}
////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const {
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();

  for (size_t n = 0; n != sz; ++n) {
    if (beg == end || *beg != n)
      ret_lines->insert(n);
    else if (beg != end)
      ++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
}

QueryResult AndQuery::eval(const TextQuery &text) const {
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);

  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
                        right_result.begin(), right_result.end(),
                        std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const {
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);

  auto ret_lines = std::make_shared<std::set<line_no>>(left_result.begin(),
                                                       left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult NQuery::eval(const TextQuery &text) const {
  QueryResult result = AndQuery::eval(text);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto iter = result.begin(), iter_end = result.end();
  // The regexes.
  regex words_regex("[\\w']+");
  regex regexNumLr("(.)*" + left_query + " ([\\w']+ ){0," + to_string(dist) +
                   "}" + right_query + "(.)*");
  regex regexNumRl("(.)*" + right_query + " ([\\w']+ ){0," + to_string(dist) +
                   "}" + left_query + "(.)*");
  for (; iter != iter_end; ++iter) {
    string line = result.get_file()->at(*iter);
    string newLine = "";
    auto words_begin = sregex_iterator(line.begin(), line.end(), words_regex);
    auto words_end = sregex_iterator();
    for (sregex_iterator i = words_begin; i != words_end; ++i) {
      smatch match = *i;
      string match_str = match.str();
      newLine = newLine + match_str + " ";
    }
    if (regex_match(newLine, regexNumLr) || regex_match(newLine, regexNumRl)) {
      ret_lines->insert(*iter);
    }
  }
  return QueryResult(rep(), ret_lines, result.get_file());
}
/////////////////////////////////////////////////////////