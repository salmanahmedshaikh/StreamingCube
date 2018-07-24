EXECUTABLE := main.exe
LIBDIR:=
LIBS := event_pthreads event  pthread  boost_regex boost_thread boost_system boost_filesystem  curl profiler
#LIBS :=  event_pthreads event  pthread  boost_regex boost_thread boost_system boost_filesystem  curl profiler occi clntsh
INCLUDES:=
#SRCDIR:= /home/tiana528/work/boost/jsonstream/jsonstream
SRCDIR:= $(shell find ./src -type d)
#$(warning  $(SRCDIR))

CC:=g++

#for debug
#CFLAGS := -g -Wall -O3

#for performance test
CFLAGS :=  -g -Wall -O2 -DNDEBUG -fno-inline -fno-optimize-sibling-calls -fno-inline-functions-called-once -fno-inline-functions -fno-default-inline

#for release
#CFLAGS := -s -DNDEBUG -Wall -Ofast

CPPFLAGS := $(CFLAGS)
CPPFLAGS += $(addprefix -I,$(INCLUDES))
CPPFLAGS += -MMD



RM-F := rm -f
SRCS := $(wildcard *.cpp) $(wildcard $(addsuffix /*.cpp, $(SRCDIR)))
#$(warning  $(SRCS))

OBJS := $(patsubst %.cpp,%.o,$(SRCS))
#$(warning  $(OBJS))


DEPS := $(patsubst %.o,%.d,$(OBJS))
#$(warning  $(DEPS))
#$(warning  $(wildcard $(DEPS)))

MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.cpp,$(MISSING_DEPS)))

#$(warning  $(MISSING_DEPS))

.PHONY : all deps objs clean veryclean rebuild info

all: $(EXECUTABLE)

deps : $(DEPS)

objs : $(OBJS)

clean :
	@$(RM-F) $(addsuffix /*.o, $(SRCDIR))
	@$(RM-F) $(addsuffix /*.d, $(SRCDIR))
	@$(RM-F) $(EXECUTABLE)

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif
-include $(DEPS)
$(EXECUTABLE) : $(OBJS)
	#$(CC) -o $(EXECUTABLE) $(OBJS)  $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))
	$(CC) -o $(EXECUTABLE) $(OBJS)  $(addprefix -L,$(LIBDIR)) $(addprefix -l,$(LIBS))

info:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(DEPS)
	@echo $(MISSING_DEPS)
	@echo $(MISSING_DEPS_SOURCES)
