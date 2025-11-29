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
