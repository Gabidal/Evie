#include "lexer.h"

#include <limits>
#include <utility>

namespace lexer{

    namespace cluster{
        group get_group(char c){
            for (auto g : groups){
                if (c >= g.min && c <= g.max){
                    return g;
                }
            }

            return group(token::types::UNKNOWN, c, c);
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
    void slice_tokens(std::string text, unsigned file_id, std::vector<token::base*>& tokens, std::vector<unsigned int>& wrapper_indicies, std::vector<unsigned int>& number_indicies, std::vector<unsigned int>& newline_indicies){
        if (text.empty()) return;

        std::vector<std::pair<unsigned short, unsigned short>> coordinates(text.size());
        unsigned short line = 0;
        unsigned short column = 0;
        for (size_t idx = 0; idx < text.size(); ++idx){
            coordinates[idx] = { column, line };
            if (text[idx] == '\n'){
                column = 0;
                if (line < std::numeric_limits<unsigned short>::max()) ++line;
            } else {
                if (column < std::numeric_limits<unsigned short>::max()) ++column;
            }
        }

        cluster::group previous_group = cluster::get_group(text[0]);
        size_t previous_start_offset = 0;

        auto emit_token = [&](size_t end_offset){
            if (end_offset <= previous_start_offset) return;

            std::string subText = text.substr(previous_start_offset, end_offset - previous_start_offset);

            auto [start_column, start_line] = coordinates[previous_start_offset];

            token::base* current_token = create(
                token::position(
                    start_column,
                    start_line,
                    file_id
                ),
                previous_group.type,
                subText
            );

            if (current_token->get_type() == token::types::WRAPPER){
                wrapper_indicies.push_back(tokens.size());
            } else if (current_token->get_type() == token::types::NUMBER){
                number_indicies.push_back(tokens.size());
            } else if (current_token->get_type() == token::types::SEPARATOR && static_cast<token::separator*>(current_token)->type == token::separator::types::NEWLINE){
                newline_indicies.push_back(tokens.size());
            }

            tokens.push_back(current_token);
        };

        for (size_t i = 1; i < text.size(); ++i){
            cluster::group current_group = cluster::get_group(text[i]);

            if (current_group.type != previous_group.type || !previous_group.sticky){
                emit_token(i);
                previous_start_offset = i;
                previous_group = current_group;
            }
        }

        emit_token(text.size());
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
        
        for (unsigned int wrapper_start_index = 0; wrapper_start_index < wrapper_indicies.size(); wrapper_start_index++){
            int wrapper_nested_count = 0;

            char wrapper_start_identity = static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_start_index]])->identity;

            std::pair<char, char> wrap_condition;

            // find the right wrap condition based on the starting wrap_identity.
            for (auto pair : wrapper_pairs){
                if (pair.first == wrapper_start_identity){
                    wrap_condition = pair;
                    break;
                }
            }

            const bool symmetric_pair = wrap_condition.first == wrap_condition.second;

