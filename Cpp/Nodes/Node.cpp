#include "../../H/Nodes/Node.h"
#include "../../H/Nodes/Token.h"
#include "../../H/Parser/Algebra.h"
#include "../../H/Parser/Parser.h"
#include "../../H/Docker/Mangler.h"
#include "../../H/UI/Safe.h"
#include "../../H/UI/Usr.h"
#include "../../H/Docker//Docker.h"

#include <vector>
#include <string>
#include <climits>

extern Usr* sys;

using namespace std;

void Node::Update_Defined_Stack_Offsets()
{
	//skip the returnin address.
	int Parameter_Offset = 8;
	int Local_Offset = 0;
	//first conmpute all the memory locaitions.
	for (auto& i : Defined) {
		if (i->is(OBJECT_NODE) || i->is(OBJECT_DEFINTION_NODE)) {
			//every local variable is defined default as a value in a register.
			if (i->Requires_Address) {
				i->Memory_Offset = Local_Offset/* + Size_of_Call_Space*/;
				Local_Offset += i->Size;
			}
		}
		if (i->is(PARAMETER_NODE)) {
			if (Token(i, true).is(TOKEN::REGISTER)) {
				//This happends when the parameter would be in a register but its in debug, so it uses stack
				//we need to tell the system that it CAN use the Local_Offset as storing the parameter
				i->Memory_Offset = Local_Offset/* + Size_of_Call_Space*/;
				Local_Offset += i->Size;
			}
			else if (!Token(i).is(TOKEN::REGISTER)) {
				//the parameters locate below the local variable space and the returning address 
				//also, do remeber that, the pushes that the code needs for the nonvolatiles
				//those come before the parameter space.
				i->Memory_Offset = Parameter_Offset/* + Size_of_Call_Space*/;
				Parameter_Offset += i->Size;
			}
		}
	}
}

Variable_Descriptor::Variable_Descriptor(Node* v, int i, vector<Node*> source) {
	Define_Index = i;
	Var = v;

	vector<Node*> Linear_Ast = v->Get_all({OBJECT_DEFINTION_NODE, OBJECT_NODE, PARAMETER_NODE});
	//for complex inlinings
	for (int n = i; n < source.size(); n++) {
		if (source[n]->is(ASSIGN_OPERATOR_NODE) || source[n]->is(POSTFIX_NODE)) {
			for (auto c : Linear_Ast) {
				if (source[n]->Left->Name == c->Name) {
					Expiring_Index = n;
					goto Stop;
				}
			}
		}
		if (source[n]->is(PREFIX_NODE)) {
			for (auto c : Linear_Ast) {
				if (source[n]->Right->Name == c->Name) {
					Expiring_Index = n;
					goto Stop;
				}
			}
		}
		if (source[n]->is(CALL_NODE)) {
			for (auto c : Linear_Ast) {
				for (auto p : source[n]->Function_Implementation->Parameters) {
					if (p->is("ptr"))
						if (p->Name == c->Name) {
							Expiring_Index = n;
							goto Stop;
						}
				}
			}
		}
		Expiring_Index = n;
	}
Stop:;
}

/// <summary>
/// 
/// </summary>
/// <param name="Skip_Prefixes">: Skip ptr/ref</param>
/// <param name="Get_Name">: include name</param>
/// <param name="Skip_Keywords"></param>
/// <returns></returns>
vector<string> Node::Get_Inheritted(bool Skip_Prefixes, bool Get_Name, bool Skip_Keywords) {
	vector<string> Result;
	//when using the address cast the inheritant of the casted is not changes so use that.
	if (Cast_Type != nullptr && Cast_Type->Name != "address") {
		if (MANGLER::Is_Base_Type(Cast_Type))
			Result = { Cast_Type->Name };
		else
			Result = Cast_Type->Get_Inheritted(Skip_Prefixes, Get_Name, Skip_Keywords);

		return Result;
	}
	else if (MANGLER::Is_Base_Type(this) || Get_Name) {
		return { Name };
	}
	else if (is(NUMBER_NODE)) {
		//1.29348
		if (find(Name.begin(), Name.end(), '.') != Name.end()) {
			if ((Name.end() - find(Name.begin(), Name.end(), '.')) <= 7)
				return Find(4, Global_Scope, "decimal")->Get_Inheritted(Skip_Prefixes, true);
			else
				return Find(8, Global_Scope, "decimal")->Get_Inheritted(Skip_Prefixes, true);
		}
		else {
			if (atoll(Name.c_str()) > INT_MAX) {
				return Find(8, Global_Scope, "integer")->Get_Inheritted(Skip_Prefixes, true);
			}
			return Find(4, Global_Scope, "integer")->Get_Inheritted(Skip_Prefixes, true);
		}
	}
	else {
		for (int i = 0; i < Inheritted.size(); i++) {
			if (Skip_Prefixes && ((Inheritted[i] == "ptr") || (Inheritted[i] == "ref")))
				continue;
			if (Skip_Keywords && Lexer::GetComponent(Inheritted[i]).is(::Flags::KEYWORD_COMPONENT) && Inheritted[i] != "ptr")
				continue;
			Result.push_back(Inheritted[i]);
		}

		return Result;
	}
}

string Node::Get_Inheritted(string Seperator, bool Skip_Prefixes, bool Get_Name, bool Skip_Keywords, bool Ignore_cast) {
	//when using the address cast the inheritant of the casted is not changes so use that.
	if (Cast_Type != nullptr && !Ignore_cast && Cast_Type->Name != "address") {
		string Result = "";

		if (MANGLER::Is_Base_Type(Cast_Type))
			Result = Seperator + Cast_Type->Name;
		else
			Result = Cast_Type->Get_Inheritted(Seperator, Skip_Prefixes, Get_Name, Skip_Keywords);

		return Result;
	}
	if (MANGLER::Is_Base_Type(this) || Get_Name) {
		return Seperator + Name;
	}
	else if (is(NUMBER_NODE) && Cast_Type == nullptr && !Ignore_cast) {
		//1.29348
		if (find(Name.begin(), Name.end(), '.') != Name.end()) {
			if ((Name.end() - find(Name.begin(), Name.end(), '.')) <= 7)
				return Find(4, Global_Scope, "decimal")->Get_Inheritted(Seperator, Skip_Prefixes, true);
			else
				return Find(8, Global_Scope, "decimal")->Get_Inheritted(Seperator,  Skip_Prefixes, true);
		}
		else {
			if (atoll(Name.c_str()) > INT_MAX) {
				return Find(8, Global_Scope, "integer")->Get_Inheritted(Seperator, Skip_Prefixes, true);
			}
			return Find(4, Global_Scope, "integer")->Get_Inheritted(Seperator, Skip_Prefixes, true);
		}
	}
	else if (is(NUMBER_NODE) && Cast_Type != nullptr && !Ignore_cast) {
		if (Cast_Type != nullptr && !Ignore_cast)
			return Seperator + Cast_Type->Name;
	}
	else {
		string Result = "";
		for (int i = 0; i < Inheritted.size(); i++) {
			if (Skip_Prefixes && ((Inheritted[i] == "ptr") || (Inheritted[i] == "ref")))
				continue;
			else if (Skip_Keywords && Lexer::GetComponents(Inheritted[i])[0].is(Flags::KEYWORD_COMPONENT))
				continue;
			Result += Seperator + Inheritted[i];
		}

		return Result;
	}
}

