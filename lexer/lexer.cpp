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

    // this function will slice the inputted text into simple word tokens.
    void slice_tokens(std::string text, unsigned file_id, std::vector<token::base*>& tokens, std::vector<unsigned int>& wrapper_indicies){
        // carry information about previous and current token states.
        cluster::group previous_group = cluster::get_group(text[0]);
        unsigned int previous_start_offset = 0;

        // this tokenizer stage there will NOT be any wrapped content. 
        for (unsigned int i = 0; i < text.size(); i++){
            unsigned int current_offset = i - previous_start_offset;

            cluster::group current_group = cluster::get_group(text[i]);

            if (current_group.type != previous_group.type){
                token::base* current_token = create(
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
                );

                // check if the new token is an wrap token, if so then add it to the wrapper_indicies
                if (current_token->get_type() == token::types::WRAPPER){
                    wrapper_indicies.push_back(tokens.size());
                }

                tokens.push_back(current_token);

                previous_start_offset = i;
                previous_group = current_group;
            }

        }
    }

    std::vector<std::pair<char, char>> wrapper_pairs = {
        { '(', ')' },
        { '[', ']' },
        { '{', '}' },
        { '"', '"' },
        { '\'', '\'' },
        { '#', '\n' }
    };

    // this function will wrap the tokens in the wrapper_indicies with the correct wrapper pairs.
    void wrap_tokens(std::vector<token::base*>& tokens, std::vector<unsigned int> wrapper_indicies){
        // go through wrapper_indicies and cross-reference the tokens at those indicies and check if wrapper pair found.
        // if an wrapper pair has been found then extract the wrapper_indicies between the two wrapper pair indicies and pass them again to this function.
        
        unsigned int wrapper_nested_count = 0;
        for (unsigned int wrapper_start_index = 0; wrapper_start_index < wrapper_indicies.size(); wrapper_start_index++){

            char wrapper_start_identity = static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_start_index]])->identity;

            std::pair<char, char> wrap_condition;

            // find the right wrap condition based on the starting wrap_identity.
            for (auto pair : wrapper_pairs){
                if (pair.first == wrapper_start_identity){
                    wrap_condition = pair;
                    break;
                }
            }

            for (unsigned int wrapper_end_index = wrapper_start_index + 1; wrapper_end_index < wrapper_indicies.size(); wrapper_end_index++){
                
                char wrapper_end_identity = static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_end_index]])->identity;

                // make sure that the ending wrap identity is of an right type.
                if (wrapper_end_identity == wrap_condition.first){
                    // increase nested count.
                    wrapper_nested_count++;
                    continue;
                }

                // make sure that the ending wrap identity is of an right type.
                if (wrapper_end_identity == wrap_condition.second){
                    // decrease nested count.
                    wrapper_nested_count--;
                
                    if (wrapper_nested_count == 0){

                        // now create an sub-copy of the wrapper_indicies
                        std::vector<unsigned int> sub_wrapper_indicies(
                            wrapper_indicies.begin() + wrapper_start_index + 1,
                            wrapper_indicies.begin() + wrapper_end_index
                        );

                        // now also make all the sub_wrapper_indices relative to the current wrapper_start_index as an zero origin
                        for (unsigned int i = 0; i < sub_wrapper_indicies.size(); i++){
                            sub_wrapper_indicies[i] -= wrapper_start_index + 1;
                        }

                        // now create an sub-copy of the tokens between the starting wrapped index and the end
                        std::vector<token::base*> sub_tokens(
                            tokens.begin() + wrapper_indicies[wrapper_start_index] + 1,
                            tokens.begin() + wrapper_indicies[wrapper_end_index]
                        );

                        // now call the wrap_tokens on the tokens of the wrapper
                        wrap_tokens(sub_tokens, sub_wrapper_indicies);

                        static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_start_index]])->tokens = sub_tokens;

                        // now remove the copied tokens from the tokens list
                        tokens.erase(
                            tokens.begin() + wrapper_indicies[wrapper_start_index] + 1,
                            tokens.begin() + wrapper_indicies[wrapper_end_index]
                        );

                        // now remove the copied wrapper_indicies from the wrapper_indicies list
                        wrapper_indicies.erase(
                            wrapper_indicies.begin() + wrapper_start_index + 1,
                            wrapper_indicies.begin() + wrapper_end_index
                        );

                        // now break the inner loop
                        break;
                    }
                }

            }
        }
    }

    // this function will produce primitive patterns from the tokens.
    void preprocess_tokens(std::vector<token::base*>& tokens, unsigned int file_id){
        // will create decimals from 
    }

    std::vector<token::base*> tokenize(std::string text, unsigned file_id){
        std::vector<token::base*> tokens;

        if (text.empty()){
            // TODO: log error
            return {};
        }

        std::vector<unsigned int> wrapper_indicies;

        slice_tokens(text, file_id, tokens, wrapper_indicies);
        
        wrap_tokens(tokens, wrapper_indicies);

        preprocess_tokens(tokens, file_id);

        return tokens;
    }

    
}