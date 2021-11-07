//
// Created by yorkin on 7/11/21.
//

#include"Lexer.h"
#include<iostream>
#include <utility>
#include"Exception.h"
#include"../utils/Logger.h"

#define ReturnWithoutForward(token) {\
    stream.putback(c);\
    return Token(lexeme,token,Position(beginX,beginY,x-beginX,source));\
    }

#define ReturnWithForward(token) {\
    increaseX(c);\
    lexeme.push_back(c);\
    return Token(lexeme,token,Position(beginX,beginY,x-beginX,source));\
    }

namespace evoBasic {

#define E(x) #x
#define E_(x) #x
    const vector<string> Token::reserved = {LEXEME_LIST};
#define E(x) {#x,x}
#define E_(x) {#x,x##_}
    const map<string, Token::Enum> Token::reservedToEnumMap{LEXEME_LIST};
#undef E
#undef E_

    const Position Position::Empty{};

    bool isDigit(char c) {
        return c <= '9' && c >= '0';
    }

    bool isLetter(char c) {
        return c >= 'a' && c <= 'z';
    }

    bool isNewLine(char c) {
        return c == '\n' || c == '\r';
    }

    bool isWhiteSpace(char c) {
        return c == '\t' || c == ' ' || isNewLine(c);
    }

    char ToLower(char c) {
        if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
        else return c;
    }

    void Lexer::increaseX(char c) {
        if (isNewLine(c)) {
            y++;
            x = 1;
        } else x++;
    }

    Token Lexer::LL() {
        string lexeme;
        enum {
            Err, START, NUM, POINT, E1, E2, ID1, ID2, COM1, COM2, CB1, CB2, STR, CHAR1, CHAR2, L, E, G
        } state = START;
        auto& stream = source->getStream();
        while (true) {
            char c = ToLower(stream.get());
            if (stream.eof())c = -1;
            switch (state) {
                case START:
                    beginX = x;
                    beginY = y;
                    lexeme = "";
                    if (isDigit(c))state = NUM;
                    else if (isLetter(c))state = ID1;
                    else if (c == '/')state = COM1;
                    else if (c == '"')state = STR;
                    else if (c == '\'')state = CHAR1;
                    else if (c == '<')state = L;
                    else if (c == '(') ReturnWithForward(Token::LB)
                    else if (c == ')') ReturnWithForward(Token::RB)
                    else if (c == '*') ReturnWithForward(Token::MUL)
                    else if (c == '\\') ReturnWithForward(Token::DIV)
                    else if (c == '+') ReturnWithForward(Token::ADD)
                    else if (c == '-') ReturnWithForward(Token::MINUS)
                    else if (c == '.') ReturnWithForward(Token::DOT)
                    else if (c == ',') ReturnWithForward(Token::COMMA)
                    else if (isWhiteSpace(c))state = START;
                    else if (c == '<')state = L;
                    else if (c == '=')state = E;
                    else if (c == '>')state = G;
                    else if (c == -1)return Token("EOF", Token::EOF_, Position(beginX, beginY, x - beginX,source));
                    else state = Err;
                    break;
                case NUM:
                    if (isDigit(c))state = NUM;
                    else if (c == '.')state = POINT;
                    else if (c == 'e')state = E1;
                    else ReturnWithoutForward(Token::DIGIT)
                    break;
                case POINT:
                    if (isDigit(c))state = POINT;
                    else if (c == 'e')state = E1;
                    else ReturnWithoutForward(Token::DECIMAL)
                    break;
                case E1:
                    if (c == '-' || isDigit(c))state = E2;
                    else state = Err;
                    break;
                case E2:
                    if (isDigit(c))state = E2;
                    else ReturnWithoutForward(Token::DECIMAL)
                    break;
                case ID1:
                    if (isLetter(c))state = ID1;
                    else if (isDigit(c))state = ID2;
                    else {
                        auto res = Token::reservedToEnumMap.find(lexeme);
                        if (res == Token::reservedToEnumMap.end()) ReturnWithoutForward(Token::ID)
                        else ReturnWithoutForward(res->second)
                    }
                    break;
                case ID2:
                    if (isDigit(c))state = ID2;
                    else if (isLetter(c))state = ID1;
                    else {
                        auto res = Token::reservedToEnumMap.find(lexeme);
                        if (res == Token::reservedToEnumMap.end()) ReturnWithoutForward(Token::ID)
                        else ReturnWithoutForward(res->second)
                    }
                    break;
                case COM1:
                    if (c == '/')state = COM2;
                    else if (c == '*') {
                        state = CB1;
                    } else ReturnWithoutForward(Token::FDIV)
                    break;
                case COM2:
                    if (isNewLine(c))state = START;
                    else state = COM2;
                    break;
                case CB1:
                    if (c == '*')state = CB2;
                    else state = CB1;
                    break;
                case CB2:
                    if (c == '/') state = START;
                    else state = CB1;
                    break;
                case STR:
                    if (c == '"') ReturnWithForward(Token::STRING)
                    else state = STR;
                    break;
                case CHAR1:
                    state = CHAR2;
                    break;
                case CHAR2:
                    if (c == '\'') ReturnWithForward(Token::CHAR)
                    break;
                case L:
                    if (c == '>') ReturnWithForward(Token::NE)
                    else if (c == '=') ReturnWithForward(Token::LE)
                    else ReturnWithoutForward(Token::LT);
                    break;
                case E:
                    if (c == '<') ReturnWithForward(Token::LE)
                    else if (c == '=') ReturnWithForward(Token::EQ)
                    else ReturnWithoutForward(Token::ASSIGN)
                    break;
                case G:
                    if (c == '=') ReturnWithForward(Token::GE)
                    else ReturnWithoutForward(Token::GT);
                    break;
                case Err:
                    Logger::error(Position(beginX, beginY, x - beginX,source), "无法识别的标识符");
                    state = START;
                    stream.putback(c);
                    break;
            }
            lexeme.push_back(c);
            increaseX(c);
        }

    }

