#ifndef   TOKENIZER_HPP
# define  TOKENIZER_HPP

# include <string>
# include <vector>
# include <cstddef>

enum TokenType {
    TOK_WORD,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_EOF
};

struct Token {
    TokenType   type;
    std::string value;
    std::size_t line;
    std::size_t col;

    Token();
    Token(TokenType tp, const std::string& vl, std::size_t ln, std::size_t cl) : 
            type(tp), value(vl), line(ln), col(cl) {}
};

class Tokenizer {
private:
    std::string _input;
    std::size_t _index;
    std::size_t _line;
    std::size_t _col;

    void        skipWhiteSpaces();
    void        skipComment();
    Token       symbolToken(char c);
    Token       wordToken();

public:
    Tokenizer(const std::string& input);
    std::vector<Token> tokenize();
};

#endif
