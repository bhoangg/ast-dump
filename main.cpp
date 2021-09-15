#include<iostream>
#include<vector>
#include<string>

#include "ast-dump.h"

using namespace std;

int main(int argc, char* argv[]){

	Function f;
	LIST_Function l;
	f.getFunction(l, argv[1]);
	cout << "Number of functions: " << l.size() << '\n';
	for(int i=0; i<l.size(); i++){
		cout << l[i]->returnType << " "; 
		if(!l[i]->className.empty())
			cout << l[i]->className << "::";
		cout << l[i]->funtionName;
		cout << "(";
		for(int j=0; j<l[i]->numParams; j++){
			cout << l[i]->params[j].first << " " << l[i]->params[j].second;
			if(j<l[i]->numParams-1)
				cout << ", ";
		}
		cout << ")\n";
	}

	return 0;
}