Node* Node::Get_Definition_Type(bool Ignore_Cast)
{
	//the operator flag is added for global variable operators.
	//The CALL_NODE represents the fetcher of amember that that call node returns as a type.
	if (!Has({ OBJECT_NODE, OBJECT_DEFINTION_NODE, PARAMETER_NODE, CONTENT_NODE, OPERATOR_NODE, CALL_NODE }))
		return this;

	vector<string> Right_Inherit = Get_Right_Inheritted(Ignore_Cast);

	if (Right_Inherit.size() == 0) {
		return this;
		Report(Observation(ERROR, "Parser could not give the inheritted, why tho?"));
	}

	//go through the inheritted and find the one that has the members init.
	for (auto i : Right_Inherit) {
		if (Lexer::GetComponent(i).is(Flags::KEYWORD_COMPONENT))
			continue;

		Node* scope = Fetcher;

		if (scope == nullptr)
			scope = Cast_Type;

		if (scope == nullptr)
			scope = Scope;

		//							  for cumulative types,
		Node* Inherit = Find(i, scope, {OBJECT_DEFINTION_NODE, CLASS_NODE, TEMPLATE_NODE});

		return Inherit;
	}

	return nullptr;
}

vector<string> Node::Get_Right_Inheritted(bool Ignore_Cast)
{
	vector<string> Result;

	if (is(CONTENT_NODE) && Childs.size() == 1) {
		Result = Childs[0]->Get_Right_Inheritted();
	}

	if (Fetcher && !Fetcher->is(CLASS_NODE)) {
		Result = Fetcher->Get_Right_Inheritted();
	}

	//this node is more closer to the right node that the fetcher.
	if (Inheritted.size() > 0)
		Result = Inheritted;

	if (!Ignore_Cast)
	//cast type is more closer to the right node than the og.
		if (Cast_Type) {
			//give the function the cast type by its self name so that
			//if the cast has its own cast it gets picket, if not then this cast type is picket
			//and not the cast type inheritted.
			vector<string> tmp = Cast_Type->Inheritted;
			Cast_Type->Inheritted = { Cast_Type->Name };
			Result = Cast_Type->Get_Right_Inheritted();
			Cast_Type->Inheritted = tmp;
		}

	return Result;
}

int Node::Calculate_Inheritted_Distance(Node* Val, Node* Loader, string type)
{
	int Val_Ptr_Count = Val->Get_All(type);
	int Loader_Ptr_Count = Loader->Get_All(type);

	//if there is ptr involved					&& if the ptr distance is more than 1 
	if ((Val_Ptr_Count + Loader_Ptr_Count > 0) && (abs(Val_Ptr_Count - Loader_Ptr_Count) > 2)) {
		if (Loader_Ptr_Count > Val_Ptr_Count && type == "ptr") {
			//this means that the Loader ptr count is greter than the Val ptr by *2
			Report(Observation(ERROR, "Cannot wrap " + Val->Name + to_string(abs(Val_Ptr_Count - Loader_Ptr_Count)) + " times into " + Loader->Name, *Val->Location));
		}
	}

	return abs(Val_Ptr_Count - Loader_Ptr_Count);
}

Node* Node::Find_Scope(Node* n)
{
	Node* Current_Scope = n->Scope;
	vector<Node*> Fetchers = n->Get_All_Fetchers();

	reverse(Fetchers.begin(), Fetchers.end());

	while (true) {
		for (auto& i : Current_Scope->Defined) {
			if (Fetchers.size() == 0)
				break;

			if (Fetchers.back()->Name == i->Name) {
				Current_Scope = i;
				Fetchers.pop_back();
			}
		}

		if (Fetchers.size() == 0)
			break;
		if (Current_Scope->Scope == nullptr)
			return nullptr;

		Current_Scope = Fetchers.back();
		Fetchers.pop_back();
	}

	Current_Scope = Current_Scope->Get_Definition_Type();

	return Current_Scope;
}

Node* is(Node* Result) {
	if (Result) {
		return Result;
	}

	//out of bounds
	else if (Result == nullptr) {
		return 0;
	}
}

#define TOO_BIG (Node*)1

#define TOO_SMOLL nullptr

