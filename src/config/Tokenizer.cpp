#include "Tokenizer.hpp"

Tokenizer::Tokenizer(const std::string& input)
        : _input(input), _index(0), _line(1), _col(1)
{ }

void Tokenizer::skipWhiteSpaces() {
    while (_index < _input.size()) {
        std::size_t start = _input.find_first_not_of(" \t\v\f\r", _index); // \n
        if (start == std::string::npos)
            return ;
        if (_input[start] == '\n') {
            _line++;
            _col = 1;
            _index = start + 1;
            continue ;
        }
        _col += start - _index;
        _index = start;
        return ;
    }
}

void Tokenizer::skipComment() {
    std::size_t nl = _input.find_first_of("\n", _index);
    if (nl == std::string::npos) {
        _index = _input.size();
        return ;
    }
    _line++;
    _col = 1;
    _index = nl + 1;
}

Token Tokenizer::symbolToken(char c) {
    if (c == '{')
        return Token(TOK_LBRACE, "{", _line, _col) ;
    if (c == '}')
        return Token(TOK_RBRACE, "}", _line, _col) ;
    return Token(TOK_SEMICOLON, ";", _line, _col) ;
}

Token Tokenizer::wordToken() {
    std::string word;
    std::size_t nt = _input.find_first_of(" \t\v\f\r\n{};#", _index);
    if (nt == std::string::npos) {
        word = _input.substr(_index);
        _index = _input.size();
        return Token(TOK_WORD, word, _line, _col) ;
    }
    std::size_t tmp = _col;
    _col += nt - _index;
    word = _input.substr(_index, nt - _index);
    _index = nt;
    return Token(TOK_WORD, word, _line, tmp);
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    while (_index < _input.size()) {
        skipWhiteSpaces();
        if (_input[_index] == '\0')
            break ;
        if (_input[_index] == '#') {
            skipComment();
            continue ;
        }
        char c = _input[_index];
        if (c == '{' || c == '}' || c == ';') {
            tokens.push_back(symbolToken(c));
            _index++;
            _col++;
            continue ;
        }
        tokens.push_back(wordToken());
    }
    tokens.push_back(Token(TOK_EOF, "", _line, _col));
    return tokens;
}
