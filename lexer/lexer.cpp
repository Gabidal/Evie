#include "lexer.h"

namespace lexer{

    namespace cluster{
        group get_group(char c){
            for (auto g : groups){
                if (c >= g.min && c <= g.max){
                    return g;
                }
            }
        }
    }

    token::base* create(token::position position, token::types type, std::string& text){
        switch (type)
        {
        case token::types::TEXT:
            return new token::text(position, text);
        case token::types::OPERATOR:
            return new token::op(position, text);
        case token::types::SEPARATOR:
            return new token::separator(position, text);
        case token::types::NUMBER:
            return new token::number(position, text);
        case token::types::WRAPPER:
            return new token::wrapper(position, text);
        case token::types::CONTROL:
            return new token::control(position, text); 
        default:
            return new token::base(position, token::types::UNKNOWN);
        }
    }

    std::vector<token::base*> tokenize(std::string text, unsigned file_id){
        std::vector<token::base*> result;

        if (text.empty()){
            // TODO: log error
            return {};
        }

        cluster::group previous_group = cluster::get_group(text[0]);

        unsigned int previous_start_offset = 0;

        for (unsigned int i = 0; i < text.size(); i++){
            unsigned int current_offset = i - previous_start_offset;

            cluster::group current_group = cluster::get_group(text[i]);

            if (current_group.type != previous_group.type){
                result.push_back(create(
                    token::position(
                        previous_start_offset,
                        current_offset,
                        file_id
                    ),
                    previous_group.type,
                    text.substr(
                        previous_start_offset, 
                        current_offset
                    )
                ));

                previous_start_offset = i;
                previous_group = current_group;
            }

        }

        return result;
    }

}