    void Lexer::printLexeme(Token nextToken, string action = "") {
        stringstream stream;
        stream << action;
        stream.width(18);
        stream.setf(ios::left);
        stream.fill(' ');
        stream << Token::reserved[(int)nextToken.kind] << " '";
        stream.width(30);
        stream.setf(ios::left);
        stream.fill(' ');
        stream << nextToken.lexeme << "'  " << nextToken.pos.getY() << ':' << nextToken.pos.getX() << endl;
        stream.unsetf(ios::left);
        Logger::dev(stream.str());
    }

    Lexer::Lexer(shared_ptr<Source> source) : source(std::move(source)) {
        Token nextToken, prvToken;
        bool next = true;
        map<Token::Enum, Token::Enum> merge = {
                {Token::if_,       Token::end_if},
                {Token::sub_,      Token::end_sub},
                {Token::function_, Token::end_function},
                {Token::module_,   Token::end_module},
                {Token::enum_,     Token::end_enum},
                {Token::class_,    Token::end_class},
                {Token::select_,   Token::end_select},
                {Token::type_,     Token::end_type},
                {Token::init,      Token::end_init},
                {Token::operator_, Token::end_operator},
                {Token::interface, Token::end_interface}
        };

        do {
            nextToken = LL();
            if (!tokens.empty() && tokens.back().kind == Token::end_ && merge.contains(nextToken.kind)) {
                auto pos = tokens.back().pos;
                tokens.pop_back();
                nextToken.kind = merge[nextToken.kind];
                nextToken.lexeme = string("end ") + nextToken.lexeme;
                nextToken.pos = Position(pos.getX(), pos.getY(), nextToken.pos.getW() + nextToken.pos.getX() - pos.getX(),this->source);
                tokens.push_back(nextToken);
            } else {
                tokens.push_back(nextToken);
            }
        } while (nextToken.kind != Token::EOF_);

        for (auto x:tokens) {
            printLexeme(x);
        }
    }


    const Token &Lexer::getNextToken() {
        return tokens[current_idx + 1];
    }

    const Token &Lexer::getToken() {
        if (current_idx == -1)throw "error";
        return tokens[current_idx];
    }

    void Lexer::match(Token::Enum token) {
        if (token == getNextToken().kind) {
            current_idx++;
            printLexeme(getToken(), "matched:");
        } else throw SyntaxException(getNextToken(), token);
    }

    void Lexer::match(initializer_list<Token::Enum> token_seq) {
        int count = 0;
        for (auto &token:token_seq) {
            if (token == getNextToken().kind) {
                current_idx++;
                count++;
                printLexeme(getToken(), "matched:");
            } else {
                auto &error = getNextToken();
                current_idx -= count;
                throw SyntaxException(error, token);
            }
        }
    }

    bool Lexer::match(Token::Enum token, set<Token::Enum> follows, string errorMessage) {
        if (token == getNextToken().kind) {
            current_idx++;
            return true;
        } else {
            Logger::error(getNextToken().pos, errorMessage);
            skipUntilFollow(follows);
            return false;
        }
    }

    void Lexer::skipUntilFollow(const set<Token::Enum> &follows) {
        if (follows.empty())throw "follows is empty";
        printFollows(follows, "Until");
        while (!follows.contains(getNextToken().kind)) {
            current_idx++;
            printLexeme(getToken(), "skip:");
            if (getToken().kind == Token::EOF_)throw "end of file";
        }
    }

    void Lexer::traceBack(int count) {
        if (current_idx - count < -1)throw "error";
        current_idx -= count;
    }

    void Lexer::forward(int count) {
        if (current_idx + count > tokens.size() - 1)throw "error";
        current_idx += count;
    }


    bool Lexer::match(initializer_list<Token::Enum> token_seq,
                      set<Token::Enum> follows, string errorMessage) {
        int count = 0;
        for (auto &token:token_seq) {
            if (token == getNextToken().kind) {
                current_idx++;
                count++;
                return true;
            } else {
                Logger::error(getNextToken().pos, errorMessage);
                current_idx -= count;
                skipUntilFollow(follows);
                return false;
            }
        }
        throw "error";
    }

    void printFollows(set<Token::Enum> follows, string name) {
        stringstream stream;
        stream << "follow(" << name << "):";
        for (auto x:follows)stream << Token::reserved[x] << ' ';
        stream << endl;
        Logger::dev(stream.str());
    }

    Position Position::accross(const Position &begin, const Position &end) {
        if(begin.y>end.y || begin.x>=end.ex || begin.source != end.source)throw "error";
        Position ret(begin.x,begin.y,end.ex,end.ey,begin.source);
        return ret;
    }

    const std::string &Source::operator[](int idx) {
        return getLine(idx);
    }

    const std::string &Source::getLine(int idx) {
        return lines[idx];
    }

    Source::Source(const std::string& file_path):stream(file_path),file_path(file_path){
        string str;
        while(getline(stream,str)){
            lines.push_back(str);
        }
        lines.emplace_back("");
        stream.clear();
        stream.seekg(ios::beg);
    }

    std::istream &Source::getStream() {
        return stream;
    }
}
