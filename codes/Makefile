LDFLAGS=-static -s 
.PHONY:	clean
search.exe:	main.cc anqi.cc ClientSocket.cpp Protocol.cpp HashTable.cpp Search.cpp
	$(CXX) -o $@ $^ $(LDFLAGS)
clean:
	DEL search.exe 2>NUL

# Windows cmd : g++ main.cc anqi.cc ClientSocket.cpp Protocol.cpp HashTable.cpp Search.cpp -static -s  -lws2_32 -o search.exe
