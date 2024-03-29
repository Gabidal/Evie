#include "../../H/Parser/PostProsessor.h"
#include "../../H/Docker/Mangler.h"
#include "../../H/Parser/Parser.h"
#include "../../H/Lexer/Lexer.h"
#include "../../H/UI/Safe.h"
#include "../../H/UI/Usr.h"
#include "../../H/Docker/Docker.h"

#include <cstring>
#include <unordered_set>

#include "../../H/BackEnd/IRGenerator.h"

extern Node* Global_Scope;
extern Usr* sys;

void PostProsessor::Factory() {
	Transform_Component_Into_Node(); 

	if (!Ignore_Scope_Defined) {
		vector<Node*> All_Defined = Scope->Defined;
		Scope->Append(All_Defined, Scope->Inlined_Items);

		for (int i = 0; i < All_Defined.size(); i++) {
			//the prototypes needs the types to have sizes to determine the number parameters assosiative type.
			Type_Size_Definer(All_Defined[i]);
		}
		for (int i = 0; i < All_Defined.size(); i++) {
			//the prototypes needs the types to have sizes to determine the number parameters assosiative type.
			Handle_Imports(All_Defined[i]);
		}
		for (int i = 0; i < All_Defined.size(); i++) {
			Type_Definer(All_Defined[i]);
			Open_Namespace_For_Prosessing(All_Defined[i]);
		}
		for (auto& i : All_Defined) {
			//the prototypes needs the types to have sizes to determine the number parameters assosiative type.
			Member_Function_Defined_Outside(i);
			Member_Function_Defined_Inside(i);
			Open_Function_For_Prosessing(i);
		}
	}

	if (sys->Info.Is_Service && sys->Service_Info != Document_Request_Type::ASM)
		return;

	for (int i = 0; i < Input.size(); i++) {
		Cast(Input[i]);
		//Never ever duper gever seperate these two funktions from each others, or modify this order.
		Open_Paranthesis(i);
		Reduntant_Paranthesis_Cleaner(i);
		//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
		Operator_Overload(i);
		Open_Condition_For_Prosessing(i);
		Open_Loop_For_Prosessing(i);
		Combine_Member_Fetching(Input[i]);
		Go_Through_Un_Combined_Fetching(Input[i]);
		Determine_Return_Type(i);
		Determine_Array_Type(i);
		Open_Call_Parameters_For_Prosessing(i);
		Find_Call_Owner(Input[i]);

		Handle_Numbers(Input[i]);

		//String handlers
		Handle_String_Hexadecimals(Input[i]);
		Handle_String_Char_Numbers(Input[i]);
		Handle_Const_Char_Strings(Input[i]);
		Change_Local_Strings_To_Global_Pointters(i);

		//Why is this even a thing if Determine return type is a thing?
		//WHD? This function puts destructs in order if there is need for.
		Analyze_Return_Value(Input[i]);

		Increase_Calling_Number_For_Function_Address_Givers(Input[i]);
		Open_PreFix_Operator(i);
		Open_PostFix_Operator(i);
	}
	for (int i = 0; i < Input.size(); i++)
		Combine_Condition(i);
	for (int i = 0; i < Input.size(); i++)
		Move_Global_Varibles_To_Header(i);
}

void PostProsessor::Transform_Component_Into_Node()
{
	//<summary>
	//Extracts the Node ptr's from the component tokens
	//</summary>
	for (auto i : Components) {
		if (i.node == nullptr)
			continue;
		Node* n = i.node;
		Input.push_back(n);
	}
	return;
}

void PostProsessor::Type_Definer(Node* type)
{
	if (type->Type != CLASS_NODE)
		return;
	if (type->Templates.size() > 0)	//template types are constructed elsewhere.
		return;
	if (type->is(PARSED_BY::POSTPROSESSOR::TYPE_DEFINER))
		return;
	type->Set(PARSED_BY::POSTPROSESSOR::TYPE_DEFINER);

	//If this is a namespace skip the default constructor builder
	if (type->is("static")) {
		return;
	}

	//check for static members and move them into Header section to be labelazed
	for (auto& j : type->Childs)
		if (j->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, CONDITION_OPERATOR_NODE, BIT_OPERATOR_NODE })) {
			if (j->Left->is("static") && Node::Has(type->Header, j->Left) == false)
				type->Header.push_back(j);
		}
		else if (j->is("static") && Node::Has(type->Header, j) == false)
			type->Header.push_back(j);

	//infiltrate the class type and inject this behemoth
	Node* Type = type;
	if ((MANGLER::Is_Base_Type(Type) == false) && (!MANGLER::Is_Based_On_Base_Type(Type) || Type->Defined.size() > 0) && sys->Info.Reference_Count_Size > 0) {
		Node* Reference_Count = new Node(OBJECT_DEFINTION_NODE, Type->Location);
		Reference_Count->Name = "Reference_Count";
		Reference_Count->Scope = Type;

		Node* Size_Representer = Type->Find(sys->Info.Reference_Count_Size, Type->Get_Scope_As(CLASS_NODE, { "static" }, Type), CLASS_NODE, "integer", true);

		if (Size_Representer == nullptr) {
			Report(Observation(WARNING, "Cannot find suitable size type for the reference countter", *Type->Location));
			//we can still save this!
			Node* Size = new Node(OBJECT_DEFINTION_NODE, Type->Location);
			Size->Name = "size";
			Size->Size = sys->Info.Reference_Count_Size;
			Size->Inheritted.push_back("internal");

			Reference_Count->Defined.push_back(Size);
			Reference_Count->Inheritted.push_back("type");
		}
		else
			Reference_Count->Inheritted.push_back(Size_Representer->Name);

		Type->Defined.insert(Type->Defined.begin(), Reference_Count);
	}

	//DISABLE default constructor if user has already defined one.
	bool Constructor_Already_Defined = false;
	vector<Node*> All_Defined = Scope->Defined;
	Scope->Append(All_Defined, Scope->Inlined_Items);
	Scope->Append(All_Defined, type->Defined);
	for (auto j : All_Defined) {
		if (!j->is(FUNCTION_NODE))
			continue;
		if (j->Name != type->Name)
			continue;

		if (!j->is("ptr"))
			continue;	//constructor must return a ptr
		if (!j->is(type->Name))
			continue;	//constructor must return its self typed class type ptr.

		if (j->Parameters.size() != 1)
			continue;
		if (!j->Parameters[0]->is(type->Name))
			continue;
		if (!j->Parameters[0]->is("ptr"))
			continue;	//constructor must take itself as a ptr.
		Constructor_Already_Defined = true;	//the user has already defined the default constructor for us.
	}

	//make a default constructor.
	//insert the constructor into global scopes funciton list.
	Node* Function;
	Node* This;
	if (!Constructor_Already_Defined) {
		//Make the returning type to follow the template returning protocols.
		Component* Uninitialized_Templates;
		if (type->Is_Template_Object) {
			string Result = type->Get_Uninitialized_Templates();

			vector<Component> tmp = Lexer::GetComponents(Result);

			Node* tmp_Class = new Node(CLASS_NODE, new Position());
			tmp_Class->Templates = type->Templates;

			Parser p(tmp_Class);
			p.Input = tmp;
			p.Factory();

			Uninitialized_Templates = new Component(type->Generate_Uninitialized_Template_Component(p.Input));
		}


		Function = new Node(FUNCTION_NODE, type->Location);
		Function->Name = type->Name;
		if (type->Is_Template_Object) {
			Function->Inheritted = { "ptr" };
			Function->Un_Initialized_Template_Inheritance = { { *Uninitialized_Templates, 0 } };
		}
		else {
			Function->Inheritted = { type->Name, "ptr" };
		}

		Function->Scope = Scope->Get_Scope_As(CLASS_NODE, { "static" }, Scope);

		This = new Node(PARAMETER_NODE, type->Location);
		if (type->Is_Template_Object) {
			This->Inheritted = { "ptr" };
			This->Un_Initialized_Template_Inheritance = { { *Uninitialized_Templates, 0 } };
		}
		else {
			This->Inheritted = { type->Name, "ptr" };
		}
		This->Name = "this";
		//This->Defined = type->Defined;
		This->Scope = Function;
		This->Update_Size();

		Function->Parameters.push_back(This);
		Function->Defined.push_back(This);

		Function->Update_Size();

		if (type->Has_Inheritted({ "cpp", "evie", "vivid" }) != -1)
			Function->Inheritted.push_back(type->Inheritted[type->Has_Inheritted({ "cpp", "evie", "vivid" })]);

		if (!Constructor_Already_Defined) {
			type->Defined.push_back(Function);
		}
	}

	//constructed template classes are defined later on in the scope, so theyre memebrs are not defined properly
	for (auto& i : type->Defined) {
		Update_Template_Member_Members(i, type);
	}

	Destructor_Generator(type);

	if (!Constructor_Already_Defined) {
		PostProsessor p(type);
		Function->Childs = type->Childs;
		type->Childs.clear();
		//First make sure all the class intializations are member fetching combined.
		for (auto& i : Function->Childs) {
			p.Combine_Member_Fetching(i);
		}

		Function->Childs = p.Insert_Dot(Function->Childs, Function, This);

		//call all the inheritted default or overrided constructor calls.
		vector<Node*> tmp = Dottize_Inheritanse(type, This, Function);

		Function->Childs.insert(Function->Childs.begin(), tmp.begin(), tmp.end());

		//make the return of this pointter
		Node* ret = new Node(FLOW_NODE, type->Location);
		ret->Name = "return";
		ret->Right = new Node(*This);
		ret->Scope = Function;
		Function->Childs.push_back(ret);

		PostProsessor P(Function, Function->Childs);

		Function->Childs = P.Input;

		Scope->Append(Function->Childs, P.Output);
	}

	for (auto& j : type->Defined)
		if (j->is(FUNCTION_NODE) /* && (j->Parameters.size() == 0 || j->Parameters[0]->Inheritted[0] == Scope->Defined[i]->Name) */ ) {
			PostProsessor p(type);
			p.Member_Function_Defined_Inside(j);
			p.Member_Function_Defined_Outside(j);
		}
		else{
			// here we give the member of the class access to be allocated into memory, static or internal are ignored
			if (j->Inheritted.size() == 0 || !j->is("internal") || !j->is("static"))
				j->Requires_Address = true;
		}

	for (auto& j : type->Defined)
		if (j->is(FUNCTION_NODE)) {
			PostProsessor p(type);
			p.Open_Function_For_Prosessing(j);
		}


	return;
}

void PostProsessor::Destructor_Generator(Node* Type)
{
	return;
	if (sys->Info.Reference_Count_Size < 1)
		return;

	if (MANGLER::Is_Base_Type(Type) || (MANGLER::Is_Based_On_Base_Type(Type) && Type->Defined.size() == 0))
		return;
	//[type].Destructor(){
	//	if ([this] != 0->address && --[this].Reference_Count < 1){
	//		[member].Destructor()
	// 	}
	//  Deallocate<[type]>(this)
	//}

	string Result = "if (this != 0->address && --this.Reference_Count == 0){\n";

	//now we add the members that need to be revoked theyre destrucotrs
	for (auto Member : Type->Defined) {
		if (Member->is(FUNCTION_NODE))
			continue;
		if (!Member->is("ptr"))
			continue;

		//if the member is a base type pointter
		//char ptr -> Deallocate<char>(banana)
		if (MANGLER::Is_Based_On_Base_Type(Member)) {
			Result += "Deallocate<" + Member->Get_Inheritted((string)" ", true, false, true) + ">(this." + Member->Name + ")\n";
		}

		//if the memebr is not a base type
		//foo ptr m -> m.Destructor()
		else {
			Result += Member->Name + ".Destructor()\n";
		}
	}

	//deallocate one self
	Result += "Deallocate<" + Type->Name + ">(this)\n";

	Result += "}";
	
	//Construct the function that is conne host this Result string content
	Node* Destructor = new Node(FUNCTION_NODE, Type->Location);
	Destructor->Name = "Destructor";
	Destructor->Scope = Type;
	Destructor->Inheritted.push_back("func");

	Node* This = new Node(PARAMETER_NODE, Type->Location);
	This->Name = "this";
	This->Scope = Destructor;
	This->Inheritted = Type->Get_Inheritted();

	Destructor->Defined.push_back(This);
	Destructor->Parameters.push_back(This);

	//now we need to insert these new generated content to this new function
	vector<Component> Components = Lexer::GetComponents(Result);

	Parser p(Destructor);
	p.Input = Components;
	p.Factory();

	Destructor->Childs.push_back(p.Input[0].node);

	PostProsessor post(Destructor, Destructor->Childs);

	//Now that all procedures are finished we give this finished product to the type
	Type->Defined.push_back(Destructor);
}

