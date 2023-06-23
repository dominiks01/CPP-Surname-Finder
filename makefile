CC = g++ -Wall 
FILE = program.cpp
CFLAGS = -Wall -lcurl -ljsoncpp 

.SILENT all: test

compile: $(FILE) 
	@ $(CC) $< $(CFLAGS) -o  $(FILE:.cpp=.exe)
	
test: clean compile
	@ make compile
	@ ./$(FILE:.cpp=.exe) 
	
clean:
	@ rm -f *.exe *.txt  *.out *.in 2> /dev/null