//this algorithm is optimized by starting from low-hi.
//because this system ingnores automatically if the node is already beoynd the line and character number.
//Returns 1 if not found and returns 0 if the node is beyond the location.
vector<Node*> Find_Trace;
Node* Node::Find(Position& location)
{
	for (int j = 0; j < Find_Trace.size(); j++) {
		if (this == Find_Trace[j]) {
			return TOO_BIG;
		}
	}

	Find_Trace.push_back(this);

	if (Location->GetAbsolute() > location.GetAbsolute()) {
		Find_Trace.pop_back();
		return TOO_BIG;
	}
	else if (Location->GetAbsolute() < location.GetAbsolute()) {

		vector<Node*> Defined_Reversed = Defined;

		::sort(Defined_Reversed.begin(), Defined_Reversed.end(), [](Node* a, Node* b) { 
			return ((a->Location->GetFileIndex() > b->Location->GetFileIndex())
				|| (a->Location->GetFileIndex() == b->Location->GetFileIndex()
					&& a->Location->GetLine() > b->Location->GetLine()
					));

			//!((file1 > file2) || (file1 == file2 && line1 > line))
		});

		/*return += { 
			reverse(Defined.begin(), Defined.end());
		}*/
		for (auto& i : Defined_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		vector<Node*> Childs_Reversed = Childs; 
		
		::sort(Childs_Reversed.begin(), Childs_Reversed.end(), [](Node* a, Node* b) {
			return ((a->Location->GetFileIndex() > b->Location->GetFileIndex())
				|| (a->Location->GetFileIndex() == b->Location->GetFileIndex()
					&& a->Location->GetLine() > b->Location->GetLine()
					));

			//!((file1 > file2) || (file1 == file2 && line1 > line))
			});

		for (auto& i : Childs_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		vector<Node*> Parameter_Reversed = Parameters;
		reverse(Parameter_Reversed.begin(), Parameter_Reversed.end());

		for (auto& i : Parameter_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		if (Left) {
			Node* Result = Left->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Right) {
			Node* Result = Right->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Fetcher) {
			Node* Result = Fetcher->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Cast_Type) {
			Node* Result = Cast_Type->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
	}
	else {
		Find_Trace.pop_back();
		return this;
	}

	Find_Trace.pop_back();
	return TOO_SMOLL;
}

Node* Node::Find_Template(Node* T)
{
	for (auto i : Templates)
		if (i->Name == T->Name)
			return i;

	for (auto i : Inheritable_templates)
		if (i->Name == T->Name)
			return i;

	if (Scope)
		return Scope->Find_Template(T);

	return nullptr;
}

Node* Node::Find_Template(string T)
{
	for (auto i : Templates)
		if (i->Name == T)
			return i;

	for (auto i : Inheritable_templates)
		if (i->Name == T)
			return i;

	if (Scope)
		return Scope->Find_Template(T);

	return nullptr;
}

Node* Node::Find(bool Lock_Parent, Node* Memeber, Node* Parent)
{
	if (Parent->Defined.size() == 0) {
		Parent = Parent->Get_Definition_Type();
	}

	for (auto* i : Parent->Defined)
		if (Memeber->Name == i->Name)
			return i;
	return nullptr;
}

bool Node::Compare_Fetchers(Node* other)
{
	string This_Fethcers = "";
	for (auto i : Get_All_Fetchers()) {
		if (Find(i, Scope, CLASS_NODE) || (Scope->Name == i->Name && Scope->is(CLASS_NODE)))
			This_Fethcers += i->Name;
		else
			This_Fethcers += i->Get_Inheritted("", false, false, false);

		//multi dimensional templates are disabled
		for (auto j : i->Templates)
			This_Fethcers += j->Name;
	}
	if (this->Fetcher == nullptr)
		if (this->Has({ FUNCTION_NODE, CALL_NODE }))
			This_Fethcers += this->Name;

	string Other_Fethcers = "";
	for (auto i : other->Get_All_Fetchers()) {
		if (Find(i, Scope, CLASS_NODE) || (Scope->Name == i->Name && Scope->is(CLASS_NODE)))
			Other_Fethcers += i->Name;
		else
			Other_Fethcers += i->Get_Inheritted("", false, false, false);

		for (auto j : i->Templates)
			Other_Fethcers += j->Name;
	}
	if (other->Fetcher == nullptr)
		if (other->Has({ FUNCTION_NODE, CALL_NODE }))
			Other_Fethcers += other->Name;

	if (This_Fethcers == Other_Fethcers)
		return true;
	return false;
}

vector<Node*> Node::Get_All_Fetchers()
{
	vector<Node*> Result;
	if (Fetcher != nullptr) {
		vector<Node*> tmp = Fetcher->Get_All_Fetchers();
		Result.insert(Result.end(), tmp.begin(), tmp.end());
		Result.push_back(Fetcher);
	}
	//Result.push_back(this);
	return Result;
}

Node* Node::Get_Scope_As(int F, Node* scope, bool Must_Be_Found) {
	if (scope->is(F))
		return scope;
	if (scope->Scope != nullptr)
		return Get_Scope_As(F, scope->Scope, Must_Be_Found);
	if (Must_Be_Found)
		Report(Observation(ERROR, "Parent NULL!!", *Location));
	return nullptr;
}

Node* Node::Get_Scope_As(int F, vector<string> inheritted, Node* scope)
{
	if (!scope->is(F))
		return Get_Scope_As(F, inheritted, scope->Scope);

	for (auto i : inheritted)
		if (!scope->is(i))
			goto Not_Right_Scope;

	return scope;
	
	Not_Right_Scope:;
	return Get_Scope_As(F, inheritted, scope->Scope);
}

Node* Node::Get_Scope_As(vector<int> Flags, Node* Parent, bool Must_Be_Found)
{
	for (auto i : Flags) {
		if (Parent->is(i))
			return Parent;
	}

	if (Parent->Scope) {
		Node* scope = Get_Scope_As(Flags, Parent->Scope, Must_Be_Found);
		if (scope)
			return scope;
	}
	else if (Must_Be_Found)
		Report(Observation(ERROR, "Parent NULL!", *Location));

	return nullptr;
}

Node* Node::Get_Context_As(int F, Node* Context)
{
	if (Context->is(F))
		return Context;
	if (Context->Context)
		return Get_Context_As(F, Context->Context);
	return nullptr;
}

Node* Node::Get_Context_As(string n, Node* Context)
{
	if (Context->Name == n)
		return Context;
	if (Context->Context)
		return Get_Context_As(n, Context->Context);
	return nullptr;
}

/// <summary>
/// Gets a list of all the upper parents that this is defined in, up to global scope :D
/// </summary>
/// <returns></returns>
vector<Node*> Node::Get_Scope_Path(bool Include_Global_Scope)
{
	vector<Node*> Result;
	Node* Current_Scope = Scope;
	while (true) {

		if (Current_Scope->Name == "GLOBAL_SCOPE") {
			if (Include_Global_Scope) {
				Result.push_back(Current_Scope);
				break;
			}
			else
				break;
		}
		else {
			Result.push_back(Current_Scope);

			Current_Scope = Current_Scope->Scope;

			if (Current_Scope == nullptr)
				Report(Observation(ERROR, "Parental Scope was not found.", *this->Location));
		}
	}
	return Result;
}

Node* Node::Find(Node* n, Node* s, bool Get_Inheritted_Definition)
{
	Node* Const_Scope = s;

	//The 'n != s' is for this: 'n->Find(n, n);' to not to 
	bool Is_Valid_To_Check_Scope_Definition =
		n != s &&
		!s->Has({ OPERATOR_NODE, CONDITION_OPERATOR_NODE, BIT_OPERATOR_NODE, LOGICAL_OPERATOR_NODE, ASSIGN_OPERATOR_NODE, ARRAY_NODE, NODE_CASTER }) && 
		Get_Inheritted_Definition;

	if (s->Defined.size() == 0 && Is_Valid_To_Check_Scope_Definition) {
		//we need to ignore the casts that the scope might have, becasue something after must not 
		//affect something before
		Node* S = s->Get_Definition_Type(true);

		if (S) {
			s = S;
			if (n->Name == s->Name)
				return s;

			//if the parent is a content node that is also casted to Page ptr, this will have
			//transformed into _VIRTUAL_CLASS_PAGE_PTR_ type, which has the same base types but not same name.
			if (s->Get_Inheritted("_", false, false, true, true) == n->Get_Inheritted("_", false, false, true))
				for (auto i : s->Get_Inheritted(true, false, true)) {
					S = Find(i, s, { CLASS_NODE });
					if (S) {
						//If S is same named as n, then n is maybe trying to access .size attribute
						if (n->Name == S->Name)
							return S;
						s = S;
						break;
					}
				}
		}
	}

	//some criteria
	if (n->Name == "\n")
		return nullptr;
	if (n->is(NUMBER_NODE) || n->is(STRING_NODE) || n->is(LABEL_NODE))
		return n;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
	}

	if (n->is(TEMPLATE_NODE))
		for (auto& i : s->Templates)
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name)
					return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->Templates.size() == n->Templates.size())
			if (i->Name == n->Name) {
				if (n->Cast_Type != nullptr) {
					Node* tmp;
					i->Copy_Node(tmp, i, i->Scope);
					tmp->Cast_Type = n->Cast_Type;
					return tmp;
				}
				return i;
			}

	for (Node* i : s->Inlined_Items) {
		if (i->Templates.size() == n->Templates.size())
			if (i->Name == n->Name) {
				if (n->Cast_Type != nullptr) {
					Node* tmp;
					i->Copy_Node(tmp, i, i->Scope);
					tmp->Cast_Type = n->Cast_Type;
					return tmp;
				}
				return i;
			}
	}

	if (n->Fetcher != nullptr)
		if (n->Fetcher != s)
			for (auto& i : Find_Scope(n)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	if (s->Fetcher != nullptr)
		if (s->Fetcher != s->Scope)
			for (auto& i : Find_Scope(s)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr)
		if (Find(n, s->Scope) != nullptr)
			return Find(n, s->Scope);

	//IDK what this does, please explain!
	if (s->Cast_Type != nullptr)
		for (auto& i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name)
					return i;

	if (Get_Inheritted_Definition) {
		Node* Result = Find(n, Const_Scope, false);
		if (Result)
			return Result;
	}

	return nullptr;
}

Node* Node::Find(int size, Node* Parent, int flags, string format, bool Needs_To_Be_Base_Type) {
	if (Parent->Defined.size() == 0) {
		Node* S = Parent->Get_Definition_Type();

		if (S)
			Parent = S;
	}

	for (Node* i : Parent->Defined)
		if (i->is(flags) && (i->Size == size)) {
			i->Update_Format();
			if (i->Format == format)
				if (Needs_To_Be_Base_Type) {
					if (MANGLER::Is_Base_Type(i))
						return i;
				}
				else
					return i;
		}

	for (Node* i : Parent->Inlined_Items)
		if (i->is(flags) && (i->Size == size)) {
			i->Update_Format();
			if (i->Format == format)
				if (Needs_To_Be_Base_Type) {
					if (MANGLER::Is_Base_Type(i))
						return i;
				}
				else
					return i;
		}

	if (Parent->Scope != nullptr) {
		return Find(size, Parent->Scope, flags, format, Needs_To_Be_Base_Type);
	}

	return nullptr;
}

Node* Node::Find(Node* n, Node* s, int f, bool Get_Inheritted_Definition, bool Ignore_Parental_Defined)
{
	if (s->Defined.size() == 0 && Get_Inheritted_Definition) {
		Node* S = s->Get_Definition_Type();

		if (S) {
			s = S;

			if (n->Name == s->Name && s->is(f))
				return s;
		}
	}

	//some criteria
	if (n->Name == "\n")
		return nullptr;
	if (n->is(NUMBER_NODE) || n->is(STRING_NODE) || n->is(LABEL_NODE))
		return n;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
	}

	if (n->is(TEMPLATE_NODE))
		for (auto& i : s->Templates)
			if (i->is(n->Type))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->is(f))
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name) {
					if (n->Cast_Type != nullptr) {
						Node* tmp;
						i->Copy_Node(tmp, i, i->Scope);
						tmp->Cast_Type = n->Cast_Type;
						return tmp;
					}
					return i;
				}

	for (Node* i : s->Inlined_Items) {
		if (i->is(f))
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name) {
					if (n->Cast_Type != nullptr) {
						Node* tmp;
						i->Copy_Node(tmp, i, i->Scope);
						tmp->Cast_Type = n->Cast_Type;
						return tmp;
					}
					return i;
				}
	}

	if (n->Fetcher != nullptr && !Ignore_Parental_Defined)
		for (auto& i : Find_Scope(n)->Defined)
			if (i->is(f))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	if (s->Fetcher != nullptr && !Ignore_Parental_Defined) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(n, F, f, Get_Inheritted_Definition) != nullptr)
				return Find(n, F, f, Get_Inheritted_Definition);
	}

	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr && !Ignore_Parental_Defined)
		if (Find(n, s->Scope, f, Get_Inheritted_Definition) != nullptr)
			return Find(n, s->Scope, f, Get_Inheritted_Definition);

	//IDK what this does, please explain!
	if (s->Cast_Type != nullptr && s->Cast_Type->Name != n->Name)
			for (auto& i : s->Find(s->Cast_Type, s, CLASS_NODE, Get_Inheritted_Definition)->Defined)
				if (i->is(f))
					if (i->Templates.size() == n->Templates.size())
						if (i->Name == n->Name)
							return i;
	
	return nullptr;
}