void PostProsessor::Destructor_Caller(Node* v, vector<Node*> &childs)
{
	return;
	if (!v->is(OBJECT_DEFINTION_NODE))
		return;
	if (MANGLER::Is_Based_On_Base_Type(v))
		return;
	if (!v->is("ptr"))
		return;
	if (v->is(PARSED_BY::POSTPROSESSOR::DESTRUCTOR_CALLER))
		return;

	Parser p(v->Scope);
	p.Input = Lexer::GetComponents(v->Name + ".Destructor()");
	p.Factory();

	v->Set(PARSED_BY::POSTPROSESSOR::DESTRUCTOR_CALLER);

	PostProsessor P(v, p.Input);
	//v->Append(Output, P.Input);

	bool There_Is_No_User_Defined_Return = true;
	for (int i = childs.size()-1; i >= 0; i--) {
		if (childs[i]->Name == "return") {
			childs.insert(childs.begin() + i, P.Input.begin(), P.Input.end());
			There_Is_No_User_Defined_Return = false;
		}
	}

	if (There_Is_No_User_Defined_Return) {
		childs.insert(childs.end(), P.Input.begin(), P.Input.end());
	}
}

void PostProsessor::Open_Namespace_For_Prosessing(Node* n){

	if (!n->is("static"))
		return;

	if (!n->is(CLASS_NODE))
		return;

	if (n->is(PARSED_BY::POSTPROSESSOR::OPEN_NAMESPACE))
		return;

	n->Set(PARSED_BY::POSTPROSESSOR::OPEN_NAMESPACE);

	// for (auto& i : n->Defined) {
	// 	if (i->is(FUNCTION_NODE)) {
	// 		PostProsessor p(n);
	// 		p.Open_Function_For_Prosessing(i);
	// 	}
	// }

	PostProsessor p(n, n->Childs);

	//Create the initialization code to be given to global scope
	for (auto& i : n->Childs){

		for (auto& j : i->Get_all()){
			if (j->Has({NUMBER_NODE, OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE}))
				continue;
			if (j->Fetcher)
				continue;

			Node* Definition = j->Find(j, j);

			if (Definition->Scope != n)
				continue;

			
			j->Fetcher = n;

		}

	}

	Global_Scope->Childs.insert(Global_Scope->Childs.end(), n->Childs.begin(), n->Childs.end());

}

//"\xA1FF" -> "\1234"
void PostProsessor::Handle_String_Hexadecimals(Node* n)
{
	if (!n->is(STRING_NODE))
		return;

	string Result = "";

	for (int i = 0; i < n->Name.size(); i++) {

		char Value = n->Name[i];

		//"\xAFBG"
		if (i - 2 >= 0 && n->Name[i - 2] == '\\' && n->Name[i - 1] == 'x') {
			size_t Index = i;

			Value = stoi(n->Name.substr(Index), &Index, 16);

			i += Index - i;
		}

		Result += Value;
	}

	n->Name = Result;
}

//"\0" -> 0
void PostProsessor::Handle_String_Char_Numbers(Node* n)
{
	if (!n->is(STRING_NODE))
		return;

	string Content = n->Name.substr(1, n->Name.size() - 2);

	if (Content.size() == 0)
		return;

	long long Result = 0;

	if (Content[0] == '\\') {
		if (Content.size() < 2) {
			Report(Observation(ERROR, "Missing escape value after escape character '\\'", *n->Location, MISSING_CRITICAL_INFORMATION));
		}

		size_t Length = 0;

		Result = stoi(Content.substr(1), &Length, 10);

		n->Name = to_string(Result);

		n->Type = NUMBER_NODE;

		if (n->Cast_Type == nullptr) {
			//Here is a prophesi: this sizeof(char) is going to break the entire world.
			Node* Char = n->Find(sizeof(char), n, CLASS_NODE, "integer", true);

			n->Cast_Type = Char;
		}
	}


}

// "a" -> 48
void PostProsessor::Handle_Const_Char_Strings(Node* n)
{
	if (!n->is(STRING_NODE))
		return;

	if (n->Name.size() > 4)
		return;

	if (n->Context && IRGenerator::Get_Amount("ptr", n->Get_Pair()) > IRGenerator::Get_Amount("ptr", n))
		return;

	int Result = 0;

	constexpr int Byte = 8;

	string New_Name = n->Name.substr(1, n->Name.size() - 2);

	for (auto c : New_Name) {

		Result = Result << Byte;

		Result += c;
	}

	n->Name = to_string(Result);
	n->Size = 1;

	n->Type = NUMBER_NODE;

	if (n->Cast_Type == nullptr) {
		Node* Char = n->Find(n->Size, n, CLASS_NODE, "integer", true);

		n->Cast_Type = Char;
	}
}

vector<Node*> PostProsessor::Insert_Dot(vector<Node*> Childs, Node* Function, Node* This)
{
	vector<Node*> Result;
	for (auto Child : Childs) {
		Update_Operator_Inheritance(Child);
		//this is for the size and format to not be included in the default constructor.
		if (Child->is("internal") || Child->is("static"))
			continue;
		/*if (Child->is("static") != -1)
			continue;*/

		//this is for the class constructor to not include functions as a dottize operation.
		if (Child->is(FUNCTION_NODE))
			continue;

		//Because the child in question is the highest in the ast towards the function as a child.
		Node* Child_Copy;
		Child->Copy_Node(Child_Copy, Child, Function);

		vector<Node*> Objects = Child_Copy->Get_all({ OBJECT_DEFINTION_NODE, OBJECT_NODE, CALL_NODE }, [](Node* n) { if (n->Name == "this") return true; return false; });
		//insert this. infront of every member
		for (auto& Object : Objects) {
			//But why would there be a AST with a '.' node at this stage?
			//Well, it's because this may occur on the Class initialization
			//Altough functions have this covered, class body are not.
			//this.Next = previus.Next
			//{Next, Next}
			//the latter one is not 'this' owned
			Node** Handle = &Object;
			/*if (Object->Context && Object->Context->Name == ".") {
				Handle = &Object->Context;
				Combine_Member_Fetching(*Handle);
			}*/

			//Banana().Apple()
			//even tho Apple is called by Banana, Banana may be need of this pointter
			Node*& Dot_Needing_Object = Get_Possible_Fetcher(*Handle);

			if (Dot_Needing_Object->is(NUMBER_NODE) || Dot_Needing_Object->is(FUNCTION_NODE) || Dot_Needing_Object->is("internal") || Dot_Needing_Object->is("static") || MANGLER::Is_Base_Type(Dot_Needing_Object))
				continue;
			if ((Dot_Needing_Object->is(OBJECT_DEFINTION_NODE) || Dot_Needing_Object->is(OBJECT_NODE)) && This->Find(true, Dot_Needing_Object, This) && !Dot_Needing_Object->is(PARSED_BY::POSTPROSESSOR::THIS_AND_DOT_INSERTER)) {
				//Node* define = c->Find(linear_n, Function);

				Node* Dot = new Node(OPERATOR_NODE, Function->Location);
				Dot->Name = ".";
				Dot->Scope = Dot_Needing_Object->Scope;

				This->Copy_Node(Dot->Left, This, This->Scope);

				Dot->Right = new Node(*Dot_Needing_Object);
				Dot->Right->Set(PARSED_BY::POSTPROSESSOR::THIS_AND_DOT_INSERTER);

				Dot->Context = Dot_Needing_Object->Context;

				*Dot_Needing_Object = *Dot;
				Dot_Needing_Object->Update_Members_To_New_Parent();
			}
			else if (Dot_Needing_Object->is(CALL_NODE)) {
				//because this insert_dot is called upon a non hight AST member it needs to use the dynamic scope above.
				if (Dot_Needing_Object->Parameters.size() > 0) {
					//check if the current first parameter is the this pointer, if not. then insert one.
					bool Found = false;
					for (auto P : Dot_Needing_Object->Parameters[0]->Get_all({ OBJECT_NODE, OBJECT_DEFINTION_NODE, PARAMETER_NODE })) {
						if (P->Name == "this" && P->Context == Dot_Needing_Object) {
							Found = true;
							goto STOP;
						}
						else {
							//check if this current object has this call node as a member.
							Node* tmp = Dot_Needing_Object->Find(Dot_Needing_Object, P, FUNCTION_NODE, true, true);
							//check if this pointer isnt pointing to somewhere else: banana(this.size())
							if (P->Context == Dot_Needing_Object && tmp) {
								Found = true;
								goto STOP;
							}
						}
					}
				STOP:
					if (!Found && !Dot_Needing_Object->Fetcher) {
						//because of the callation complexity we need to emulate as if the this pointter has been already assigned to this callation node.
						//after emulation we need to make sure that noe emulation exess data is leaked.

						Dot_Needing_Object->Parameters.insert(Dot_Needing_Object->Parameters.begin(), This);
						Find_Call_Owner(Dot_Needing_Object, false);

						if (!Dot_Needing_Object->Function_Implementation) {
							Dot_Needing_Object->Parameters.erase(Dot_Needing_Object->Parameters.begin());
						}
					}
				}
				else if (!Dot_Needing_Object->Fetcher) {
					//because of the callation complexity we need to emulate as if the this pointter has been already assigned to this callation node.
					//after emulation we need to make sure that noe emulation exess data is leaked.

					Dot_Needing_Object->Parameters.insert(Dot_Needing_Object->Parameters.begin(), This);
					Find_Call_Owner(Dot_Needing_Object, false);

					if (!Dot_Needing_Object->Function_Implementation) {
						Dot_Needing_Object->Parameters.erase(Dot_Needing_Object->Parameters.begin());
					}
				}
				Dot_Needing_Object->Parameters = Insert_Dot(Dot_Needing_Object->Parameters, Dot_Needing_Object->Scope, This);
			}
		}

		//This is mainly for member funtions defined inside a class.
		//If there are user made 'this' usages then update all theyre inheritten because it's not made elsewhere. 
		Objects = Child_Copy->Get_all();
		for (auto& Object : Objects) {
			if (Object->Name == "this") {
				Object->Inheritted = This->Inheritted;
				Object->Size = This->Size;
			}
		}

		Result.push_back(Child_Copy);
	}
	return Result;
}

vector<Node*> PostProsessor::Dottize_Inheritanse(Node* Class, Node* This, Node* Funciton)
{
	vector<Node*> Result;
	for (auto i : Class->Inheritted) {
		if (Lexer::GetComponents(i)[0].is(Flags::KEYWORD_COMPONENT))
			continue;

		Node* Call = new Node(CALL_NODE, Class->Location);
		Node* New_This;
		This->Copy_Node(New_This, This, Funciton);
		Call->Parameters.push_back(New_This);

		Call->Parameters.back()->Context = Call;

		Call->Name = i;
		Call->Scope = Funciton;

		Result.push_back(Call);
	}
	return Result;
}

