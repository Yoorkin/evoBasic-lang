//
// Created by yorkin on 10/26/21.
//

#ifndef EVOBASIC2_TOKEN_H
#define EVOBASIC2_TOKEN_H
#include <string>
#include <map>
#include <vector>
#include <istream>
#include <fstream>
#include <sstream>

namespace evoBasic{

    class Source{
        std::vector<std::string> codes;
    protected:
        void load(std::istream &stream);
    public:
        const std::string& getLine(int idx)const;
        virtual std::istream &getStream() = 0;
    };

    class FileSource : public Source{
        std::string file_path;
        std::ifstream stream;
    public:
        explicit FileSource(const std::string& path);
        const std::string& getPath()const;
        std::istream &getStream()override;
    };

    class StringSource : public Source{
        std::istringstream str_stream;
    public:
        explicit StringSource(std::string code);
        std::istream &getStream()override;
    };

    class Location{
       int begin_x,end_x,begin_y,end_y;
       Source *source_ = nullptr;
    public:
        Location(int beginX,int endX,int y,Source *source);
        Location(const Location *begin,const Location *end);
        int getBeginX()const;
        int getBeginY()const;
        int getEndX()const;
        int getEndY()const;
        std::string toString()const;
        Source *getSource()const;
    };

    class Token{
    public:
        enum Enum{
            Error = 0,ID,DIGIT,DECIMAL,STRING,CHAR,DOT,COMMA,
            LE,GE,EQ,LT,GT,NE,LP,RP,LB,RB,
            MUL,DIV,FDIV,MINUS,ADD,ASSIGN,EOF_,
            /* reserved key word */
            dim_,module_,class_,public_,private_,friend_,protected_,static_,virtual_,override_,
            function_,sub_,byval_,byref_,optional_,as_,let_,end_,enum_,type_,
            select_,case_,for_,to_,step_,while_,wend_,if_,then_,elseif_,else_,
            and_,or_,xor_,not_,return_,continue_,exit_,next_,false_,true_,
            import_,declare_,lib_,alias_,operator_,impl,extend_,interface_,new_,const_,
            /* end domain token */
            END_IF,END_FUNCTION,END_SUB,END_MODULE,END_CLASS,END_TYPE,END_ENUM,END_SELECT,END_INIT,END_OPERATOR,END_INTERFACE,
        };
        static std::map<std::string,Enum> reserved_words;
        static std::vector<std::string> enum_to_string;
    private:
        std::string lexeme_;
        Location location_;

        Enum kind_;
    public:
        Token(Location location,Enum kind,std::string lexeme);
        Location* getLocation();
        const std::string& getLemexe()const;
        Enum getKind();
        void setKind(Token::Enum kind);
        std::string toString();
    };

}


#endif //EVOBASIC2_TOKEN_H