Node* Node::Find(string name, Node* s, int flags, bool Get_Inheritted_Definition) {
	if (s->Defined.size() == 0 && Get_Inheritted_Definition) {
		Node* S = s->Get_Definition_Type();

		if (S) {
			s = S;

			if (name == s->Name)
				return s;
		}
	}
	
	if (name == "\n")
		return nullptr;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
		return nullptr;
	}

	for (auto& i : s->Templates)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	for (Node* i : s->Defined) {
		if (i->is(flags))
			if (i->Name == name)
				return i;
	}

	for (Node* i : s->Inlined_Items)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	if (s->Fetcher != nullptr) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(name, F, flags) != nullptr)
				return Find(name, F, flags);
	}

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, flags) != nullptr)
			return Find(name, s->Scope, flags);

	if (s->Cast_Type != nullptr && s->Cast_Type->Name != name && s->Find(s->Cast_Type, s, flags) != nullptr)
		for (auto& i : s->Find(s->Cast_Type, s, { CLASS_NODE, OBJECT_DEFINTION_NODE, OBJECT_NODE })->Defined)
				if (i->Name == name)
					return i;

	//if (s->Fetcher != nullptr) {
	//	Node* F = Find_Scope(s);
	//	if (F != nullptr)
	//		if (Find(name, F, flags) != nullptr)
	//			return Find(name, F, flags);
	//}
	return nullptr;
}

Node* Node::Find(string name, Node* s, bool Need_Parent_existence) {
	if (s->Defined.size() == 0) {
		Node* S = s->Get_Definition_Type();

		if (S) {
			s = S;

			if (name == s->Name)
				return s;
		}
	}
	
	if (name == "\n")
		return nullptr;
	if (s == nullptr /*&& Need_Parent_existance*/) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
		return nullptr;
	}

	for (auto& i : s->Templates)
		if (i->Name == name)
			return i;

	for (Node* i : s->Defined)
		if (i->Name == name)
			return i;

	for (Node* i : s->Inlined_Items)
		if (i->Name == name)
			return i;

	//List<T>.Foo(){}
	if (s->Fetcher != nullptr) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(name, F, Need_Parent_existence) != nullptr)
				return Find(name, F, Need_Parent_existence);
	}

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, Need_Parent_existence) != nullptr)
			return Find(name, s->Scope, Need_Parent_existence);

	if (s->Cast_Type != nullptr && s->Cast_Type->Name != name )
		for (auto& i : s->Find(s->Cast_Type, s, { CLASS_NODE, OBJECT_DEFINTION_NODE, OBJECT_NODE })->Defined)
				if (i->Name == name)
					return i;

	//if (s->Fetcher != nullptr) {
	//	Node* F = Find_Scope(s);
	//	if (F != nullptr)
	//		if (Find(name, F, Need_Parent_existence) != nullptr)
	//			return Find(name, F, Need_Parent_existence);
	//}
	return nullptr;
}

void Node::Get_Inheritted_Class_Members() {
	for (auto Inherit : Inheritted) {
		if (Lexer::GetComponent(Inherit).is(::Flags::KEYWORD_COMPONENT))
			continue;
		Node* Based = Find(Inherit, Scope);

		//Check if the Based class is also based checked
		if (!Based->is(PARSED_BY::NODE::GET_INHERITTED_MEMBERS)) {
			//update the base type
			Based->Get_Inheritted_Class_Members();
			Based->Set(PARSED_BY::NODE::GET_INHERITTED_MEMBERS);
		}

		vector<Node*> Inheritted_Members = Based->Defined;

		reverse(Inheritted_Members.begin(), Inheritted_Members.end());

		for (auto Member : Inheritted_Members)
			if (!Member->is("internal"))
				if (Locate(Member, Defined) == false)
					Defined.insert(Defined.begin(), Member);
	}
}