Node*& PostProsessor::Get_Possible_Fetcher(Node*& n)
{
	//This function returns the n if no fethcers found.
	Node** Result = &n;
	while ((*Result)->Fetcher) {
		Result = &(*Result)->Fetcher;
	}
	return *Result;
}

void Lambda(Node*& n, unordered_set<Node*>& Trace) {

	if (Trace.find(n) != Trace.end())
		return;

	Trace.insert(n);

	if (n->Fetcher) {
		n->Modify_AST(n->Fetcher, [](Node* n) { return true; }, Lambda, Trace);
	}

	if (n->Fetcher && n->Fetcher->Name == ".") {
		PostProsessor p(n->Fetcher);
		p.Combine_Member_Fetching(n->Fetcher);
	}
}

void PostProsessor::Go_Through_Un_Combined_Fetching(Node* n)
{
	unordered_set<Node*> Trace;

	n->Modify_AST(n, [](Node* n) { return true; }, Lambda , Trace);
}

void PostProsessor::Cast(Node* n)
{
	if (!n->is(NODE_CASTER))
		return;

	n->Left->Cast_Type = n->Right;
	n->Left->Cast_Type->Type = OBJECT_NODE;
	n->Left->Context = n->Context;

	*n = *n->Left;
}

void PostProsessor::Increase_Calling_Number_For_Function_Address_Givers(Node* n)
{
	if (!n->is(OBJECT_NODE))
		return;

	//If this object is same named as a function, but it is not a function address giver.
	//This missconseption can be solved by checking if the definition of this object has 'func ptr'.
	if (!n->is("func")){

		return;

	}

	Node* f;
	if (!n->Find(n->Name, n->Scope, FUNCTION_NODE)) {
		if (!n->Find(n->Name, n->Scope, IMPORT)) {
			if (!n->Find(n->Name, n->Scope, PROTOTYPE))
				return;
			else
				f = n->Find(n->Name, n->Scope, PROTOTYPE);
		}
		else
			f = n->Find(n->Name, n->Scope, IMPORT);
	}
	else
		f = n->Find(n->Name, n->Scope, FUNCTION_NODE);

	f->Calling_Count++;

	n->Function_Address_Giver = true;
}

void PostProsessor::Process_Function_Pointters(Node* scope)
{
	//first we need to identify all function pointters
	//the identification of a function pointter is:
	//func ptr X
	for (auto& Defined : scope->Defined) {
		if (!Defined->is(vector<string>{"func", "ptr"}))
			return;

			Defined->Function_Ptr = true;	
			
			//letus find the function adress giver
			for (int Line = 0; Line < scope->Childs.size(); Line++) {
				for (auto& Operator : scope->Childs[Line]->Get_all()) {//we dont want to filter the get all beacuse the get all will sort the returned items
					if (Operator->is(ASSIGN_OPERATOR_NODE) && Operator->Left->Name == Defined->Name && Operator->Right->is(OBJECT_NODE)) {
						if (Operator->Find(Operator->Right, Operator->Right)->Has({ IMPORT, FUNCTION_NODE })) {
							string Function_Name = Operator->Right->Name;
							Node* First_Call_Type = nullptr;
							//Now that we know the function name we need to know what function 
							//overload we want to call.
							for (int i = Line + 1; i < scope->Childs.size(); i++) {
								for (auto& j : scope->Childs[i]->Get_all()) {
									if (j->is(ASSIGN_OPERATOR_NODE) && j->Left->Name == Defined->Name)
										goto quit;

									if (j->is(CALL_NODE)) {
										Node* Call = new Node(CALL_NODE, j->Location);
										Call->Name = Function_Name;
										Call->Parameters = j->Parameters;
										Call->Scope = j->Scope;

										Update_Inheritance(j);
										Call->Inheritted = j->Inheritted;

										Find_Call_Owner(Call);

										j->Function_Implementation = Call->Function_Implementation;
										 
										if (First_Call_Type == nullptr)
											First_Call_Type = Call;
										else if (First_Call_Type->Function_Implementation != Call->Function_Implementation) {
											Node* Assing = new Node(ASSIGN_OPERATOR_NODE, j->Location);
											Assing->Name = "=";
											Assing->Scope = j->Scope;

											Assing->Left = new Node(*Defined);
											Assing->Right = new Node(OBJECT_NODE, j->Location);
											Assing->Right->Name = Function_Name;

											Assing->Left->Context = Assing;
											Assing->Right->Context = Assing;

											j->Header.push_back(Assing);
											goto quit;
										}

									}
								}
							}
						quit:;

							if (First_Call_Type) {
								Operator->Right->Function_Implementation = First_Call_Type->Function_Implementation;
							}
						}
						else {
							//this happends when a function pointter is passed to another one
							Report(Observation(ERROR, "YEET"));
						}
					}

				}
			}
	}


}

void PostProsessor::Member_Function_Defined_Outside(Node* f)
{
	//<summary>
	//	The function is in global scope's childs list-
	//	but it can be found in the respective scope that the fetchers describe.
	//	The Find function in Node.cpp can handle complex AST search to find the respective function.
	//	
	//	If the function in question is not a static type then we need to apply this pointters and other cool stuf.
	//</summary>
	if (f->Type != FUNCTION_NODE)
		return;
	if (f->is(PARSED_BY::POSTPROSESSOR::MEMBER_FUNCTION_DEFINED_OUTSIDE))
		return;
	if (f->Fetcher == nullptr)
		return;
	if (f->is("static"))
		return;

	Node* func = f;

	Node* Class = func->Find(func->Fetcher->Name, func, CLASS_NODE);

	//namespace functions still have the namespace as the fetcher.
	if (Class->is("static"))
		return;

	unordered_set<Node*> Trace;

	//Combine all the member fetchings that are not This based.
	func->Modify_AST(func, [](Node* n) { if (n->Name == ".") return true; return false; }, [](Node*& n, unordered_set<Node*>& Trace) {
		PostProsessor p(n);
		p.Combine_Member_Fetching(n);
	}, Trace);

	PostProsessor p(func);

	func->Childs = p.Insert_Dot(func->Childs, func, func->Parameters[0]);

	Node* Fetcher = func->Find_Scope(func);

	func->Fetcher = Fetcher;

	func->Set(PARSED_BY::POSTPROSESSOR::POSTPROSESSOR);
	func->Set(PARSED_BY::POSTPROSESSOR::MEMBER_FUNCTION_DEFINED_OUTSIDE);

	return;
}

void PostProsessor::Member_Function_Defined_Inside(Node* f)
{
	if (!f->is(FUNCTION_NODE))
		return;
	if (f->is("static"))
		return;
	if (Scope->is("static"))
		return;
	if (f->Scope->is("static"))
		return;
	if (f->Fetcher != nullptr)
		return;
	if (f->is(PARSED_BY::POSTPROSESSOR::MEMBER_FUNCTION_DEFINED_INSIDE))
		return;

	Node* func = f;

	Node* Class = func->Find(Scope->Name, func, CLASS_NODE);

	//Node* This = new Node(PARAMETER_NODE, "this", func->Location);
	//This->Inheritted = { Scope->Name, "ptr" };
	//This->Scope = func;
	//This->Size = _SYSTEM_BIT_SIZE_;
	////This->Defined = Class->Defined;
	//This->Inheritable_templates = Class->Inheritable_templates;

	//func->Defined.push_back(This);

	//func->Parameters.insert(func->Parameters.begin(), This);

	PostProsessor p(func);
	func->Childs = p.Insert_Dot(func->Childs, func, func->Parameters[0]);

	Node* scope = Scope->Find(Scope->Name, Scope, CLASS_NODE);

	func->Set(PARSED_BY::POSTPROSESSOR::POSTPROSESSOR);
	func->Set(PARSED_BY::POSTPROSESSOR::MEMBER_FUNCTION_DEFINED_INSIDE);

	return;
}

void PostProsessor::Update_Template_Member_Members(Node* n, Node* Class)
{
	//this n represents a member in a class that inherits a template type
	PostProsessor p(Scope);

	//no need to trace this, because the type_definer marks it's procecced
	for (auto inheritted : n->Inheritted) {
		if (Lexer::GetComponent(inheritted).is(Flags::KEYWORD_COMPONENT))
			continue;

		Node* Inheritted_Type = n->Find(inheritted, n, CLASS_NODE);

		//if this is the class constructor it inherits the class type.
		//this is to prevent looping infinitelly.
		if (Class->Name == Inheritted_Type->Name)
			continue;

		p.Type_Definer(Inheritted_Type);
	}
}

void PostProsessor::Open_Function_For_Prosessing(Node* f)
{
	//here we just go trugh the insides of the function
	//for optimization and other cool stuff :D
	if (!f->is(FUNCTION_NODE))
		return;
	if (f->Is_Template_Object)
		return;
	if (f->is(PARSED_BY::POSTPROSESSOR::OPEN_FUNCTION_FOR_PROCESSING))
		return;
	for (auto j : f->Get_All_Fetchers())
		if (j->Is_Template_Object)
			return;

	f->Set(PARSED_BY::POSTPROSESSOR::OPEN_FUNCTION_FOR_PROCESSING);
	/*for (auto j : Input[i]->Parameters)
		if (j->is("type") != -1)
			return;
	*/
	
	PostProsessor p(f);
	p.Input = f->Childs;

	//prepare the local variables
	p.Define_Sizes(f);

	f->Update_Format();

	f->Update_Size();

	Process_Function_Pointters(f);

	p.Factory();

	f->Childs = p.Input;

	Open_Safe({ f });

	//NOTE: This might not be able to detect nested scope members that might return
	//this implies that the nested scope needs to also deduce the destructors of the main scope defined
	for (auto& v : f->Defined)
		p.Destructor_Caller(v, f->Childs);

	/*if (!sys->Info.Is_Service || sys->Service_Info == Document_Request_Type::ASM)
		for (auto& v : f->Defined) {
			for (auto j : f->Childs) {
				Analyze_Variable_Address_Pointing(v, j);
				if (v->Requires_Address)
					break;
			}
		}*/

	//Define_Sizes(f);

	//Parent->Defined[i]->Update_Defined_Stack_Offsets();
	Scope->Append(f->Childs, p.Output);

	f->Set(PARSED_BY::POSTPROSESSOR::OPEN_FUNCTION_FOR_PROCESSING);
	f->Set(PARSED_BY::POSTPROSESSOR::POSTPROSESSOR);
	return;
}

void PostProsessor::Open_Condition_For_Prosessing(int i)
{
	if (!Input[i]->is(IF_NODE) && !Input[i]->is(ELSE_IF_NODE) && !Input[i]->is(ELSE_NODE))
		return;
	//this add the L number to it
	//Input[i]->Name += to_string(LNumber++);

	//preprare the local variables
	Define_Sizes(Input[i]);

	if (!Input[i]->is(ELSE_NODE)) {
		PostProsessor p(Input[i], Input[i]->Parameters);

		Input[i]->Parameters = p.Input;
	}

	Process_Function_Pointters(Input[i]);

	//here we now postprosess also the insides of the condition
	PostProsessor p(Input[i], Input[i]->Childs);

	Input[i]->Childs = p.Input;

	//Algebra_Laucher(Input[i], Input[i]->Childs);

	for (auto& v : Input[i]->Defined)
		p.Destructor_Caller(v, Input[i]->Childs);

	Scope->Append(Input[i]->Childs, p.Output);

	return;
}

void PostProsessor::Open_Paranthesis(int i)
{
	if (!Input[i]->is(CONTENT_NODE))
		return;

	PostProsessor p(Input[i], Input[i]->Childs);

	Input[i]->Childs = p.Input;

	for (auto& v : Input[i]->Defined)
		p.Destructor_Caller(v, Input[i]->Childs);

	Scope->Append(Input[i]->Childs, p.Output);

	if (Input[i]->Childs.back()->Cast_Type)
		Input[i]->Inheritted.push_back(Input[i]->Childs.back()->Cast_Type->Name);

	if (Input[i]->Inheritted.size() == 0)
		Input[i]->Inheritted = Input[i]->Childs.back()->Inheritted;
}

