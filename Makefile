.PHONY:clean
.PHONY:header_check
.PHONY:bin_check
.PHONY:all

CSTD = 			-std=c99
CXXSTD = 		-std=gnu++14    				#   C++ standard
LIBRARIES = 	-lpthread -lxml2 -lboost_date_time -lboost_system -lboost_filesystem -lboost_thread -lboost_exception -lstdc++ -lboost_thread			# linking libraries
DEFINES = 		
HEADERDIR = 	-I /usr/include/libxml2  -Ithird_party_include#-I=../headers
DBGFLAG = 		-g3
WARNLVL = 		-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
				-Werror=parentheses \
				-Werror=uninitialized \
				-Werror=write-strings \
				-Werror=pointer-arith \
				-Werror=format \
				-Werror=format-y2k \
				-Werror=address \
				-Werror=implicit-fallthrough \
				-Werror=array-bounds \
				-Werror=float-equal \
				-Werror=shadow \
				-Werror=cast-qual \
				-Werror=unreachable-code \
				-Werror=return-stack-address


OPTIMIZATION =  -O0
CXXFLAGS = $(HEADERDIR) $(DBGFLAG) $(WARNLVL) $(CXXSTD)  $(DEFINES) $(OPTIMIZATION) -fPIC
CFLAGS =   $(HEADERDIR) $(DBGFLAG) $(WARNLVL) $(CSTD)    $(DEFINES) $(OPTIMIZATION) -fPIC
LINKFLAGS = $(LIBRARIES)  
DYNAMIC_LIB = libkrys.so
CXX = 	clang++
CC = 	clang
OBJS = 	$(shell ls *.c *.cpp 2>/dev/null | sed "s/\(.*\.\)\(c\|cpp\)/\1o/")
LINK = $(CXX)

all:header_check $(BIN)

header_check:
	ls -rt *.h *.o 2>/dev/null | tail -1 | grep "\.h$$" &>/dev/null && touch $$(ls *.cpp *.c 2> /dev/null) \
		|| echo "header clean"
		


all:$(DYNAMIC_LIB)

$(DYNAMIC_LIB):$(OBJS)
	$(LINK) -fPIC -shared $^ -o $@ 
%:%.o
	$(LINK) $(LINKFLAGS) $^ -o $@

%.o:%.cpp 
	$(CXX) $< $(CXXFLAGS)  -c -o $@

%.o:%.c 
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	$(RM)  $(DYNAMIC_LIB) *.o