void Node::Update_Inheritance() {
	if (Cast_Type != nullptr)
		return;
	//save all keyword inheritances.
	vector<string> Keyword_Inheritance;
	for (auto i : Inheritted)
		if (Lexer::GetComponents(i)[0].is(Flags::KEYWORD_COMPONENT))
			Keyword_Inheritance.push_back(i);
	//remove all inheritances
	Inheritted.clear();

	Inheritted.push_back(Cast_Type->Name);
	Inheritted.insert(Inheritted.end(), Keyword_Inheritance.begin(), Keyword_Inheritance.end());
}

void Node::Transform_Dot_To_Fechering(Node* To)
{
	//(((A.B).C).D).Banana()
	//we start at the Dot that is left side of Banana()
	//((A.B).C).D | this == (..).D
	if (Name == ".") {
		//set the left side to To
		To->Fetcher = Right;
		if (Left->Name == ".") {
			Left->Transform_Dot_To_Fechering(Right);
		}
	}
	else
		To->Fetcher = this;
}

Node* Node::Get_Closest_Context(int Flags)
{
	if (!this)
		return nullptr;

	if (is(Flags))
		return this;

	Node* Closest_Context = Context->Get_Closest_Context(Flags);

	if (Closest_Context)
		return Closest_Context;

	return nullptr;
}

void Node::Clean()
{
	Header.clear();
	Defined.clear();
	Parameters.clear();
	Childs.clear();
	Inlined_Items.clear();
	Member_Functions.clear();
	Operator_Overloads.clear();
	Templates.clear();
	Inheritable_templates.clear();
	Template_Children.clear();
	Numerical_Return_Types.clear();
}

string Node::Print()
{
	string Result = "";

	if (is(CALL_NODE)) {
		Result += Get_Inheritted(" ", false, false, false) + " ";

		Result += Get_Uninitialized_Templates();

		Result += "(";

		for (int i = 0; i < Parameters.size(); i++) {
			string Seperator = ",";
			if (i == Parameters.size() - 1) {
				Seperator = ")";
			}
			Result += Parameters[i]->Get_Inheritted(" ", false, false, false) + Seperator;
		}
		if (Parameters.size() == 0)
			Result += ")";
	}
	return Result;
}

string Node::Get_Uninitialized_Templates()
{
	string Result = "";
	if (Templates.size() > 0)
		Result += Name + "<";
	for (int i = 0; i < (int)Templates.size() - 1; i++) {
		Result += Templates[i]->Get_Uninitialized_Templates() + ", ";
	}
	if (Templates.size() > 0)
		Result += Templates.back()->Get_Uninitialized_Templates() + ">";
	else
		Result += Name;
	return Result;
}

Component Node::Generate_Uninitialized_Template_Component(vector<Component> c)
{
	for (int i = 0; i < c.size(); i++) {
		if (c[i].is(Flags::TEMPLATE_COMPONENT) && i-1 >= 0 && c[i-1].is(Flags::TEXT_COMPONENT)) {
			//List<T, B>
			vector<Component> tmp = { c[i] };
			c[i - 1].Components = { Generate_Uninitialized_Template_Component(tmp) };
		}
		else if (c[i].is(Flags::TEMPLATE_COMPONENT)) {
			//<List<T>, B>
			for (int j = 0; j < c[i].Components.size(); j++) {
				vector<Component> tmp = { c[i].Components[j] };
				c[i].Components[j] = Generate_Uninitialized_Template_Component(tmp);
			}
		}
	}
	return c[0];
}

Node** Node::Get_Intepreted_Value(int i)
{
	for (auto &v : Values) {
		if (v.first == i)
			return v.second;
	}
	Node* Empty = new Node(NUMBER_NODE, new Position());
	Empty->Name = "0";
	Empty->Inheritted = Find(min(_SYSTEM_BIT_SIZE_, 4), this, CLASS_NODE, "integer", true)->Inheritted;
	Empty->Scope = this;

	Node** Handle = new Node*;
	Handle[0] = Empty;

	Values.push_back({ i, Handle });

	return Handle;
}

vector<string> Node::Get_Recursive_Inheritance(vector<string> Tray)
{
	vector<string> Result;
	for (auto i : Inheritted) {
		bool Already_Recorded = false;
		for (auto t : Tray) {
			if (i == t) {
				Already_Recorded = true;
			}
		}
		if (!Already_Recorded) {
			if (Lexer::GetComponent(i).is(Flags::KEYWORD_COMPONENT))
				continue;

			Result.push_back(i);

			Append(Result, Find(i)->Get_Recursive_Inheritance(Result));
		}
	}

	return Result;
}

void Node::Update_Members_To_New_Parent()
{
	for (auto& D : Defined) {
		D->Scope = this;
	}
	if (Left)
		Left->Context = this;
	if (Right)
		Right->Context = this;
	
	for (auto& P : Parameters) {
		if (this->is(CALL_NODE)) {
			P->Context = this;
		}
		else
			P->Scope = this;
	}
}

vector<Node*>& Node::Get_Template()
{
	if (Templates.size() > 0)
		return Templates;
	if (Fetcher)
		if (Fetcher->Inheritable_templates.size() > 0)
			return Fetcher->Inheritable_templates;
	return *(new vector<Node*>());
}

vector<Node*> Node::Get_Inheritted_Node_List()
{
	vector<Node*> Result;

	for (auto I : Inheritted) {
		if (Lexer::GetComponent(I).is(Flags::KEYWORD_COMPONENT))
			continue;

		Result.push_back(Find(I, this, CLASS_NODE));
	}

	return Result;
}

vector<Node*> Trace_Update_Size;
int Node::Update_Size() {
	if (is("internal") && Size != 0 || Is_Template_Object)
		return Size;

	for (int j = 0; j < Trace_Update_Size.size(); j++)
		if (this == Trace_Update_Size[j]) {
			//Trace_Update_Size.pop_back();
			if (is("ptr") || is("func"))
				return _SYSTEM_BIT_SIZE_;
			return Size;
		}

	Trace_Update_Size.push_back(this);

	Size = 0;
	//This can be activated by functions and classes.
	//We can skip ptr inheritted types.
	//Functions use the Inherit loop to determine the return size.
	//Classes use this to update their own functions or other members.
	//NOTE! Local or global variables cant use this, because local/global variables dont have direct access to their inheritted definitions.
	//In other words, local/global variables that inherit complex classes can't use this.
	if (!is(FUNCTION_NODE) && !is("ptr"))
		for (auto Member : Defined) {
			if (Member->Has({ FUNCTION_NODE, PROTOTYPE, IMPORT, EXPORT }))
				Member->Update_Size();
			else
				Size += Member->Update_Size();
		}

	vector<string> Inh = Inheritted;

	if (Cast_Type){
		if (MANGLER::Is_Base_Type(Cast_Type))
			Inh = {	Cast_Type->Name };
		else
			Inh = Cast_Type->Inheritted;
	}

	//this must be done after the members size are all set because the ptr will override them size.
	for (auto Inherit : Inh) {
		if (Lexer::GetComponent(Inherit).is(Flags::KEYWORD_COMPONENT)) {
			if (Inherit == "ptr" || Inherit == "func") {
				Size = _SYSTEM_BIT_SIZE_;
				break;
			}
			continue;
		}
		else //if (MANGLER::Is_Base_Type(Find(Inherit)))
			Size += Find(Inherit, this, { OBJECT_NODE, OBJECT_DEFINTION_NODE, PARAMETER_NODE, TEMPLATE_NODE, CLASS_NODE, /*FUNCTION_NODE, EXPORT, IMPORT*/ })->Update_Size();
	}
	Trace_Update_Size.pop_back();
	return Size;
}