Node* Get_Last_Cast(Node* n){
	if (!n)
		return nullptr;

	Node* r = Get_Last_Cast(n->Cast_Type);
	if (r){
		return r;
	}

	return n;
}

void PostProsessor::Reduntant_Paranthesis_Cleaner(int i)
{
	if (!Input[i]->is(CONTENT_NODE))
		return;

	//((a + b)->(int ptr)->int)->address
	if (Input[i]->Childs.size() == 1 && Input[i]->Childs[0]->is(CONTENT_NODE)) {
		bool Parent_Has_Cast = Input[i]->Cast_Type;
		bool Child_Has_Cast = Input[i]->Childs[0]->Cast_Type;

		Node* Last_Cast = Get_Last_Cast(Input[i]->Childs[0]);

		Last_Cast->Cast_Type = Input[i]->Cast_Type;
		
		Input[i]->Childs[0]->Context = Input[i]->Context;
		Input[i] = Input[i]->Childs[0];
	}
}

void PostProsessor::Find_Call_Owner(Node* n, bool Stop)
{
	if (!n->is(CALL_NODE))
		return;
	if (n->Function_Implementation != nullptr)
		return;

	//stuff that this function chekcs
	// 
	// Differential parametric count analyzation
	// Differential return type analyzation
	// Differential template inserion analysis
	// Differential template class insertion analysis
	// 

	vector<pair<Node*, Node*>> Candidates;
	bool It_Is_A_Function_Pointter = false;
	bool Use_All_Scopes = false;

	while (n->Function_Implementation == nullptr) {
		Candidates = Find_Suitable_Function_Candidates(n, It_Is_A_Function_Pointter, Use_All_Scopes);
		int Note = Choose_Most_Suited_Function_Candidate(Order_By_Accuracy(Candidates, n), n, It_Is_A_Function_Pointter);

		// Note is 0 if found caller's function implemitation.
		// Note is 1 if constructed a new function implemitation based on the template types
		if (Note == 0 || Note == 1) {
			Upscale_Number_Parameter_Sizes(n);

			break;
		}
		//Stop if there is a problem, we use Stop = false when we want to ignore the error's
		//This is used in emulated situations, see Insert_Dot() function for more info.
		else if (Stop == false) {
			return;
		}
		// Note is -1 if nothing found
		else if (Note == -1) {
			if (Use_All_Scopes == false)
				Use_All_Scopes = true;
			else if (It_Is_A_Function_Pointter == false)
				It_Is_A_Function_Pointter = true;
			else
				Report(Observation(ERROR, "Cannot find function to call!", *n->Location));
		}
		// returns 2 if there is more than one suitable functions to call.
		else if (Note == 2) {
			Report(Observation(ERROR, "Cannot decide which function overload to call!", *n->Location));
		}
	}
	//the incerement of all func ptr candidates calling count happends at Choose_Most_Suited_Function_Candidate()
	if (It_Is_A_Function_Pointter == false) {
		//n->Function_Implementation->Calling_Count++;
		n->Inheritted = n->Function_Implementation->Inheritted;
	}
	else {
		if (n->is("ptr") && n->is("func")) {
			n->Size = n->Find(n, n)->Size;
			n->Inheritted = n->Find(n, n)->Inheritted;
		}
		else {
			Report(Observation(ERROR, "Can't find the function '" + n->Print() + "' calls to.", *n->Location));
		}
	}
}

//All numbers in evie will only occupie the smallest amount of size.
//Because of this we need to upscale number parameters to the call host size, so that the registers are fully cleared of extern values.
void PostProsessor::Upscale_Number_Parameter_Sizes(Node* caller){

	for (int i = 0; i < caller->Parameters.size(); i++){

		if (caller->Parameters[i]->is(NUMBER_NODE)){

			caller->Parameters[i]->Size = caller->Function_Implementation->Parameters[i]->Update_Size();

		}

	}

}

vector<pair<Node*, Node*>> PostProsessor::Find_Suitable_Function_Candidates(Node* caller, bool Skip_Name_Comparison, bool Use_All_Scopes)
{
	vector<pair<Node*, Node*>> Result;

	//first try to find the scope, by checking fetchers
	vector<Node*> Scopes;

	//String x
	//String y
	//Compare(x, y)
	//x is not a fetcher in this example but the Compare is still it's member function.
	Node* Fetcher = caller->Fetcher;
	//Checkif the first parameter could work as the fetcher
	if (Fetcher == nullptr && caller->Parameters.size() > 0 && !caller->Parameters[0]->is(NUMBER_NODE)) {

		//if the fetcher is residing inside the first parameter, then add it to Scopes.
		if (caller->Parameters[0]->Cast_Type && caller->Parameters[0]->Cast_Type->Name != "address") {
			Fetcher = Scope->Find(caller->Parameters[0]->Cast_Type, Scope, { CLASS_NODE, OBJECT_DEFINTION_NODE });

			//check if the first parameter is actually the fewtcher or just a ordinary parameter.
			//if so then nullify the fetcher node.
			if (MANGLER::Is_Base_Type(Fetcher)) {
				Node* tmp = Fetcher->Find(caller, Fetcher, { FUNCTION_NODE }, true, true);
				if (!tmp)
					Fetcher = nullptr;
			}

			if (Fetcher)
				Scopes.push_back(Fetcher);
		}

		//If the fetcher NOW is nullptr, it means that the first parameter was not the fetcher.
		if (!Fetcher)
			//We still will not give up, we will try to find the fetcher from the inheritance of the the first parameter.
			for (auto i : caller->Parameters[0]->Inheritted) {
				if (Lexer::GetComponent(i).is(Flags::KEYWORD_COMPONENT))
					continue;

				Fetcher = Scope->Find(i, Scope, { CLASS_NODE, OBJECT_DEFINTION_NODE });

				//Is the first parameters inheritance contains the caller, then set it as the fetcher.
				if (MANGLER::Is_Base_Type(Fetcher)) {
					Node* tmp = Fetcher->Find(caller, Fetcher, { FUNCTION_NODE }, true, true);
					if (!tmp)
						Fetcher = nullptr;
				}

				if (Fetcher)
					Scopes.push_back(Fetcher);
			}
	}

	if (Use_All_Scopes) {
		//This is used if a function is in globalscope but current scope is in different namespace.
		DOCKER::Append(Scopes, Scope->Get_Scope_Path(Use_All_Scopes));
	}
	else if (Fetcher) {
		//this is used to unwrap virtual inheritance, no need to loop, only single inheritance support atm.
	
		//loop through the fetchers inherits
		for (auto i : Fetcher->Inheritted) {
			if (Lexer::GetComponent(i).is(Flags::KEYWORD_COMPONENT))
				continue;

			Node* Inheritted = Scope->Find(i, Scope, { CLASS_NODE, OBJECT_DEFINTION_NODE });

			Scopes.push_back(Inheritted);
		}

		if (Scopes.size() == 0) {

			if (Fetcher->is("static")){

				//Fond the definition of the Fetcher
				Node* tmp = Fetcher->Find(Fetcher, Fetcher, { CLASS_NODE }, false);

				Scopes.push_back(tmp);

			}
			
		}

		
	}
	else if (caller->Get_Scope_As(CLASS_NODE, { "static" }, Scope) != Global_Scope) {
		Scopes.push_back(caller->Get_Scope_As(CLASS_NODE, {"static"}, Scope));
	}
	else {
		Scopes.push_back(Global_Scope);
	}

	string New_Name = "";
	bool Inherit_Templates = false;
	if (caller->Get_Template().size() > 0) {
		New_Name = caller->Construct_Template_Type_Name();
	}
	else {
		Inherit_Templates = true;
	}

	bool Can_Remove_Templates = false;

	//now that our scopes are ready to go, we can loop through them and find suitable candidates.
	for (auto Scope : Scopes) {
		if (Inherit_Templates && Scope->Inheritable_templates.size() > 0) {
			caller->Templates = Scope->Inheritable_templates;
			New_Name = caller->Construct_Template_Type_Name();
			Can_Remove_Templates = true;
		}

		vector<Node*> All_Defined = Scope->Defined;
		Scope->Append(All_Defined, Scope->Inlined_Items);

		for (auto Func : All_Defined) {
			bool Is_Similiar_To_Existing_Template_Func_That_Has_Been_Constructed = false;
			if (!Func->Has({ FUNCTION_NODE, PROTOTYPE, IMPORT, EXPORT }))
				continue;

			if (Scope == caller->Fetcher) {
				if (Func->Fetcher != nullptr) {
					//this function has been declared at global scope
					continue;
				}
			}

			//this part is skipped if the function is a fuinction pointer.
			if (!Skip_Name_Comparison) {
				//check if the names are smae
				if (Func->Name != caller->Name)
					//if there is a mangled template name
					if (New_Name != "") {
						//check the mangled template name
						if (New_Name != Func->Name)
							continue;
						else
							Is_Similiar_To_Existing_Template_Func_That_Has_Been_Constructed = true;
					}
					else
						continue;
				//Check if the templates are temporarly made for just this instance.
				else if (Func->Templates.size() == 0 && Can_Remove_Templates) {
					caller->Templates.clear();
				}
			}
			if (Func->Parameters.size() != caller->Parameters.size())
				continue;
			if (Func->Templates.size() != caller->Templates.size() && !Is_Similiar_To_Existing_Template_Func_That_Has_Been_Constructed)
				continue;

			Result.push_back({ Func, Scope });
		}
	}
	if (Can_Remove_Templates && caller->Templates.size() > 0) {
		caller->Templates.clear();
	}
	return Result;
}

