#ifndef AST_DUMP_H
#define AST_DUMP_H

#include <iostream>
#include <vector>
#include <string>

struct FunctionProto{
	std::string returnType;
	std::string className;
	std::string funtionName;
	int numParams;
	std::vector<std::pair<std::string,std::string>> params;
};

typedef std::vector<FunctionProto*> LIST_Function;

class Function{
public:
	Function(){}
	int getFunction(LIST_Function &f, const char * filename);
private:
};

#endif /*AST_DUMP_H*/