            for (unsigned int wrapper_end_index = wrapper_start_index + 1; wrapper_end_index < wrapper_indicies.size(); wrapper_end_index++){
                
                char wrapper_end_identity = static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_end_index]])->identity;

                // make sure that the ending wrap identity is of an right type.
                if (!symmetric_pair && wrapper_end_identity == wrap_condition.first){
                    // increase nested count.
                    wrapper_nested_count++;
                    continue;
                }

                // make sure that the ending wrap identity is of an right type.
                if (wrapper_end_identity == wrap_condition.second){
                    if (wrapper_nested_count > 0){
                        wrapper_nested_count--;
                        continue;
                    }

                    // now create an sub-copy of the wrapper_indicies
                    std::vector<unsigned int> sub_wrapper_indicies(
                        wrapper_indicies.begin() + wrapper_start_index + 1,
                        wrapper_indicies.begin() + wrapper_end_index
                    );

                    // now also make all the sub_wrapper_indices relative to the current wrapper_start_index as an zero origin
                    for (unsigned int i = 0; i < sub_wrapper_indicies.size(); i++){
                        sub_wrapper_indicies[i] -= wrapper_indicies[wrapper_start_index] + 1;
                    }

                    // now create an sub-copy of the tokens between the starting wrapped index and the end
                    std::vector<token::base*> sub_tokens(
                        tokens.begin() + wrapper_indicies[wrapper_start_index] + 1,
                        tokens.begin() + wrapper_indicies[wrapper_end_index]
                    );

                    // Deep copy the sub_tokens before assigning to wrapper to prevent dangling pointers
                    std::vector<token::base*> deep_copied_tokens;
                    deep_copied_tokens.reserve(sub_tokens.size());
                    for (const auto& token : sub_tokens) {
                        if (token) {
                            deep_copied_tokens.push_back(token->clone());
                        }
                    }
                    
                    // now call the wrap_tokens on the tokens of the wrapper
                    wrap_tokens(deep_copied_tokens, sub_wrapper_indicies);

                    static_cast<token::wrapper*>(tokens[wrapper_indicies[wrapper_start_index]])->tokens = deep_copied_tokens;

                    // now remove the copied tokens from the tokens list
                    for (unsigned int i = wrapper_indicies[wrapper_start_index] + 1; i < wrapper_indicies[wrapper_end_index]; i++) {
                        tokens[i]->redundant = true;
                    }

                    tokens[wrapper_indicies[wrapper_end_index]]->redundant = true; // ensure the closing wrapper is marked redundant

                    // tokens.erase(
                    //     tokens.begin() + wrapper_indicies[wrapper_start_index] + 1,
                    //     tokens.begin() + wrapper_indicies[wrapper_end_index]
                    // );

                    // remove the closing wrapper token itself
                    // tokens.erase(tokens.begin() + wrapper_indicies[wrapper_start_index] + 1);

                    // now remove the copied wrapper_indicies from the wrapper_indicies list
                    wrapper_indicies.erase(
                        wrapper_indicies.begin() + wrapper_start_index + 1,
                        wrapper_indicies.begin() + wrapper_end_index
                    );

                    // remove the closing wrapper index entry
                    wrapper_indicies.erase(wrapper_indicies.begin() + wrapper_start_index + 1);

                    // now break the inner loop
                    break;
                }

            }
        }
    }

    // this function will produce primitive patterns from the tokens.
    void combine_numerical_texts(std::vector<token::base*>& tokens, unsigned int /*file_id*/, const std::vector<unsigned int>& number_indicies){
        if (tokens.empty() || number_indicies.empty()) return;

        for (size_t idx = number_indicies.size(); idx > 0; --idx){
            size_t current_index_in_indices = idx - 1;
            size_t currentNumberIndex = number_indicies[current_index_in_indices];
            if (currentNumberIndex == 0 || currentNumberIndex >= tokens.size()) continue;

            token::base* current_token_base = tokens[currentNumberIndex];
            if (!current_token_base) continue;

            if (tokens[currentNumberIndex - 1]->get_type() == token::types::TEXT){
                token::number* number_token = static_cast<token::number*>(current_token_base);
                token::text* text_token = static_cast<token::text*>(tokens[currentNumberIndex - 1]);
                text_token->data += number_token->text;
                number_token->redundant = true;
            }

            if (current_index_in_indices == 0) continue;

            size_t previousNumberIndex = number_indicies[current_index_in_indices - 1];
            if (previousNumberIndex >= tokens.size() || previousNumberIndex >= currentNumberIndex || currentNumberIndex - previousNumberIndex != 2) continue;

            token::base* separator_candidate = tokens[currentNumberIndex - 1];
            if (separator_candidate->get_type() == token::types::OPERATOR && static_cast<token::op*>(separator_candidate)->text[0] == '.'){
                token::number* number_base = static_cast<token::number*>(tokens[previousNumberIndex]);
                number_base->number_type = token::number::types::FLOAT;
                number_base->text = number_base->text + "." + static_cast<token::number*>(current_token_base)->text;
                separator_candidate->redundant = true;
                current_token_base->redundant = true;
            }
        }
    }

    void combine_newlines(std::vector<token::base*>& tokens, const std::vector<unsigned int>& newline_indicies){
        if (tokens.empty() || newline_indicies.size() < 2) return;

        for (size_t idx = 1; idx < newline_indicies.size(); ++idx){
            unsigned int current_index = newline_indicies[idx];
            unsigned int previous_index = newline_indicies[idx - 1];
            if (current_index == previous_index + 1 && current_index < tokens.size()){
                tokens[current_index]->redundant = true;
            }
        }
    }

    void remove_redundant_tokens(std::vector<token::base*>& tokens){
        for (size_t i = 0; i < tokens.size();){
            if (tokens[i]->get_type() == token::types::WRAPPER){
                remove_redundant_tokens(static_cast<token::wrapper*>(tokens[i])->tokens);
            }

            if (tokens[i]->redundant){
                delete tokens[i];
                tokens.erase(tokens.begin() + i);
                continue;
            }

            ++i;
        }
    }

    std::vector<token::base*> tokenize(std::string text, unsigned file_id){
        std::vector<token::base*> tokens;

        if (text.empty()){
            // TODO: log error
            return {};
        }

        std::vector<unsigned int> wrapper_indicies;
        std::vector<unsigned int> number_indicies;
        std::vector<unsigned int> newline_indicies;

        slice_tokens(text, file_id, tokens, wrapper_indicies, number_indicies, newline_indicies);
        
        combine_numerical_texts(tokens, file_id, number_indicies);

        combine_newlines(tokens, newline_indicies);
        
        wrap_tokens(tokens, wrapper_indicies);

        remove_redundant_tokens(tokens);

        return tokens;
    }

    
}