map<int, vector<pair<pair<Node*, Node*>, Node*>>> PostProsessor::Order_By_Accuracy(vector<pair<Node*, Node*>> Candidates, Node* Caller)
{
	//save the candidatea by int accuracity
	map<int, vector<pair<pair<Node*, Node*>, Node*>>> Result;

	for (auto Candidate : Candidates) {
		//for every point the accuracity increases.
		int Accuracity = 0;

		Node* Func = Candidate.first;

		//banana<int>(...)
		//make the template ready for comparison.
		if (Candidate.first->Is_Template_Object) {
			Candidate.first->Copy_Node(Func, new Node(*Candidate.first), Candidate.second);

			for (int T = 0; T < Caller->Get_Template().size(); T++) {

				//T ptr banana<T>() -> int ptr banana<int>()
				for (auto& Return_Type : Func->Inheritted) {
					if (Return_Type == Func->Templates[T]->Name)
						Return_Type = Caller->Get_Template()[T]->Name;
				}

				//List<T> banana<T>() -> List<int> banana<int>()
				for (auto& Return_Type : Func->Un_Initialized_Template_Inheritance) {
					for (auto& Template : Return_Type.first.Components[0].Get_all()) {
						if (Template->Value == Func->Templates[T]->Name)
							Template->Value = Caller->Get_Template()[T]->Name;
					}
				}

				//func banana<T>(T a)
				for (auto& Parameter : Func->Parameters) {
					for (auto& Inherit : Parameter->Inheritted) {
						if (Inherit == Func->Templates[T]->Name)
							Inherit = Caller->Get_Template()[T]->Name;
					}
				}

				//func banana<T>(List<T> a)
				for (auto& Parameter : Func->Parameters) {
					for (auto& Template : Parameter->Un_Initialized_Template_Inheritance) {
						for (auto& Nested_Template : Template.first.Get_all()) {
							if (Nested_Template->Value == Func->Templates[T]->Name)
								Nested_Template->Value = Caller->Get_Template()[T]->Name;
						}
					}
				}
			}

			//Now flatten the Un_initialized_Templates list into Inheritances
			for (auto Return_Type : Func->Un_Initialized_Template_Inheritance) {
				//construct a plain string from the component.
				string Flatten = Return_Type.first.To_String();

				//insert the flatten text into parser that there is a class to reference to.
				Parser p(Scope);
				p.Input = Lexer::GetComponents(Flatten);
				p.Factory();

				Func->Inheritted.insert(Func->Inheritted.begin() + Return_Type.second, Flatten);
			}

			//Now flatten the Un_Initialized_Templates from Parameters
			for (auto Parameter : Func->Parameters) {
				for (auto Return_Type : Parameter->Un_Initialized_Template_Inheritance) {
					//construct a plain string from the component.
					string Flatten = Return_Type.first.To_String();

					//insert the flatten text into parser that there is a class to reference to.
					Parser p(Scope);
					p.Input = Lexer::GetComponents(Flatten);
					p.Factory();

					if (p.Input.size() > 1) {
						Report(Observation(ERROR, "Tell Gabe to fix this!", "Internal"));
					}

					Parameter->Inheritted.insert(Parameter->Inheritted.begin() + Return_Type.second, p.Input.back().Value);
				}
			}
		}

		//dont worry about pointter amount, we will check them in another function-
		//that is to run after this function.
		if (Func->Get_Inheritted((string)"_", true, false, true) == Caller->Get_Inheritted((string)"_", true, false, true))
			Accuracity++;

		//checks basetype casting in parameters.
		bool All_Parameters_Match = true;
		for (int i = 0; i < Func->Parameters.size(); i++) {
			if (Func->Parameters[i]->Get_Inheritted((string)"_", true, false, true) != Caller->Parameters[i]->Get_Inheritted((string)"_", true, false, true))
				for (int j = 0; j < Func->Parameters[i]->Inheritted.size(); j++)
					if (!Lexer::GetComponent(Func->Parameters[i]->Inheritted[j]).is(Flags::KEYWORD_COMPONENT))
						//banana --> fruit
						if (!Find_Castable_Inheritance(Caller->Parameters[i]->Get_Inheritted(true, false), Func->Parameters[i]->Inheritted[j]))
							//fruit --> banana
							for (int Caller_i = 0; Caller_i < Caller->Parameters[i]->Get_Inheritted(true, false).size(); Caller_i++)
								if (!Lexer::GetComponent(Caller->Parameters[i]->Get_Inheritted(true, false)[Caller_i]).is(Flags::KEYWORD_COMPONENT))
									if (!Find_Castable_Inheritance(Func->Parameters[i]->Get_Inheritted(true, false), Caller->Parameters[i]->Get_Inheritted(true, false)[Caller_i])) {
										All_Parameters_Match = false;
										goto Break_Parameter_Check;
									}
		}
		Break_Parameter_Check:

		if (All_Parameters_Match)
			Accuracity++;

		//check if the function is already constructed for this caller.
		if (Caller->Templates.size() > 0) {
			string New_Name = Caller->Construct_Template_Type_Name();
			if (New_Name == Func->Name)
				Accuracity++;
		}

		Result[Accuracity].push_back({ {Candidate.first, Func}, Candidate.second });

	}
	return Result;
}

// returns 0 if found caller's function implemitation.
// returns 1 if constructed a new function implemitation based on the template types
// returns -1 if nothing found
// returns 2 if there is more than one suitable functions to call.
int PostProsessor::Choose_Most_Suited_Function_Candidate(map<int, vector<pair<pair<Node*, Node*>, Node*>>> Candidates, Node* Caller, bool Is_Func_Ptr)
{
	//Max accuracity is currently capped out at 3
	int Max_Accuracity = 3;

	Node* Best_Candidate = nullptr;
	Node* Best_Candidate_Copy;
	Node* Scope = Caller->Scope;

	string New_Name = Caller->Construct_Template_Type_Name();

	//	  Candidate, Distance
	vector<pair<pair<pair<Node*, Node*>, Node*>, int>> Candidate_Distance;
	for (int Current_Accuracy = Max_Accuracity; Current_Accuracy >= 0; Current_Accuracy--) {
		for (auto Func : Candidates[Current_Accuracy]) {
			//Check whoose closer if the return type or parameters need to be casted
			int Current_Candidate_Distance = 0;
			//the more casting is made the further the candidate is from the caller
			vector<string> Callers_Return_Types = Caller->Get_Inheritted(true, false, true);
			vector<string> Func_Return_Types = Func.first.second->Get_Inheritted(true, false, true);

			for (int P = 0; P < Func.first.second->Parameters.size(); P++) {
				//calculate the casting distance
				Current_Candidate_Distance += Get_Casting_Distance(Caller->Parameters[P], Func.first.second->Parameters[P]);
				//calculate the ptr distance
				Current_Candidate_Distance += Caller->Parameters[P]->Calculate_Inheritted_Distance(Func.first.second->Parameters[P], "ptr");
			}

			if (Callers_Return_Types.size() == Func_Return_Types.size())
				for (int P = 0; P < Callers_Return_Types.size(); P++)
					Current_Candidate_Distance += Get_Casting_Distance(Scope->Find(Callers_Return_Types[P]), Scope->Find(Func_Return_Types[P]));
			else {
				//if the caller has return type but is not same as the func the distance grows.
				if (Callers_Return_Types.size() > 0)
					Current_Candidate_Distance += Caller->Calculate_Inheritted_Distance(Func.first.second, "ptr");
				//but if the call's return type is empty it means that the context is hard to read.
				//so the distance won be affected.
			}

			//if the function is already defined for this caller the distance should be smaller.
			if (Caller->Templates.size() > 0) {
				if (New_Name == Func.first.first->Name) {
					Current_Candidate_Distance--;
				}
			}

			Candidate_Distance.push_back({ {{Func.first.first, Func.first.second}, Func.second}, Current_Candidate_Distance });
		}
	}

	if (Is_Func_Ptr) {
		for (auto& I : Candidate_Distance) {
			I.first.first.first->Calling_Count++;
		}
		Caller->Function_Ptr = true;
		return 0;
	}

	//now determine the best candidate
	pair<pair<pair<Node*, Node*>, Node*>, int>* Closest = nullptr;
	for (auto& I : Candidate_Distance) {
		if (Closest == nullptr || Closest->second > I.second)
			Closest = &I;
	}

	int Identical_Function_Candidate_Count = 0;
	for (auto& I : Candidate_Distance) {
		if (I.second == Closest->second)
			if (Closest != &I)
				if (Closest->first.first != I.first.first)
					Identical_Function_Candidate_Count++;
	}

	if (Identical_Function_Candidate_Count > 0)
		return 2;

	if (Closest == nullptr) {
		return -1;
	}

	Best_Candidate = Closest->first.first.first;
	Best_Candidate_Copy = Closest->first.first.second;
	Scope = Closest->first.second;

	if (Best_Candidate->Is_Template_Object) {
		//check if the template function is already generated for this caller.
		Best_Candidate_Copy->Name = New_Name;

		Node* Function = Scope->Find(New_Name, Scope, { FUNCTION_NODE, PROTOTYPE, IMPORT, EXPORT });
		if (Function != nullptr && Best_Candidate_Copy->Compare_Fetchers(Function) && (Function->Template_Children[1].Components.size() > 0 && Function->Childs.size() > 0)) {
			
			/*if (Function->Template_Children[1].Components.size() > 0 && Function->Childs.size() == 0) {
				Parser parser(Function);
				parser.Input = Function->Template_Children[1].Components;

				for (auto component : parser.Input) {

				}

				parser.Factory();

				for (auto I : parser.Input)
					if (I.node)
						Function->Childs.push_back(I.node);

				Function->Is_Template_Object = false;

				PostProsessor postprosessor(Function);
				postprosessor.Open_Function_For_Prosessing(Function);
			}*/
			
			Caller->Function_Implementation = Function;
			Caller->Name = New_Name;
			Caller->Templates.clear();
			//Function->Calling_Count++;
			return 0;
		}

		//reset the name to normal
		Best_Candidate_Copy->Name = Best_Candidate->Name;
		Best_Candidate_Copy->Templates = Caller->Get_Template();

		//We need to construct our own Scope, because the fetcher could be from a namespace
		Node* Real_Scope = Scope;

		//If the best candinate has a fetcher & it's a namespace then use it.
		if (Best_Candidate_Copy->Fetcher && Best_Candidate_Copy->Fetcher->is("static"))
			Real_Scope = Scope->Find(Best_Candidate_Copy->Fetcher, Scope);

		//here we generate the template function from the template types
		Parser P(Real_Scope);
		P.Input = P.Template_Function_Constructor(Best_Candidate_Copy, Best_Candidate->Templates, Caller->Get_Template());
		P.Factory();

		//now that the function has been generated we want to find it where it has been generated.
		//If the generated function generated a Virtual function that would be at the end.
		//So we start from the end and try to find the generated function.
		for (int i = Real_Scope->Defined.size() - 1; i >= 0; i--) {
			if (Real_Scope->Defined[i]->Name == Best_Candidate_Copy->Construct_Template_Type_Name()) {
				Caller->Function_Implementation = Real_Scope->Defined[i];
				break;
			}
		}

		Caller->Name = New_Name;
		Caller->Templates.clear();

		if (Caller->Function_Implementation == nullptr) {
			Report(Observation(ERROR, "Given template arguments to a non-template call!", *Caller->Location));
		}

		if (!Caller->Function_Implementation->is(PARSED_BY::POSTPROSESSOR::POSTPROSESSOR)) {
			if (Caller->Function_Implementation->Fetcher != nullptr || Caller->Function_Implementation->Get_Scope_As(CLASS_NODE, Caller->Function_Implementation) != Global_Scope) {
				Member_Function_Defined_Inside(Caller->Function_Implementation);
				Member_Function_Defined_Outside(Caller->Function_Implementation);
			}
			Open_Function_For_Prosessing(Caller->Function_Implementation);
		}

		return 1;
	}
	else {
		Caller->Function_Implementation = Best_Candidate;
		Caller->Name = Caller->Function_Implementation->Name;
		Caller->Templates.clear();
		return 0;
	}
}

int PostProsessor::Get_Casting_Distance(Node* a, Node* b, bool Layer)
{
	int Result = 0;
	//banana -> fruit
	Node* Current = a;
	Node* Goal = b;

	if (Current->Get_Inheritted((string)"_", true, false, true) == Goal->Get_Inheritted((string)"_", true, false, true))
		return Result + Layer;

	/*
	There are atm two different types of basetype complexes

	The Legacy:
	ptr char{} <- undone!

	The Tneplate system:
	ptr char __VIRTUAL_CLASS_CHAR_PTR__{}

	*/

	if (!MANGLER::Is_Base_Type(Current))
		for (auto I : Current->Get_Inheritted(true, false, true)) {
			//This is to prevent infinite loop if the current is a same type pointter.
			if (I == Current->Name)
				continue;
			//try to chack if this inheritted is connected to the goal type.
			int Distance = Get_Casting_Distance(Current->Find(I), Goal, true);
			if (Distance > 0)
				Result += Distance;
		}

	if (Result == 0) {
		//fruit -> banana
		if (!MANGLER::Is_Base_Type(Goal))
			for (auto I : Goal->Get_Inheritted(true, false, true)) {
				//This is to prevent infinite loop if the goal is a same type pointter.
				if (I == Goal->Name)
					continue;
				//try to chack if this inheritted is connected to the goal type.
				int Distance = Get_Casting_Distance(Goal->Find(I), Current, true);
				if (Distance > 0)
					Result += Distance;
			}
	}
	return Result + Layer;
}