vector<Node*> Trace_Update_Member_Variable;
void Node::Update_Member_Variable_Offsets(Node* obj) {

	for (int j = 0; j < Trace_Update_Member_Variable.size(); j++)
		if (obj == Trace_Update_Member_Variable[j]) {
			return;
		}

	Trace_Update_Member_Variable.push_back(obj);

	int Current_Offset = 0;
	for (auto i : obj->Defined) {
		if (i->is(FUNCTION_NODE))
			continue;

		if (!obj->is(FUNCTION_NODE)) {
			i->Memory_Offset = Current_Offset;
			Current_Offset += i->Size;
		}

		if (i->Defined.size() > 0) {
			Update_Member_Variable_Offsets(i);
		}
	}

	Trace_Update_Member_Variable.pop_back();
}

vector<Node*> Node::Get_All_Exept(vector<int> flags)
{
	vector<int> Flags;
	for (int i = 0; i < COUNT; i++) {
		bool Found = false;
		for (auto j : flags) {
			if (j == i) {
				Found = true;
				break;
			}
		}
		if (!Found)
			Flags.push_back(i);
	}


	/*vector<Node*> Result;
	for (auto i : Flags) {
		vector<Node*> tmp = Get_All_Exept(i, vector<Node*>());
		Result.insert(Result.end(), tmp.begin(), tmp.end());
	}*/
	return Get_All_Exept(Flags, vector<Node*>());
}

vector<Node*> Node::Get_All_Exept(vector<int> flags, vector<Node*> Trace)
{
	if (this->is(FUNCTION_NODE)) {
		bool Found = false;
		for (auto i : flags) {
			if (this->is(i)) {
				Found = true;
				break;
			}
		}

		if (Found) {
			return { this };
		}
		else
			return {};
	}

	for (int j = 0; j < Trace.size(); j++)
		if (this == Trace[j]) {
			Trace.pop_back();
			for (auto i : flags)
				if (this->is(i))
					return { new Node(*this) };
			return {};
		}

	bool Found = false;
	for (auto i : flags) {
		if (this->is(i)) {
			Found = true;
			break;
		}
	}

	if (!Found)
		return {};

	Trace.push_back(this);

	vector<Node*> Result;
	if (Left != nullptr) {
		vector<Node*> left = Left->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), left.begin(), left.end());
	}
	if (Right != nullptr) {
		vector<Node*> right = Right->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), right.begin(), right.end());
	}
	if (Succsessor != nullptr) {
		vector<Node*> Succsessors = Succsessor->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), Succsessors.begin(), Succsessors.end());
	}
	if (Predecessor != nullptr) {
		vector<Node*> Predecessors = Predecessor->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), Predecessors.begin(), Predecessors.end());
	}
	if (Fetcher != nullptr) {
		//vector<Node*> Fetchers = Fetcher->Get_all(f, Trace);
		//Result.insert(Result.end(), Fetchers.begin(), Fetchers.end());
		for (auto i : flags)
			if (Fetcher->is(i))
				Result.push_back(Fetcher);
	}
	for (Node* i : Header) {
		vector<Node*> Headers = i->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), Headers.begin(), Headers.end());
	}
	for (Node* i : Childs) {
		vector<Node*> childs = i->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Parameters) {
		vector<Node*> childs = i->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Defined) {
		vector<Node*> childs = i->Get_All_Exept(flags, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	if (Cast_Type) {
		//vector<Node*> childs = Cast_Type->Get_all(f, Trace);
		//Result.insert(Result.end(), childs.begin(), childs.end());
		for (auto i : flags)
			if (Cast_Type->is(i))
				Result.push_back(Cast_Type);
	}

	for (auto i : flags)
		if (is(i))
			Result.push_back(this);

	for (int i = 0; i < Result.size(); i++)
		for (int j = 0; j < Result.size(); j++)
			if (Result[i] == Result[j] && i != j)
				Result.erase(Result.begin() + j--);

	Trace.pop_back();

	return Result;
}

vector<Node*> Node::Get_all(int f, vector<Node*> Trace)
{
	if (this->is(FUNCTION_NODE))
		if (this->is(f) || f == -1)
			return { this };
		else
			return {};

	for (int j = 0; j < Trace.size(); j++)
		if (this == Trace[j]) {
			Trace.pop_back();
			if (this->is(f))
				return { new Node(*this) };
			return {};
		}

	Trace.push_back(this);

	vector<Node*> Result;
	if (Left != nullptr) {
		vector<Node*> left = Left->Get_all(f, Trace);
		Result.insert(Result.end(), left.begin(), left.end());
	}
	if (Right != nullptr) {
		vector<Node*> right = Right->Get_all(f, Trace);
		Result.insert(Result.end(), right.begin(), right.end());
	}
	if (Succsessor != nullptr) {
		vector<Node*> Succsessors = Succsessor->Get_all(f, Trace);
		Result.insert(Result.end(), Succsessors.begin(), Succsessors.end());
	}
	if (Predecessor != nullptr) {
		vector<Node*> Predecessors = Predecessor->Get_all(f, Trace);
		Result.insert(Result.end(), Predecessors.begin(), Predecessors.end());
	}
	if (Fetcher != nullptr) {
		//vector<Node*> Fetchers = Fetcher->Get_all(f, Trace);
		//Result.insert(Result.end(), Fetchers.begin(), Fetchers.end());
		//if (Fetcher->is(f))
		//	Result.push_back(Fetcher);
	}
	for (Node* i : Header) {
		vector<Node*> Headers = i->Get_all(f, Trace);
		Result.insert(Result.end(), Headers.begin(), Headers.end());
	}
	for (Node* i : Childs) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Parameters) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Defined) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	if (Cast_Type) {
		//vector<Node*> childs = Cast_Type->Get_all(f, Trace);
		//Result.insert(Result.end(), childs.begin(), childs.end());
		//if (Cast_Type->is(f))
		//	Result.push_back(Cast_Type);
	}

	if (is(f) || f == -1)
		Result.push_back(this);

	for (int i = 0; i < Result.size(); i++)
		for (int j = 0; j < Result.size(); j++)
			if (Result[i] == Result[j] && i != j)
				Result.erase(Result.begin() + j--);

	Trace.pop_back();

	return Result;
}

