#include <iostream>
#include <iomanip>
#include <utils/format.h>
#include "token.h"
#include <utils/nullSafe.h>
using namespace std;
using namespace evoBasic::unicode;
namespace evoBasic{

    void Source::load(std::istream &stream){
        unicode::Utf8String line;
        while(getline(stream,line)){
            source_+=line;
            source_+='\n';
            codes.push_back(line);
        }
        codes.push_back("< end of file >"_utf8);
    }

    const unicode::Utf8String &Source::getLine(int idx) const {
        return codes[idx];
    }

    unicode::Utf8String &Source::getSource() {
        return source_;
    }


    FileSource::FileSource(const unicode::Utf8String &path)
            : stream(path.getBytes()),file_path(path) {
        load(stream);
    }


    const unicode::Utf8String &FileSource::getPath() const {
        return file_path;
    }


    StringSource::StringSource(unicode::Utf8String code){
        load(str_stream);
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

    unicode::Utf8String Location::toString() const {
        return Format() << '('
                        << "x:" << unicode::to_string(begin_x)
                        << ",y:" << unicode::to_string(begin_y)
                        << ",w:" << unicode::to_string(end_x - begin_x) << ')';
    }

    Source *Location::getSource() const {
        return source_;
    }


    map<unicode::Utf8String,Token::Enum> Token::reserved_words = {
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
        {"operator",Enum::operator_},
        {"impl",Enum::impl},
        {"extend",Enum::extend_},
        {"interface",Enum::interface_},
        {"new",Enum::new_},
        {"const",Enum::const_},
        {"paramarray",Enum::paramArray_}
    };

    vector<unicode::Utf8String> Token::enum_to_string = {
        "< Error >","ID","DIGIT","DECIMAL","STRING","CHAR","DOT","COMMA","COLON",
        "LE","GE","EQ","LT","GT","NE","LP","RP","LB","RB",
        "MUL","DIV","FDIV","MINUS","ADD","ASSIGN","< EOF >",
            /* reserved key word */
        "Dim","Module","Class","Public","Private","Friend","Protected","Static","Virtual","Override",
        "Function","Sub","Byval","Byref","Optional","As","Let","End","Enum","type",
        "Select","Case","For","To","Step","While","Wend","If","Then","ElseIf","Else",
        "And","Or","Xor","Not","Return","Continue","Exit","Next","False","True",
        "Import","Declare","Lib","Alias","Operator","Impl","Extend","Interface","New","Const","ParamArray",
            /* end domain token */
        "End If","End Function","End Sub","End Module","End Class","End type","End Enum",
        "End Select","End New","End Operator","End Interface"
    };

    Token::Token(Location location, Enum kind, unicode::Utf8String lexeme)
        : location_(location),kind_(kind),lexeme_(lexeme){}

    Location *Token::getLocation() {
        return &location_;
    }

    unicode::Utf8String &Token::getLexeme() {
        return lexeme_;
    }

    Token::Enum Token::getKind() {
        return kind_;
    }

    unicode::Utf8String Token::toString() {
        return Format() << setiosflags(ios::left)
                        << setw(15) << enum_to_string[(int)kind_]
                        << setw(20) << lexeme_
                        << setw(20) << location_.toString();
    }

    void Token::setKind(Token::Enum kind) {
        kind_ = kind;
    }

}