bool PostProsessor::Find_Castable_Inheritance(vector<string> types, string target)
{
	for (auto type : types) {
		if (Lexer::GetComponents(type)[0].is(Flags::KEYWORD_COMPONENT))
			continue;
		if (type == target)
			return true;
		if (!MANGLER::Is_Base_Type(Scope->Find(type, Scope, CLASS_NODE)))
			if (Find_Castable_Inheritance(Scope->Find(type, Scope, CLASS_NODE)->Get_Inheritted(true), target))
				return true;
	}
	return false;
}

void PostProsessor::Open_Call_Parameters_For_Prosessing(int i)
{
	if (!Input[i]->is(CALL_NODE))
		return;

	vector<Node*> Own_Defined = Input[i]->Defined;

	//give the post prosessor a way to reach the parameters that might have member fetching/ math
	PostProsessor p(Scope, Input[i]->Parameters);

	Input[i]->Parameters = p.Input;

	//see what outside defined has been injected to this call.
	for (int j = 0; j < Input[i]->Defined.size(); j++) {
		bool Is_Own_Defined = false;
		for (auto k : Own_Defined) {
			if (Input[i]->Defined[j] == k) {
				Is_Own_Defined = true;
			}
		}
		if (!Is_Own_Defined) {
			Input[i]->Scope->Defined.push_back(Input[i]->Defined[j]);
			Input[i]->Defined[j]->Scope = Input[i]->Scope;
			Input[i]->Defined.erase(Input[i]->Defined.begin() + j);
		}
	}

	//for (auto& v : Input[i]->Defined)
	//	p.Destructor_Caller(v, Input);

	Scope->Append(Output, p.Output);

	for (auto j : Input[i]->Parameters) {
		if (j->Header.size() > 0) {
			Input[i]->Header.insert(Input[i]->Header.end(), j->Header.begin(), j->Header.end());
			j->Header.clear();
		}
	}

	if (Input[i]->Context == nullptr || !Input[i]->Context->Has({ CALL_NODE, OPERATOR_NODE, ASSIGN_OPERATOR_NODE, CONDITION_OPERATOR_NODE, BIT_OPERATOR_NODE, LOGICAL_OPERATOR_NODE, ARRAY_NODE, FLOW_NODE })) {
		Node* Header_Giver = Input[i];
		
		Input.insert(Input.begin() + i, Input[i]->Header.begin(), Input[i]->Header.end());
		
		Header_Giver->Header.clear();
	}
	else if (Input[i]->Context) {
		Node* Header_Giver = Input[i];

		Input[i]->Context->Header.insert(Input[i]->Context->Header.end(), Input[i]->Header.begin(), Input[i]->Header.end());

		Header_Giver->Header.clear();
	}
	//use optimization into the parameters.
	//Algebra a(Input[i], &Input[i]->Parameters);	//Algebra has already optimized this!

	for (auto j : Input[i]->Parameters)
		if (j->is(OPERATOR_NODE)) {
			Update_Operator_Inheritance(j);
			j->Update_Size();
		}
}

void PostProsessor::Combine_Member_Fetching(Node*& n)
{
	if (n->Name != ".")
		return;

	//need to go recursive here, simply because otherwise this fetcher is just put as a tail behind the fetched.
	//This in turn will disable complex structures.

	//all of the casts and left side member computing is done in this one line.
	PostProsessor p(Scope, vector<Node**>{&n->Left}, true);

	if (n->Right->is(CALL_NODE)) {
		Cast(n->Right);
	}
	else {
		p = PostProsessor(Scope, vector<Node**>{&n->Right}, true);
	}

	if (n->Right->is(CALL_NODE)) {
		//We dont want to add a namespace into a parameter as 'this'
		if (!n->Left->is("static"))
			n->Right->Parameters.insert(n->Right->Parameters.begin(), n->Left);
		n->Right->Context = n->Context;
		n->Right->Fetcher = n->Left;

		//Pass the casting to the caller, if it doesnt have already one.
		if (!n->Right->Cast_Type)
			n->Right->Cast_Type = n->Cast_Type;

		n->Copy_Node(n, n->Right, n->Scope);
	}
	else {
		//Remember: Dot is constructed as any normal operator.
		//((((a.b).c[..]).d()).e) = 123
		//We have to go first to the most left sided operator.
		//set the left side
		Node* Left;
		//This if is for if the left side was also a fetcheredded component, thus it has valuable data.
		if (n->Left->Fetcher && n->Left->is(PARSED_BY::POSTPROSESSOR::COMBINE_MEMBER_FETCHER))
			Left = n->Left;
		else
			Left = Scope->Find(Get_From_AST(n->Left), Scope, { OBJECT_NODE, OBJECT_DEFINTION_NODE, PARAMETER_NODE, TEMPLATE_NODE, CLASS_NODE, /*FUNCTION_NODE, EXPORT, IMPORT*/ }, false);
		//we must also update the current left side to inherit the members from the inherit list

		//get the left side of the dot operator, this is getted from most left because it can be also an AST.
		Node* Right = n->Get_Most_Left(n->Right);

		if (Right->Name == "size") {
			Node* num = Right->Find("size", Left);
			if (num == nullptr || (num->is("internal"))) {
				//this means it is definetly a size get request
				Right->Name = to_string(Left->Size);
				Right->Type = NUMBER_NODE;	
				try {
					Right->Size = Parser::Get_Number_Size(stoll(Right->Name));
				}
				catch (...){
					if (atoll(Right->Name.c_str()) > INT32_MAX)
						Right->Size = 8;
					else
						Right->Size = 4;
				}

				Node* tmp = Right->Find(Right->Size, Right->Scope, CLASS_NODE, "integer", true);

				if (!tmp){
					Report(Observation(ERROR, "Cannot find suitable class to give inheritance to number: " + Right->Name, DEFINITION_ERROR));
				}

				Right->Inheritted = {tmp->Name};
			}
			else
				//find the inheritted definition
				n->Copy_Node(Right, n->Find(Right, Left, {OBJECT_DEFINTION_NODE, PARAMETER_NODE}), Scope);
		}
		else
			//find the inheritted definition
			n->Copy_Node(Right, n->Find(Right, Left, { OBJECT_DEFINTION_NODE, PARAMETER_NODE }), Scope);

		if (!Right->is(NUMBER_NODE))
			//set the parent as a fechable
			Right->Fetcher = Left;

		//now remove the current dot operator and replace it with the new fetched member
		Right->Context = n->Context;
		Right->Scope = n->Scope;

		if (n->Cast_Type)
			Right->Cast_Type = n->Cast_Type;

		//a.Array[1]
		//put the a.Array as the left side of the array operator
		if (n->Right->is(ARRAY_NODE)) {
			//first move to the right nodes
			//and then make copy_node to re-adjust the member variable touchings
			Right->Copy_Node(n->Right->Left, Right, Right->Scope);

			n->Copy_Node(n, n->Right, n->Scope);
		}
		else{
			Node* Old = n;

			// NOTE: the right side node that is depicked here as 'Right' will no longer represent the new 'Right' side, 
			// Since it is moved into 'n'
			Right->Copy_Node(n, Right, Right->Scope);

			// Replace the usage of the member fetch on outer, then inner members
			n->Replace_Usage_Of(Old, n);
		}
	}

	n->Set(PARSED_BY::POSTPROSESSOR::COMBINE_MEMBER_FETCHER);

}

Node* PostProsessor::Get_From_AST(Node* n)
{
	//((((a.b).c[..]).d()).e) = 123
	if (n->is(CONTENT_NODE)) {
		//childs can have only one start node for the AST because that is how math just works :/
		//(a.b + a.c) 
		//				n is parent because of the local scope
		PostProsessor p(n, n->Childs);

		n->Childs = p.Input;

		return Get_From_AST(n->Childs[0]);
	}
	else if (n->is(OPERATOR_NODE)) {
		vector<Node**> Change = { &n };
		PostProsessor p(Scope, Change);

		if (Change.size() > 1)
			Report(Observation(ERROR, "Un handled situation!", *n->Location));

		return Get_From_AST(n);	//this call the same funciton again because the structure of the AST might have been changed.
	}
	else if (n->is(ARRAY_NODE)) {

		vector<Node**> Change = { &n };

		PostProsessor p(Scope, Change);

		if (Change.size() > 1)
			Report(Observation(ERROR, "Un handled situation!", *n->Location));

		return n->Get_Most_Left();
	}
	else if (n->is(CALL_NODE)) {
		PostProsessor p(Scope, n->Parameters);

		n->Parameters = p.Input;

		Find_Call_Owner(n);
		return n;
	}
	else  {
		return n;
	}
}

//this is not called upon type initializations because this is called on function memebers
//so that unused classe's members size wont need to be calculated.
void PostProsessor::Define_Sizes(Node* p)
{
	//here we set the defined size of the variable
	for (Node* d : p->Defined) {
		//d->Get_Inheritted_Class_Members();
		d->Update_Size();
		d->Update_Format();
	}

	p->Update_Local_Variable_Mem_Offsets();
	p->Update_Member_Variable_Offsets(p);

}

void PostProsessor::Combine_Condition(int i)
{
	if (!Input[i]->is(IF_NODE))
		return;

	Node* current_condition = Input[i];
	int j;
	//loop through the next nodes if theyre else ifs
	for (j = i + 1; j < Input.size(); j++) {
		if (Input[j]->is(ELSE_IF_NODE) || Input[j]->is(ELSE_NODE)) {
			//give the else if the parent as the if
			Input[j]->Predecessor = current_condition;
			//give the if the child as successor else if
			current_condition->Succsessor = Input[j];
			//give the successor as the current pointter
			current_condition = Input[j];
		}
		else {
			break;
		}
	}

	//now remove the elses
	Input.erase(Input.begin() + i + 1, Input.begin() + j);
}

