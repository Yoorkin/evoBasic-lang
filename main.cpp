#include<iostream>
#include<fstream>
#include"Lexer.h"
#include"Parser.h"
using namespace evoBasic;
int main() {
    ifstream file("test.txt");
    string str;
    Lexer lexer(file);
    Token token;
    Logger logger(cout);
    Domain domain;
    Parser parser(lexer,domain);
    auto node = parser.parse();
    node->print("","*",true,cout);
}
