#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  smatch match;
  string str = "My new phone is";
  string str2 = "My new phone is \"smart.\" I guess that I'm not.";
  string left_query = "My";
  string right_query = "new";
  regex test(left_query + "\\s*" + right_query + ".*");
  if (regex_match(str, match, test)) {
    cout << "success" << endl;
  } else {
    cout << "faillure" << endl;
  }
}