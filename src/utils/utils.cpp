#include "utils.h"

utils::linkable::~linkable() {
    disconnect();
}

void utils::linkable::connect(linkable* other) {
    if (this == other) return;
    
    // 1. Disconnect both from their current partners
    this->disconnect();
    if (other) other->disconnect();

    // 2. Link them
    if (other) {
        this->sibling = other;
        other->sibling = this;
    }
}

void utils::linkable::disconnect() {
    if (sibling) {
        sibling->sibling = nullptr; // Tell partner we are gone
        sibling = nullptr;          // Clear our pointer
    }
}

std::string utils::sanitize(std::string_view in) {
    /**
     * Removes quote characters from the provided string.
     *
     * Notes:
     * - Performs a single pass over the data using the standard erase/remove idiom.
     * - Does not trim whitespace or perform escaping; it only removes '"' and '\''.
     */
    std::string out(in);

    out.erase(
        std::remove_if(
            out.begin(),
            out.end(),
            [](unsigned char ch) { return ch == '"' || ch == '\''; }
        ),
        out.end()
    );

    return out;
}
