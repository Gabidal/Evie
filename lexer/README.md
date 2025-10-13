# What does it take?

### Currently Evie lexer can take as an input std::string with an file id.

# What limitations does it have?

### Cannot discern template notation: `<>` as one wrapper, this is done in the preprocessor stage after lexer.

# What are specialties to look out for?

### This lexer sees comments as normal wrappers just like parenthesis where the starting bracket is the `#` symbol and the ending bracket symbol is noted by the newline `\n` control character.

# What features does this lexer lack?

### As of now this lexer cannot understand Hexadecimal nor Octal or Binary values which are preceded by the notation `0x`, `0b`, `0c`.