bool Filter_Exit_Code = false;
vector<Node*> Node::Get_all(int f, vector<Node*> Trace, bool(*Filter)(Node*))
{
	if (Filter(this) || Filter_Exit_Code) {
		Filter_Exit_Code = true;
		return { };
	}

	if (is({FUNCTION_NODE}))
		if (is(f) || f == -1)
			return { this };
		else
			return {};


	for (int j = 0; j < Trace.size(); j++)
		if (this == Trace[j]) {
			Trace.pop_back();
			if (this->is(f))
				return { new Node(*this) };
			return {};
		}

	Trace.push_back(this);

	vector<Node*> Result;
	if (Left != nullptr) {
		vector<Node*> left = Left->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), left.begin(), left.end());
	}
	if (Right != nullptr) {
		vector<Node*> right = Right->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), right.begin(), right.end());
	}
	if (Succsessor != nullptr) {
		vector<Node*> Succsessors = Succsessor->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), Succsessors.begin(), Succsessors.end());
	}
	if (Predecessor != nullptr) {
		vector<Node*> Predecessors = Predecessor->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), Predecessors.begin(), Predecessors.end());
	}
	if (Fetcher != nullptr) {
		//vector<Node*> Fetchers = Fetcher->Get_all(f, Trace);
		//Result.insert(Result.end(), Fetchers.begin(), Fetchers.end());
		if (Fetcher->is(f))
			Result.push_back(Fetcher);
	}
	for (Node* i : Header) {
		vector<Node*> Headers = i->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), Headers.begin(), Headers.end());
	}
	for (Node* i : Childs) {
		vector<Node*> childs = i->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Parameters) {
		vector<Node*> childs = i->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Defined) {
		vector<Node*> childs = i->Get_all(f, Trace, Filter);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	if (Cast_Type) {
		//vector<Node*> childs = Cast_Type->Get_all(f, Trace);
		//Result.insert(Result.end(), childs.begin(), childs.end());
		if (Cast_Type->is(f))
			Result.push_back(Cast_Type);
	}

	if (is(f) || f == -1)
		Result.push_back(this);

	for (int i = 0; i < Result.size(); i++)
		for (int j = 0; j < Result.size(); j++)
			if (Result[i] == Result[j] && i != j)
				Result.erase(Result.begin() + j--);

	Trace.pop_back();
	//reset the exit code.
	if (Trace.size() == 0)
		Filter_Exit_Code = false;

	return Result;
}

//			old  , new
vector<pair<Node*, Node*>> Trace;
void Node::Copy_Node(Node*& Result, Node* What_Node, Node* p)
{
	if (What_Node == nullptr) {
		Report(Observation(ERROR, "Code needs a fix"));
	}

	bool Fetcher_Has_Already_Been_Updated = false;

	//disable recursive funciton copying
	if (Trace.size() > 0)
		if (What_Node->is(FUNCTION_NODE)) {
			Result = What_Node;
			return;
		}


	for (int j = 0; j < Trace.size(); j++) {
		if (What_Node == Trace[j].first) {
			Result = Trace[j].second;
			return;
		}
	}

	if (What_Node == Result) {
		Result = Internal_Copy_Node(What_Node, p);

		if (Result->is(CALL_NODE))
			for (auto& P : Result->Parameters)
				P->Context = Result;

		if (Result->Left)
			Result->Left->Context = Result;

		if (Result->Right)
			Result->Right->Context = Result;

		if (Result->is(CONTENT_NODE))
			Result->Childs[0]->Context = Result;

		return;
	}

	//this will only copy the ptrs in list but we want to also copy what those ptr point to.
	Result = new Node(*What_Node);
	Result->Scope = p;

	Trace.push_back({ What_Node, Result });

	//lets start from defined
	for (int i = 0; i < Result->Defined.size(); i++)
		Copy_Node(Result->Defined[i], Result->Defined[i], Result);

	for (int i = 0; i < Result->Templates.size(); i++)
		Copy_Node(Result->Templates[i], Result->Templates[i], Result);

	for (int i = 0; i < Result->Childs.size(); i++)
		if (Result->is(CONTENT_NODE))
			Copy_Node(Result->Childs[i], Result->Childs[i], p);
		else
			Copy_Node(Result->Childs[i], Result->Childs[i], Result);

	for (int i = 0; i < Result->Member_Functions.size(); i++)
		Copy_Node(Result->Member_Functions[i], Result->Member_Functions[i], Result);

	for (int i = 0; i < Result->Operator_Overloads.size(); i++)
		Copy_Node(Result->Operator_Overloads[i], Result->Operator_Overloads[i], Result);

	for (int i = 0; i < Result->Parameters.size(); i++) {
		Node* scope = Result;
		if (Result->is(CALL_NODE))
			scope = Result->Scope;

		//If the fetcher is same as the first parameter update it aswell
		Node* tmp = Result->Parameters[i];

		Copy_Node(Result->Parameters[i], Result->Parameters[i], scope);

		if (Result->Fetcher == tmp) {
			Result->Fetcher = Result->Parameters[i];
			Fetcher_Has_Already_Been_Updated = true;
		}

		if (Result->is(CALL_NODE))
			Result->Parameters[i]->Context = Result;
	}

	for (int i = 0; i < Result->Header.size(); i++)
		Copy_Node(Result->Header[i], Result->Header[i], p);


	for (int i = 0; i < Result->Numerical_Return_Types.size(); i++)
		Copy_Node(Result->Numerical_Return_Types[i], Result->Numerical_Return_Types[i], p);

	if (Result->Left) {
		Copy_Node(Result->Left, Result->Left, p);
		Result->Left->Context = Result;
	}
	if (Result->Right) {
		Copy_Node(Result->Right, Result->Right, p);
		Result->Right->Context = Result;
	}

	if (Result->Succsessor)
		Copy_Node(Result->Succsessor, Result->Succsessor, p);
	if (Result->Predecessor)
		Copy_Node(Result->Predecessor, Result->Predecessor, p);

	if (Result->Fetcher && !Fetcher_Has_Already_Been_Updated)
		Copy_Node(Result->Fetcher, Result->Fetcher, p);


	if (Result->Succsessor) {
		Result->Succsessor->Predecessor = Result;
	}
	if (Result->Predecessor) {
		Result->Predecessor->Succsessor = Result;
	}

	if (Result->Location)
		Result->Location = new Position(*Result->Location);

	if (Result->Cast_Type)
		Copy_Node(Result->Cast_Type, Result->Cast_Type, p);

	//The copying prosess must go downwards not upwards, otherwise it will loop forever!
	//Result->Holder = Copy_Node(Result->Holder, p);

	Trace.pop_back();
	//now we have copyed every ptr into a new base to point.
	//return Result;
}


