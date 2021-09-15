#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include<cstdlib>
#include<iostream>
#include<string>
#include<vector>
#include<utility>

#include "ast-dump.h"

using LineColumn = std::pair<unsigned, unsigned>;

struct Data {
	unsigned childOffset;
	std::string oldPrefix;
};

std::string toString(CXString cxString){
	std::string s = clang_getCString(cxString);
	clang_disposeString(cxString);
	return s;
}

LineColumn toLineColumn(CXSourceLocation location){
	CXFile file;
	unsigned line, column, offset;
	clang_getSpellingLocation(location, &file, &line, &column, &offset);
	return {line,column};
}

void printRelativeLocation(LineColumn previous, LineColumn location){
	if(location.first == previous.first){
		std::cout << "col:" << location.second;
	} else {
		std::cout << "line:" << location.first << ":" << location.second;
	}
}

CXChildVisitResult countFunction(CXCursor cursor, CXCursor parent, CXClientData data){
	auto * count = reinterpret_cast<unsigned*>(data);
	const CXCursorKind cursorKind = clang_getCursorKind(cursor);

	if(cursorKind==CXCursor_FunctionDecl || cursorKind==CXCursor_CXXMethod)
		*count +=1;
	return CXChildVisit_Continue;
}

CXChildVisitResult visitFunction(CXCursor cursor, CXCursor parent, CXClientData data){
	auto * func = reinterpret_cast<FunctionProto*>(data);
	const CXCursorKind cursorKind = clang_getCursorKind(cursor);
	const CXCursorKind parentKind = clang_getCursorKind(parent);
	const CXType type = clang_getCursorType(cursor);

	if(cursorKind == CXCursor_TypeRef && parentKind==CXCursor_CXXMethod){
		func->className = toString(clang_getTypeSpelling(type));
	}
	else if(cursorKind == CXCursor_ParmDecl)
	{
		std::string var = toString(clang_getCursorSpelling(cursor));
		std::string typ =  toString(clang_getTypeSpelling(type));
		func->params.push_back({typ,var});
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData clientData){
	CXSourceLocation location = clang_getCursorLocation(cursor);
	if(clang_Location_isInSystemHeader(location)) {
		return CXChildVisit_Continue;
	}

	auto * func = reinterpret_cast<LIST_Function*>(clientData);
	const CXCursorKind kind = clang_getCursorKind(cursor);
	if(!(kind == CXCursor_FunctionDecl || kind == CXCursor_CXXMethod))
		return CXChildVisit_Continue;
	
	// Location
	// const CXSourceRange range = clang_getCursorExtent(cursor);
	// auto parentLocation = toLineColumn(clang_getCursorLocation(parent));
	// auto start = toLineColumn(clang_getRangeStart(range));
	// auto end = toLineColumn(clang_getRangeEnd(range));

	// std::cout << "<";
	// printRelativeLocation(parentLocation, start);
	// if(start != end){
	// 	std::cout << ", ";
	// 	printRelativeLocation(start, end);
	// }
	// std::cout << "> ";

	//visit child of function
	FunctionProto* data = new FunctionProto;
	clang_visitChildren(cursor, visitFunction, data);

	// Result type
	const CXType type = clang_getCursorType(cursor);
	//std::cout << toString(clang_getTypeSpelling(clang_getResultType(type))) << " ";
	data->returnType = toString(clang_getTypeSpelling(clang_getResultType(type)));

	//define
	const CXCursor definition = clang_getCursorDefinition(cursor);
	if(!clang_Cursor_isNull(definition) &&
	   !clang_equalCursors(cursor, definition)) {
		std::cout << clang_hashCursor(definition) << " ";
	}
	
	// std::cout << toString(clang_getCursorSpelling(cursor)) << " ";
	data->funtionName = toString(clang_getCursorSpelling(cursor));
	data->numParams = data->params.size();
	
	func->push_back(data);
	
	return CXChildVisit_Continue;	
}

void traverse(CXTranslationUnit tu, LIST_Function &f){
	CXCursor root = clang_getTranslationUnitCursor(tu);

	CXCursorKind kind = clang_getCursorKind(root);
//	std::cout << toString(clang_getCursorKindSpelling(kind)) << '\n';

	unsigned int numberOfFunction = 0;
	clang_visitChildren(root, countFunction, &numberOfFunction);
	// std::cout << "NUmber of functions: " << numberOfFunction << '\n';

	clang_visitChildren(root, visit, &f);
}

int Function::getFunction(LIST_Function &f, const char * filename){
	CXIndex index = clang_createIndex(/*excludeDeclareationsFromPCH=*/true,
					  /*displayDiagnostics=*/true);

	CXTranslationUnit tu = clang_parseTranslationUnit(index,
							 /*source_filename=*/filename,
							 /*command_line_args=*/nullptr,
							 /*num_command_line_args=*/0,
							 /*unsaved_files=*/nullptr,
							 /*num_unsaved_files=*/0,
							 /*options=*/0);

	if(tu==nullptr){
		std::cerr << "Error\n";
	} else {
		traverse(tu, f);
		clang_disposeTranslationUnit(tu);
	}

	clang_disposeIndex(index);
	return 0;
}

#if 0
int main(int argc, char* argv[]){
	CXIndex index = clang_createIndex(/*excludeDeclareationsFromPCH=*/true,
					  /*displayDiagnostics=*/true);

	CXTranslationUnit tu = clang_parseTranslationUnit(index,
							 /*source_filename=*/argv[1],
							 /*command_line_args=*/nullptr,
							 /*num_command_line_args=*/0,
							 /*unsaved_files=*/nullptr,
							 /*num_unsaved_files=*/0,
							 /*options=*/0);

	if(tu==nullptr){
		std::cerr << "Error\n";
	} else {
		traverse(tu);
		clang_disposeTranslationUnit(tu);
	}

	clang_disposeIndex(index);
	return 0;
}

int main(int argc, char * argv[]){
	if(argc < 2){
		std::cerr << "Usage:\n\t ./ast-dump /path/to/your/file\n";
	}

	CXCompilationDatabase_Error compilationDatabaseError;
	CXCompilationDatabase compilationDatabase = clang_CompilationDatabase_fromDirectory( ".", &compilationDatabaseError );
	CXCompileCommands compileCommands         = clang_CompilationDatabase_getCompileCommands( compilationDatabase, argv[1]);
	unsigned int numCompileCommands           = clang_CompileCommands_getSize( compileCommands );

//	std::cerr << "Obtained " << numCompileCommands << " compile commands\n";

	CXIndex index = clang_createIndex(/*excludeDeclareationsFromPCH=*/true,
                                          /*displayDiagnostics=*/true);

	CXTranslationUnit tu;

	CXCompileCommand compileCommand = clang_CompileCommands_getCommand( compileCommands, 0 );	
	unsigned int numArguments       = clang_CompileCommand_getNumArgs( compileCommand );
	char** arguments                = new char*[ numArguments ];

//	std::cerr << "Obtained " << numArguments << " numArguments\n";

	for( unsigned int i = 0; i < numArguments; i++ )
	{
        	CXString argument       = clang_CompileCommand_getArg( compileCommand, i );
       		std::string strArgument = clang_getCString( argument );
		std::cout << strArgument << " ";
        	arguments[i]            = new char[ strArgument.size() + 1 ];

        	std::fill( arguments[i],
                	 arguments[i] + strArgument.size() + 1,
                	0 );

        	std::copy( strArgument.begin(), strArgument.end(), arguments[i] );

	        clang_disposeString( argument );
	}
	std::cout << "\n";

	tu = clang_parseTranslationUnit( index, 0, arguments, numArguments, nullptr, 0, 0);

	for( unsigned int i = 0; i < numArguments; i++ )
		delete[] arguments[i];

	delete[] arguments;

	if(tu==nullptr){
		std::cerr << "Error\n";
	} else {
		traverse(tu);
		clang_disposeTranslationUnit(tu);
	}

	clang_disposeIndex(index);
	clang_CompileCommands_dispose( compileCommands );
	clang_CompilationDatabase_dispose( compilationDatabase );
	return 0;
}
#endif