void PostProsessor::Determine_Return_Type(int i)
{
	if (!Input[i]->Has({ ASSIGN_OPERATOR_NODE, OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, LOGICAL_OPERATOR_NODE, PREFIX_NODE, POSTFIX_NODE }))
		return;

	if (Input[i]->is(PREFIX_NODE)) {
		PostProsessor r(Scope, vector<Node**>{ &Input[i]->Right});
	}
	else if (Input[i]->is(POSTFIX_NODE)) {
		PostProsessor r(Scope, vector<Node**>{ &Input[i]->Left});
	}
	else if (Input[i]->Right->is(CALL_NODE) && MANGLER::Is_Based_On_Base_Type(Input[i]->Right)) {
		PostProsessor l(Scope, vector<Node**>{ &Input[i]->Left });
		Input[i]->Right->Inheritted = Input[i]->Left->Inheritted;
		PostProsessor r(Scope, vector<Node**>{ &Input[i]->Right});
	}
	else if (Input[i]->Left->is(CALL_NODE) && MANGLER::Is_Based_On_Base_Type(Input[i]->Left)) {
		PostProsessor r(Scope, vector<Node**>{ &Input[i]->Right});
		Input[i]->Left->Inheritted = Input[i]->Right->Inheritted;
		PostProsessor l(Scope, vector<Node**>{ &Input[i]->Left });
	}
	else {
		PostProsessor r(Scope, vector<Node**>{ &Input[i]->Left, &Input[i]->Right });
	}

	if (!Input[i]->Has({ PREFIX_NODE, POSTFIX_NODE })) {
		//try to find a suitable operator overload if there is one
		for (auto& overload : Input[i]->Left->Operator_Overloads) {
			//the syntax still needs to be done!

			//the operator overloads return type is the same as the operator type for this.
			Input[i]->Inheritted = overload->Inheritted;
			return;
		}

		//This is responsible for setting inheritanse for numbers
		Node** Number = &Input[i]->Left;
		Node** Non_Number = &Input[i]->Right;

		if ((*Non_Number)->is(NUMBER_NODE)) {
			if (!(*Number)->is(NUMBER_NODE)) {
				Number = &Input[i]->Right;
				Non_Number = &Input[i]->Left;
			}
			else {
				//Both of em are numbers
				vector<string> Inheritance;
				vector<Node*> Contexts;
				
				if ((*Number)->Size > (*Non_Number)->Size) {
					Contexts = (*Number)->Get_Context_Path();
				}
				else {
					Contexts = (*Non_Number)->Get_Context_Path();
				}

				reverse(Contexts.begin(), Contexts.end());
				for (auto Context : Contexts) {
					if (Context->Inheritted.size() > 0) {
						(*Number)->Inheritted = Context->Inheritted;
						(*Non_Number)->Inheritted = Context->Inheritted;

						// update the sizes:
						(*Number)->Update_Size();
						(*Non_Number)->Update_Size();

						break;
					}
				}

				if (Inheritance.size() == 0) {
					Report(Observation(WARNING, "Un-clear context for '" + (*Number)->Name + "' to decide type", *(*Number)->Location, ONLY_NUMBER_OPERATION));
				}
			}
		}
		
		//Set the number inheritanse
		// if ((*Number)->is(NUMBER_NODE)){
		// 	// We need to skip all keywords likle "ptr".

		// 	for (int j = 0; j < (*Non_Number)->Inheritted.size(); j++){
		// 		if (!Lexer::GetComponent((*Non_Number)->Inheritted[j]).is(Flags::KEYWORD_COMPONENT)) {
		// 			(*Number)->Inheritted.push_back((*Non_Number)->Inheritted[j]);
		// 		}
		// 	} 
		// }

		if (Input[i]->Left->is(NUMBER_NODE)) {
			//Some operators cannot be swapped, like - and /
			if (Input[i]->Name == "+" || Input[i]->Name == "*" || Input[i]->Name == "==" || Input[i]->Name == "!=") {
				//these can be switched whitout any consequenses.
				Node* tmp = Input[i]->Left;
				Input[i]->Left = Input[i]->Right;
				Input[i]->Right = tmp;
			}
			else {
				//make a tmp variable and set the number constant into it
				string Constant_Name = Input[i]->Left->Name + "_TMP";

				Node* Constant_Tmp;

				if (Input[i]->Find(Constant_Name)) {
					Constant_Tmp = Input[i]->Find(Constant_Name);
				}
				else {
					Constant_Tmp = new Node(OBJECT_DEFINTION_NODE, Input[i]->Left->Location);
					Constant_Tmp->Name = Constant_Name;
					Constant_Tmp->Scope = Input[i]->Scope;
					Constant_Tmp->Inheritted = Input[i]->Left->Get_Inheritted();
					Constant_Tmp->Size = Input[i]->Left->Size;

					Scope->Defined.push_back(Constant_Tmp);
				}

				//move the constant here.
				Node* Move = new Node(ASSIGN_OPERATOR_NODE, new Position());
				Move->Name = "=";
				Move->Scope = Input[i]->Scope;

				Move->Left = new Node(*Constant_Tmp);
				Move->Left->Context = Move;

				Move->Right = new Node(*Input[i]->Left);
				Move->Right->Context = Move;

				Input[i]->Header.insert(Input[i]->Header.begin(), Move);
				*Input[i]->Left = *Constant_Tmp;
			}
		}
	}

	int Left_Size = 0;
	int Right_Size = 0;

	if (!Input[i]->is(PREFIX_NODE)) {
		for (auto j : Input[i]->Left->Get_Inheritted(false, false)) {
			if (Lexer::GetComponents(j)[0].is(Flags::KEYWORD_COMPONENT))
				continue;
			Left_Size += Scope->Find(j, Scope)->Size;
		}
		if (Input[i]->Left->Cast_Type != nullptr && Input[i]->Left->Cast_Type->Name != "address")
			Left_Size = Scope->Find(Input[i]->Left->Cast_Type, Scope)->Size;
		if (Input[i]->Left->is("ptr"))
			Left_Size = _SYSTEM_BIT_SIZE_;
	}

	if (!Input[i]->is(POSTFIX_NODE)) {
		for (auto j : Input[i]->Right->Get_Inheritted(false, false)) {
			if (Lexer::GetComponents(j)[0].is(Flags::KEYWORD_COMPONENT))
				continue;
			Right_Size += Scope->Find(j, Scope)->Size;
		}
		if (Input[i]->Right->Cast_Type != nullptr && Input[i]->Right->Cast_Type->Name != "address")
			Right_Size = Scope->Find(Input[i]->Right->Cast_Type, Scope)->Size;
		if (Input[i]->Right->is("ptr"))
			Right_Size = _SYSTEM_BIT_SIZE_;
	}

	if (Left_Size >= Right_Size)
		Input[i]->Inheritted = Input[i]->Left->Get_Inheritted(false, false);
	else
		Input[i]->Inheritted = Input[i]->Right->Get_Inheritted(false, false);

	if (!Input[i]->is(PREFIX_NODE)) {
		if (Input[i]->Left->Function_Address_Giver)
			Input[i]->Left->Size = Left_Size;
		if (Input[i]->Left->Header.size() > 0) {
			Node* Header_Giver = Input[i]->Left;

			if (Input[i]->Context == nullptr)
				Input.insert(Input.begin() + i, Input[i]->Left->Header.begin(), Input[i]->Left->Header.end());
			else
				Input[i]->Context->Header.insert(Input[i]->Context->Header.end(), Input[i]->Left->Header.begin(), Input[i]->Left->Header.end());

			Header_Giver->Header.clear();
		}
	}
	if (!Input[i]->is(POSTFIX_NODE)) {
		if (Input[i]->Right->Header.size() > 0) {
			Node* Header_Giver = Input[i]->Right;

			if (Input[i]->Context == nullptr)
				Input.insert(Input.begin() + i, Input[i]->Right->Header.begin(), Input[i]->Right->Header.end());
			else
				Input[i]->Context->Header.insert(Input[i]->Context->Header.end(), Input[i]->Right->Header.begin(), Input[i]->Right->Header.end());

			Header_Giver->Header.clear();
		}
		if (Input[i]->Right->Function_Address_Giver)
			Input[i]->Right->Size = Right_Size;
	}

	if ((Input[i]->Header.size() > 0) && (Input[i]->Context == nullptr)) {
		Node* Header_Giver = Input[i];

		Input.insert(Input.begin() + i, Input[i]->Header.begin(), Input[i]->Header.end());

		Header_Giver->Header.clear();
	}
}

void PostProsessor::Determine_Array_Type(int i)
{
	if (!Input[i]->is(ARRAY_NODE))
		return;

	PostProsessor r(Scope, vector<Node**>{ &Input[i]->Right, &Input[i]->Left });

	//Who is gay and does not pay taxes also farts in public 
	for (auto& overload : Input[i]->Left->Operator_Overloads) {
		//the syntax still needs to be done!

		//the operator overloads return type is the same as the operator type for this.
		Input[i]->Inheritted = overload->Inheritted;
		return;
	}

	//give the type inheritance to this array operator so that the upper function can check the do-ability
	Input[i]->Inheritted = Input[i]->Left->Inheritted;
}

void PostProsessor::Open_PreFix_Operator(int i)
{
	if (!Input[i]->is(PREFIX_NODE))
		return;

	PostProsessor p(Scope, vector<Node**>{ &Input[i]->Right });
}

void PostProsessor::Open_PostFix_Operator(int i)
{
	if (!Input[i]->is(POSTFIX_NODE))
		return;


	PostProsessor p(Scope, vector<Node**>{ &Input[i]->Left });
}

void PostProsessor::Type_Size_Definer(Node* t)
{
	if (t->Type != CLASS_NODE)
		return;
	if (t->Templates.size() > 0)	//template types are constructed elsewhere.
		return;
	//update members sizes
	t->Update_Size();

	//update the member stack offsets
	t->Update_Local_Variable_Mem_Offsets();

	//update format
	t->Update_Format();

	//update all member formats as well
	for (auto& i : t->Defined)
		i->Update_Format();
}

void PostProsessor::Handle_Imports(Node* i)
{
	if (!i->is(IMPORT))
		return;
	//import func new (4, ABC)
	//all numbers need to be redefined by type size.
	//and all other text is already classes.
	//pointters are inside the parameter as inheritance.
	for (int j = 0; j < i->Parameters.size(); j++) {
		/*vector<string> Inheritted = Numerical_Types[j]->Inheritted;
		if (Numerical_Types[j]->is(NUMBER_NODE)) {
			*Numerical_Types[j] = *Global_Scope->Find(atoi(Numerical_Types[j]->Name.c_str()), Global_Scope, CLASS_NODE, Numerical_Types[j]->Format);
			Numerical_Types[j]->Inheritted.insert(Numerical_Types[j]->Inheritted.end(), Inheritted.begin(), Inheritted.end());
		}
		else if (!MANGLER::Is_Base_Type(Numerical_Types[j]))
			continue;
		else if (Numerical_Types[j]->is(OBJECT_DEFINTION_NODE)) {
			if ((Numerical_Types[j]->Name == "type") || Numerical_Types[j]->is("type") != -1)
				continue;
			*Numerical_Types[j] = *Global_Scope->Find(Numerical_Types[j]->Name, Global_Scope, CLASS_NODE);
			Numerical_Types[j]->Inheritted.insert(Numerical_Types[j]->Inheritted.end(), Inheritted.begin(), Inheritted.end());
		}*/
		i->Parameters[j] = Construct_Node_From_Numerical_Info(i->Parameters[j], PARAMETER_NODE);
	}
	if (i->Numerical_Return_Types.size() > 0) {
		/*Parse_Returning_Numerical_Types = true;
		Numerical_Types = i->Numerical_Return_Types;
		goto Again;*/
		for (int j = 0; j < i->Numerical_Return_Types.size(); j++) {
			Node* Inheritted = Construct_Node_From_Numerical_Info(i->Numerical_Return_Types[j], OBJECT_DEFINTION_NODE);
			for (auto inherit : Inheritted->Inheritted)
				i->Inheritted.push_back(inherit);

			i->Numerical_Return_Types.clear();
		}
	}
}

long long Construct_Node_From_Numerical_Info_Name_ID = 0;
Node* PostProsessor::Construct_Node_From_Numerical_Info(Node* n, Node_Type Flag)
{
	if (n->is(NUMBER_NODE)) {
		//n == ptr 1 integer
		Node* Result = new Node(Flag, n->Location);
		Result->Name = "____VIRTUAL_CLASS_" + to_string(Construct_Node_From_Numerical_Info_Name_ID++);
		Result->Inheritted = n->Inheritted;
		Result->Scope = n->Scope;
		Result->Inheritted.push_back(Scope->Find(atoi(n->Name.c_str()), Scope, CLASS_NODE, n->Format)->Name);

		return Result;
	}
	else if (!MANGLER::Is_Base_Type(n))
		return n;	//normal user defined prototype/import
	else {
		//all base types should BE numericalized
		Report(Observation(ERROR, "WTF?", *n->Location));
		return nullptr;
	}
}

void PostProsessor::Open_Loop_For_Prosessing(int i)
{
	if (!Input[i]->is(WHILE_NODE))
		return;

	//while (a + 1 < a * 2){..}
	//while (int i = 0, a + i < a * i*2, i++){..}
	//we dont necessarily need to seperate the condition operator.

	//now just prosess the child tokens of while node as well.
	PostProsessor post(Input[i]);
	post.Input = Input[i]->Childs;

	//NOTE: this defined sizes might be reduntant!
	post.Define_Sizes(Input[i]);

	Process_Function_Pointters(Input[i]);

	//haha brain go brr
	post.Factory();

	Input[i]->Childs = post.Input;

	post.Input = Input[i]->Parameters;
	post.Factory();

	Input[i]->Parameters = post.Input;

	for (auto& v : Input[i]->Defined)
		post.Destructor_Caller(v, Input[i]->Childs);

	Scope->Append(Input[i]->Childs, post.Output);
}

