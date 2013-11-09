SRC=main.cpp token.cpp lexer.cpp parser.cpp irCodegenContext.cpp identifier.cpp symbolTable.cpp

all:
	g++ $(SRC) `llvm-config --ldflags --cxxflags --libs` -ggdb -O0 -frtti -UNDEBUG -DDEBUG
