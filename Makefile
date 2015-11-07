.PHONY:clean
.PHONY:header_check
.PHONY:bin_check
.PHONY:all

CSTD = 			-std=c99
CXXSTD = 		-std=c++14    				#   C++ standard
LIBRARIES = 	-lpthread -lxml2			# linking libraries
DEFINES = 		-D SYSLOG_ON -D POS_ON -D ERRLOG_ON -D DBGLOG_ON -D SOCKETLOG_ON -D _DEFAULT_SOURCE
HEADERDIR = 	-I /usr/include/libxml2  #-I=../headers
DBGFLAG = 		-g3
WARNLVL = 		-Wall
OPTIMIZATION =  -O0
CXXFLAGS = $(HEADERDIR) $(DBGFLAG) $(WARNLVL) $(CXXSTD)  $(DEFINES) $(OPTIMIZATION) -fPIC
CFLAGS =   $(HEADERDIR) $(DBGFLAG) $(WARNLVL) $(CSTD)    $(DEFINES) $(OPTIMIZATION) -fPIC
LINKFLAGS = $(LIBRARIES)  
DYNAMIC_LIB = libkrys.so
CXX = 	clang++
CC = 	clang
OBJS = 	$(shell ls *.c *.cpp 2>/dev/null | sed "s/\(.*\.\)\(c\|cpp\)/\1o/")
LINK = $(shell ls *.cpp &>/dev/null  && echo $(CXX) || echo $(CC))

all:header_check bin_check $(BIN)

header_check:
	@ls -rt *.h *.o 2>/dev/null | tail -1 | grep "\.h$$" &>/dev/null && touch $$(ls *.cpp *.c 2> /dev/null) \
		|| [ 0 ]
bin_check:
	@  ls -rt * 2>/dev/null | grep -v Makefile | tail -1 | grep '\.c\|\.cpp\|\.o' &>/dev/null || echo "Everthing is done ^_^." \
		|| [ 0 ]


all:$(DYNAMIC_LIB)

$(DYNAMIC_LIB):$(OBJS)
	@echo -e "\n----------COMPILING DONE, LINKING-------------------"
	@echo $^ | awk '{for (i=1;i<=NF;i++) print $$i}'
	@echo " |"
	@echo " |"
	@echo " V"
	@echo "$@"
	@$(LINK) -fPIC -shared $^ -o $@ 
	@echo "---------------DONE!!!!-----------------------------"

%.o:%.cpp 
	@printf  "COMPILING: %-15s ---> %-15s|\n" $^ $@
	@$(CXX) $< $(CXXFLAGS)  -c -o $@

%.o:%.c 
	@printf  "COMPILING: %-15s ---> %-15s|\n" $^ $@
	@$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f  $(DYNAMIC_LIB) *.o