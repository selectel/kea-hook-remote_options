KEA_MSG_COMPILER ?= kea-msg-compiler
KEA_INCLUDE ?= /usr/include/kea
KEA_LIB ?= /usr/lib
CXXFLAGS = -I $(KEA_INCLUDE) -fPIC -Wno-deprecated -std=c++14
LDFLAGS = -L $(KEA_LIB) \
		  -shared \
		  -lboost_system \
		  -lkea-dhcpsrv \
		  -lkea-dhcp++ \
		  -lkea-hooks \
		  -lkea-log \
		  -lkea-util \
		  -lkea-exceptions \
		  -lcurl
OBJECTS = remopts_messages.o \
		  remopts_log.o \
		  remopts_common.o \
		  pkt4_receive.o \
		  pkt4_send.o \
		  remopts_callouts.o \
		  version.o

../kea-hook-remote-opts.so: $(OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -lcurl

%.o: %.cc
	$(CXX) -MMD -c $(CXXFLAGS) -o $@ $<

# Compile remopts_messages (for logging)
remopts_messages.h remopts_messages.cc: remopts_messages.mes
	$(KEA_MSG_COMPILER) $<

clean:
	rm -f *.o *.d
	rm -f remopts_messages.h remopts_messages.cc
	rm -f ../kea-hook-remote-opts.so
