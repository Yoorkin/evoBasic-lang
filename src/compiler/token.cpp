#include<fstream>
#include <iostream>
#include <iomanip>
#include "formator.h"
#include "token.h"
#include "nullSafe.h"
using namespace std;
namespace evoBasic{

    void Source::load(std::istream &stream){
        string line;
        while(getline(stream,line)){
            codes.push_back(line);
        }
        codes.push_back("< end of file >");
        stream.clear();
        stream.seekg(ios::beg);
    }

    const std::string &Source::getLine(int idx) const {
        return codes[idx];
    }


    FileSource::FileSource(const string &path)
            : stream(path),file_path(path) {
        load(stream);
    }


    const std::string &FileSource::getPath() const {
        return file_path;
    }

    std::istream &FileSource::getStream() {
        return stream;
    }

    StringSource::StringSource(std::string code)
            : str_stream(code){
        load(str_stream);
    }

    std::istream &StringSource::getStream() {
        return str_stream;
    }


    Location::Location(int beginX, int endX, int y,Source *source) {
        NotNull(source);
        begin_x = beginX;
        end_x = endX;
        begin_y = end_y = y;
        source_ = source;
    }

    Location::Location(const Location *begin, const Location *end) {
        NotNull(begin);
        NotNull(end);
        NotNull(begin->source_);
        begin_x = begin->begin_x;
        begin_y = begin->begin_y;
        end_x = end->end_x;
        end_y = end->end_y;
        source_ = begin->source_;
    }

    int Location::getBeginX() const {
        return begin_x;
    }

    int Location::getBeginY() const {
        return begin_y;
    }

    int Location::getEndX() const {
        return end_x;
    }

    int Location::getEndY() const {
        return end_y;
    }

    std::string Location::toString() const {
        return format() << '('
        << "x:" << to_string(begin_x)
        << ",y:" << to_string(begin_y)
        << ",w:" << to_string(end_x - begin_x) << ')';
    }

    Source *Location::getSource() const {
        return source_;
    }


    map<string,Token::Enum> Token::reserved_words = {
        {"dim",Enum::dim_},
        {"module",Enum::module_},
        {"class",Enum::class_},
        {"public",Enum::public_},
        {"private",Enum::private_},
        {"friend",Enum::friend_},
        {"protected",Enum::protected_},
        {"static",Enum::static_},
        {"virtual",Enum::virtual_},
        {"override",Enum::override_},
        {"function",Enum::function_},
        {"sub",Enum::sub_},
        {"byval",Enum::byval_},
        {"byref",Enum::byref_},
        {"optional",Enum::optional_},
        {"as",Enum::as_},
        {"let",Enum::let_},
        {"end",Enum::end_},
        {"enum",Enum::enum_},
        {"type",Enum::type_},
        {"select",Enum::select_},
        {"case",Enum::case_},
        {"for",Enum::for_},
        {"to",Enum::to_},
        {"step",Enum::step_},
        {"while",Enum::while_},
        {"wend",Enum::wend_},
        {"if",Enum::if_},
        {"then",Enum::then_},
        {"elseif",Enum::elseif_},
        {"else",Enum::else_},
        {"and",Enum::and_},
        {"or",Enum::or_},
        {"xor",Enum::xor_},
        {"not",Enum::not_},
        {"return",Enum::return_},
        {"continue",Enum::continue_},
        {"exit",Enum::exit_},
        {"next",Enum::next_},
        {"false",Enum::false_},
        {"true",Enum::true_},
        {"import",Enum::import_},
        {"declare",Enum::declare_},
        {"lib",Enum::lib_},
        {"alias",Enum::alias_},
        {"init",Enum::init_},
        {"operator",Enum::operator_},
        {"impl",Enum::impl},
        {"extend",Enum::extend_},
        {"interface",Enum::interface_}
    };

    vector<string> Token::enum_to_string = {
        "< Error >","ID","DIGIT","DECIMAL","STRING","CHAR","DOT","COMMA",
        "LE","GE","EQ","LT","GT","NE","LP","RP","LB","RB",
        "MUL","DIV","FDIV","MINUS","ADD","ASSIGN","< EOF >",
            /* reserved key word */
        "Dim","Module","Class","Public","Private","Friend","Protected","Static","Virtual","Override",
        "Function","Sub","Byval","Byref","Optional","As","Let","End","Enum","type",
        "Select","Case","For","To","Step","While","Wend","If","Then","ElseIf","Else",
        "And","Or","Xor","Not","Return","Continue","Exit","Next","False","True",
        "Import","Declare","Lib","Alias","Init","Operator","Impl","Extend","Interface",
            /* end domain token */
        "End If","End Function","End Sub","End Module","End Class","End type","End Enum",
        "End Select","End Init","End Operator","End Interface"
    };

    Token::Token(Location location, Enum kind, std::string lexeme)
        : location_(location),kind_(kind),lexeme_(lexeme){}

    Location *Token::getLocation() {
        return &location_;
    }

    const std::string &Token::getLemexe() const {
        return lexeme_;
    }

    Token::Enum Token::getKind() {
        return kind_;
    }

    std::string Token::toString() {
        return format() << setiosflags(ios::left)
            << setw(15) << enum_to_string[(int)kind_]
            << setw(20) << lexeme_
            << setw(20) << location_.toString();
    }

    void Token::setKind(Token::Enum kind) {
        kind_ = kind;
    }

}