vector<pair<Node*, Node*>> Internal_Trace;
Node* Node::Internal_Copy_Node(Node* What_Node, Node* p)
{

	if (What_Node == nullptr)
		return nullptr;

	//disable recursive funciton copying
	if (Internal_Trace.size() > 0)
		if (What_Node->is(FUNCTION_NODE))
			return What_Node;

	for (int j = 0; j < Internal_Trace.size(); j++) {
		if (What_Node == Internal_Trace[j].first) {
			return Internal_Trace[j].second;
		}
	}

	//this will only copy the ptrs in list but we want to also copy what those ptr point to.
	Node* Result = new Node(*What_Node);
	Result->Scope = p;

	Internal_Trace.push_back({ What_Node, Result });

	//lets start from defined
	for (int i = 0; i < Result->Defined.size(); i++)
		Result->Defined[i] = Internal_Copy_Node(Result->Defined[i], Result);

	for (int i = 0; i < Result->Templates.size(); i++)
		Result->Templates[i] = Internal_Copy_Node(Result->Templates[i], Result);

	for (int i = 0; i < Result->Childs.size(); i++)
		if (Result->is(CONTENT_NODE))
			Result->Childs[i] = Internal_Copy_Node(Result->Childs[i], p);
		else
			Result->Childs[i] = Internal_Copy_Node(Result->Childs[i], Result);

	for (int i = 0; i < Result->Member_Functions.size(); i++)
		Result->Member_Functions[i] = Internal_Copy_Node(Result->Member_Functions[i], Result);

	for (int i = 0; i < Result->Operator_Overloads.size(); i++)
		Result->Operator_Overloads[i] = Internal_Copy_Node(Result->Operator_Overloads[i], Result);

	for (int i = 0; i < Result->Parameters.size(); i++) {
		Node* scope = Result;
		if (Result->is(CALL_NODE))
			scope = Result->Scope;

		Result->Parameters[i] = Internal_Copy_Node(Result->Parameters[i], scope);

		if (Result->is(CALL_NODE))
			Result->Parameters[i]->Context = Result;
	}

	for (int i = 0; i < Result->Header.size(); i++)
		Result->Header[i] = Internal_Copy_Node(Result->Header[i], p);


	for (int i = 0; i < Result->Numerical_Return_Types.size(); i++)
		Result->Numerical_Return_Types[i] = Internal_Copy_Node(Result->Numerical_Return_Types[i], p);

	if (Result->Left) {
		Result->Left = Internal_Copy_Node(Result->Left, p);
		Result->Left->Context = Result;
	}
	if (Result->Right) {
		Result->Right = Internal_Copy_Node(Result->Right, p);
		Result->Right->Context = Result;
	}

	Result->Succsessor = Internal_Copy_Node(Result->Succsessor, p);
	Result->Predecessor = Internal_Copy_Node(Result->Predecessor, p);

	if (Result->Fetcher)
		Result->Fetcher = Internal_Copy_Node(Result->Fetcher, p);


	if (Result->Succsessor) {
		Result->Succsessor->Predecessor = Result;
	}
	if (Result->Predecessor) {
		Result->Predecessor->Succsessor = Result;
	}

	if (Result->Location)
		Result->Location = new Position(*Result->Location);

	if (Result->Cast_Type)
		Result->Cast_Type = Internal_Copy_Node(Result->Cast_Type, p);

	//The copying prosess must go downwards not upwards, otherwise it will loop forever!
	//Result->Holder = Copy_Node(Result->Holder, p);

	Internal_Trace.pop_back();
	//now we have copyed every ptr into a new base to point.
	return Result;
}


void Node::Replace_Usage_Of(Node* Old, Node* New){
	if (Old->Context){

		if (Old->Context->Left == Old)
			Old->Context->Left = New;

		else if (Old->Context->Right == Old)
			Old->Context->Right = New;

		else if (Old->Context->Parameters.size() > 0)
			for (int i = 0; i < Old->Context->Parameters.size(); i++)
				if (Old->Context->Parameters[i] == Old)
					Old->Context->Parameters[i] = New;
	}

}

COMMENT::COMMENT(string raw) {
	if (raw.size() > 0) {
		regex expression("Depricated:.+");
		smatch matches;
		string Buffer = raw;
		if (regex_search(Buffer, matches, expression)) {
			for (auto i : matches) {
				Deprication_Information += i.str() + "\n";
			}
		}
	}
}

void Node::Modify_AST(Node*& n, bool(*Filter)(Node* n), function<void(Node*& n, unordered_set<Node*>& Trace)> Modifier, unordered_set<Node*>& Trace)
{
	if (Filter(n)) {
		Modifier(n, Trace);
	}

	for (auto& i : n->Childs) {
		i->Modify_AST(i, Filter, Modifier, Trace);
	}

	for (auto& i : n->Parameters) {
		i->Modify_AST(i, Filter, Modifier, Trace);
	}

	if (n->Left)
		n->Modify_AST(n->Left, Filter, Modifier, Trace);
	if (n->Right)
		n->Modify_AST(n->Right, Filter, Modifier, Trace);
	if (n->Cast_Type)
		n->Modify_AST(n->Cast_Type, Filter, Modifier, Trace);
}

vector<Node*> Node::Get_Context_Path(){
	vector<Node*> Result;
	Node* n = this;
	while (n) {
		Result.push_back(n);
		n = n->Context;
	}
	reverse(Result.begin(), Result.end());
	return Result;
}

string Node::Get_Name(){
	string Result = "";

	if (is(CONTENT_NODE)) {
		Result += "(";

		Result += Childs[0]->Get_Name();

		Result += ")";
	}
	if (Name == "+" || Name == "-"){
		Result += "(" + Left->Get_Name() + Right->Get_Name() + ")";
	}
	if (Coefficient){
		Result += Coefficient->Get_Name() + Name;
	}
	if (Order){
		Result = "(" + Result + ")^" + Order->Get_Name();
	}

	return Result;
}

vector<Node*> Node::Get_Adjacent_Coefficients()
{
	vector<Node*> Result;
	if (!this)
		return Result;
	if (Coefficient) {
		//tmp is to prevent the this pointer returning duplicating coefficients
		Node* tmp = Coefficient;
		Coefficient = nullptr;
		Result.push_back(this);
		DOCKER::Append(Result, tmp->Left->Get_Adjacent_Coefficients());
		DOCKER::Append(Result, tmp->Right->Get_Adjacent_Coefficients());
	}
	if (Name == "*") {
		DOCKER::Append(Result, Left->Get_Adjacent_Coefficients());
		DOCKER::Append(Result, Right->Get_Adjacent_Coefficients());
	}
}

Node* Node::Has(Node* n, unordered_set<Node*> trace){
	
	if (trace.find(this) != trace.end())
		return nullptr;

	trace.insert(this);
	
	if (this->Name == n->Name)
		if (this->Get_Inheritted() == n->Get_Inheritted())
			return this;

	if (this->is(CALL_NODE)){
		return this->Function_Implementation->Has(n, trace);
	} 

	for (auto& i : Childs) {
		Node* Result = i->Has(n, trace);
		if (Result)
			return Result;
	}

	for (auto& i : Parameters) {
		Node* Result = i->Has(n, trace);
		if (Result)
			return Result;
	}

	if (Left) {
		Node* Result = Left->Has(n, trace);
		if (Result)
			return Result;
	}
	if (Right) {
		Node* Result = Right->Has(n, trace);
		if (Result)
			return Result;
	}
	if (Cast_Type) {
		Node* Result = Cast_Type->Has(n, trace);
		if (Result)
			return Result;
	}

	return nullptr;


}

Node* Node::Has(Node* n){

	return Has(n, unordered_set<Node*>());

}