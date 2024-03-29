#ifndef _COMPONENT_H_
#define _COMPONENT_H_
#include <string>
#include <vector>
#include "Position.h"
#include "../Flags.h"
using namespace std;

class Node;

class Component 
{
public:
    string Value;
    vector<Component> Components; // Tokens
    Node* node = nullptr;
    Position Location; // LineNumber
    long Flags;
    Component(string value, long flags) : Value(value), Flags(flags) {}
    Component(string value, const Position& position, long flags) : Value(value), Location(position), Flags(flags) {}
    Component(string value, long flag, vector<Component> C): Value(value), Components(C), Flags(flag) {}
    bool is(long flag)
    {
        return (Flags & flag) == flag;
    }
    bool Has(vector<long> f) {
        for (auto i : f)
            if (is(i))
                return true;
        return false;
    }
    vector<Component*> Get_all() {
        vector<Component*> Result;

        Result.push_back(this);

        for (auto& i : Components) {
            vector<Component*> tmp = i.Get_all();
            Result.insert(Result.end(), tmp.begin(), tmp.end());
        }

        return Result;
    }
    string To_String() {
        string Result = Value;

        char End = 0;

        if (Value[0] == '(' || Value[0] == '{' || Value[0] == '[' || Value[0] == '<') {
            Result = Value[0];

            if (Result[0] == '(')
                End = ')';
            else
                End = Result[0] + 2;
        }

        for (auto i : Components) {
            Result += i.To_String();
        }

        if (End != 0)
            Result += End;


        return Result;
    }
    Component* Copy_Component();
};

#endif