void PostProsessor::Update_Used_Object_Info(Node* n)
{
	if (!n->is(FUNCTION_NODE))
		return;
	for (auto i : n->Get_all(OBJECT_NODE)) {
		i->Inheritted = n->Find(i->Name, i->Scope)->Inheritted;
		i->Update_Size();
	}
	//do the same for parameters
	for (auto i : n->Get_all(PARAMETER_NODE)) {
		i->Inheritted = n->Find(i->Name, i->Scope)->Inheritted;
		i->Update_Size();
	}
}

void PostProsessor::Operator_Overload(int i)
{
	//todo: make the override syntax
}

int PostProsessor::Get_Amount(string t, Node* n)
{
	int result = 0;
	for (string s : n->Inheritted)
		if (s == t)
			result++;

	if (n->Cast_Type != nullptr && n->Cast_Type->Name != "address")
		for (auto i : n->Find(n->Cast_Type, n)->Inheritted)
			if (i == t)
				result++;

	return result;
}

void PostProsessor::Change_Local_Strings_To_Global_Pointters(int i)
{
	if (!Input[i]->is(STRING_NODE))
		return;
	if (Scope->Name == "GLOBAL_SCOPE")
		return;
	//a = "123" to a = S0
	int Current_S_Count = 0;
	for (auto c : Global_Scope->Defined) {
		if (!c->is(LABEL_NODE))
			continue;
		Current_S_Count++;
		if (c->String == Input[i]->Name) {
			*Input[i] = *c;
			if (!Input[i]->is("ptr"))
				Input[i]->Inheritted.push_back("ptr");
			return;
		}
	}
	//if there is no string Sx make a new one.
	Node* s = new Node(LABEL_NODE, Input[i]->Location);
	s->String = Input[i]->Name;
	s->Name = "S" + to_string(Current_S_Count);
	s->Inheritted = { Global_Scope->Find(1, Global_Scope, CLASS_NODE, "integer")->Name };
	if (!s->is("ptr"))
		s->Inheritted.push_back("ptr");
	s->Scope = Global_Scope;

	Node* init = new Node(ASSIGN_OPERATOR_NODE, Input[i]->Location);
	init->Name = "=";
	init->Scope = Global_Scope;

	Node* value = new Node(STRING_NODE, Input[i]->Location);
	value->Name = s->String;
	value->Scope = Global_Scope;
	value->Size = 1;	//byte

	init->Left = s;
	init->Right = value;

	Global_Scope->Header.push_back(init);
	Global_Scope->Defined.push_back(s);

	Change_Local_Strings_To_Global_Pointters(i);
}

void PostProsessor::Move_Global_Varibles_To_Header(int& i)
{
	if (!Input[i]->is(ASSIGN_OPERATOR_NODE))
		return;
	if (Scope->Name != "GLOBAL_SCOPE" && !Scope->is("static"))
		return;

	Node* Globl_Var = Scope->Find(Input[i]->Left->Name);
	Globl_Var->Type = OBJECT_NODE;

	Globl_Var->Update_Size();

	Scope->Header.push_back(Input[i]);

	Input.erase(Input.begin() + i--);
}

bool PostProsessor::Check_If_Template_Function_Is_Right_One(Node* t, Node* c)
{
	//t = template
	//c = call
	int Type_Amount = 0;
	if (t->Templates.size() > 0) {
		for (auto T : t->Templates)
			for (auto i : t->Inheritted)
				if (i == T->Name)
					Type_Amount++;
	}
	else {
		//how many times we can skip a type
		int Type_Amount = Get_Amount("type", t);

	}

	if (c->is("type")) {
		//this means this funciton call is in template usage or this is a void calling convension.
		return true;
	}

	for (auto i : c->Inheritted) {
		if (Lexer::GetComponents(i)[0].is(Flags::KEYWORD_COMPONENT)) {
			if (!t->is(i)) {
				return false;	//teplate function must contain same keywords.
			}
		}
		else if (!t->is(i)) {
			if (Type_Amount > 0) {
				Type_Amount--;
				continue;
			}
			else
				return false;
		}
	}
	return true;
}

void PostProsessor::Open_Safe(vector<Node*> n)
{
	Safe s(n);
}

void PostProsessor::Update_Operator_Inheritance(Node* n)
{
	if (n->is(CONTENT_NODE))
		Update_Operator_Inheritance(n->Childs[0]);
	if (!n->is(ASSIGN_OPERATOR_NODE) && !n->is(CONDITION_OPERATOR_NODE) && !n->is(OPERATOR_NODE) && !n->is(BIT_OPERATOR_NODE) && !n->is(ARRAY_NODE))
		return;

	Update_Operator_Inheritance(n->Left);
	Update_Operator_Inheritance(n->Right);

	//check for operator overrides.
	n->Inheritted.clear();

	if (n->is(ARRAY_NODE)) {
		int Pointter_UnWrapping_Count = 1;	//default
		if (n->Right->Childs.size() > 1)
			Pointter_UnWrapping_Count = (int)n->Right->Childs.size();

		for (auto i : n->Find(n->Left, n->Scope)->Inheritted) {
			if (i == "ptr") {
				if (Pointter_UnWrapping_Count < 1) {
					n->Inheritted.push_back(i);
					continue;
				}
				else
					Pointter_UnWrapping_Count--;
			}
			else
				n->Inheritted.push_back(i);
		}
	}
	else {
		Node* scope = n->Left;

		if (Scope->is(CLASS_NODE))
			scope = Scope;

		if (n->Get_Scope_As({ FUNCTION_NODE, IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE }, n, false))
			scope = n->Get_Scope_As({ FUNCTION_NODE, IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE }, n, false);

		if (n->Left->Has({OPERATOR_NODE, CONDITION_OPERATOR_NODE, BIT_OPERATOR_NODE, ARRAY_NODE, LOGICAL_OPERATOR_NODE, PREFIX_NODE, POSTFIX_NODE}))
			n->Inheritted = n->Left->Inheritted;
		else if (!n->Left->is(NUMBER_NODE) && !n->Left->is(CONTENT_NODE))
			n->Inheritted = n->Left->Scope->Find(n->Left, scope)->Inheritted;
		else if (n->Left->is(NUMBER_NODE)){
			// NOTICE: using number as an inheitance is wrong in operator inheritance setting. 
			n->Inheritted = n->Left->Get_Inheritted(false, false);
			Report(Observation(ERROR, "Internal problem with Algebra not able to deal with numbers, please turn off cpu and RUN!!!"));
		}
		else {
			scope = n->Right;

			if (Scope->is(CLASS_NODE))
				scope = Scope;

			if (n->Get_Scope_As({ FUNCTION_NODE, IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE }, n, false))
				scope = n->Get_Scope_As({ FUNCTION_NODE, IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE }, n, false);

			if (n->Right->Has({ OPERATOR_NODE, CONDITION_OPERATOR_NODE, BIT_OPERATOR_NODE, ARRAY_NODE, LOGICAL_OPERATOR_NODE, PREFIX_NODE, POSTFIX_NODE }))
				n->Inheritted = n->Right->Inheritted;
			else
				//both cannot be numbers, because otherwise algebra would have optimized it away.
				n->Inheritted = n->Right->Scope->Find(n->Right, scope)->Inheritted;
		}
	}
}

void PostProsessor::Update_Inheritance(Node* n)
{
	if (n->Context->Name == "return") {
		n->Inheritted = n->Get_Scope_As(FUNCTION_NODE, n)->Inheritted;
	}
	else {
		Update_Operator_Inheritance(n->Context);
		n->Inheritted = n->Context->Inheritted;
	}
}

void PostProsessor::Analyze_Return_Value(Node* n)
{
	if (n->Name != "return")
		return;
	if (n->Right == nullptr)
		return;

	PostProsessor p(n, vector<Node**>{ &n->Right });

	if (n->Has({ OBJECT_NODE, OBJECT_DEFINTION_NODE, PARAMETER_NODE }))
		for (auto& v : n->Find(n->Right, n)->Defined)
			p.Destructor_Caller(v, Input);
		
	Scope->Append(Output, p.Output);

	Update_Operator_Inheritance(n->Right);
}

void PostProsessor::Handle_Numbers(Node* n){
	if (!n->is(NUMBER_NODE))
		return;

	if (n->is(PARSED_BY::POSTPROSESSOR::NUMBER_HANDLER))
		return;

	// Dont need to use automated return type detection if the number already is casted.
	if (n->Cast_Type)		// this will not work anymore since ALL numbers have inheritances nowdays => [n->Inheritted.size() != 0 || ]
		return;

	// This is for the numbers that are used in operator context.
	if (n->Context && !n->Context->is(CALL_NODE) && n->Context->Name != "return"){
		n->Set(PARSED_BY::POSTPROSESSOR::NUMBER_HANDLER);
		Node* Number = n;

		// It really doesnt matter if the other side is also a number, just set inheritance to both of em.
		Node* Non_Number = n->Get_Pair();

		if (Non_Number->is(CALL_NODE) && Non_Number->Function_Implementation == nullptr){

			// Because some functions are reliable of the numbering pair of the context, we will first set the call nodes inheritted to the number type.
			Non_Number->Inheritted = Number->Get_Inheritted();

			// Parse caller parameters.
			PostProsessor p(Non_Number, Non_Number->Parameters);

			// now we will need to call the call owner finder
			Find_Call_Owner(Non_Number);

			// now that the call has found the function implemitation its going to call, we can get its inheritance.
			Number->Inheritted = Non_Number->Get_Inheritted(true);

		}	
		else if (Non_Number->Get_Inheritted().size() == 0){

			// if the pair is not a uninitialized call and it doesnt have any inheritted, then it is probably a number too.
			Number->Inheritted = Number->Get_Inheritted();

		}
		else{
			// this means the pair has inheritted, we can copy from.
			Number->Inheritted = Non_Number->Get_Inheritted();	// (true)

			// Check if the new inheritance given to the number doesnt exceed _SYSTEM_BIT_SIZE_
			unsigned int Non_PTR_Inheritted_Size = Number->Update_Size();

			if (Non_PTR_Inheritted_Size > _SYSTEM_BIT_SIZE_){
				// If the Non_Number has 'ptr' as an inheritance, thenwe can put the number to also be a 'ptr', since it wont fit into a non_ptr format.
				if (Non_Number->is("ptr")){
					Report(Observation(ERROR, "Cannot un-wrap '" + Non_Number->Name + "' since it exceeds the architecture size of " + to_string(_SYSTEM_BIT_SIZE_ * 8) + " bits.", *Non_Number->Location, CANNOT_UNWRAP_PTR));
				}
				else{
					// If it doesnt even have 'ptr' and is larger than _SYSTEM_BIT_SIZE_, then we will report it as an error.
					Report(Observation(ERROR, "Incorrect use of number with object '" + Non_Number->Name + "' size of " + to_string(Non_Number->Size * 8) + " bits.", *Non_Number->Location, INVALID_OPERATION));
				}
			}
		}
	}
	else if (n->Context->is(CALL_NODE) && n->Context->Function_Implementation){
		Node* func = n->Context->Function_Implementation;

		n->Set(PARSED_BY::POSTPROSESSOR::NUMBER_HANDLER);

		int Callation_Parameter_Index = 0;

		for (auto p : n->Context->Parameters){
			if (p == n)
				break;
			Callation_Parameter_Index++;
		}

		// Now that we know the index that the number takes place in, we can check ehat parameter type does the funciton implemitation require.
		Node* Requirem = func->Parameters[Callation_Parameter_Index];

		n->Inheritted = Requirem->Get_Inheritted();
	}
	else if (n->Context->Name == "return"){
		n->Set(PARSED_BY::POSTPROSESSOR::NUMBER_HANDLER);

		Node* Parent_Function = n->Get_Scope_As(FUNCTION_NODE, n);

		n->Inheritted = Parent_Function->Get_Inheritted();
	}
}








