include ../config.inc
CBMC ?= ../..

SRC = local_ssa.cpp ssa_domain.cpp translate_union_member.cpp malloc_ssa.cpp \
      guard_map.cpp ssa_object.cpp assignments.cpp ssa_dereference.cpp \
      ssa_value_set.cpp address_canonizer.cpp simplify_ssa.cpp \
      ssa_build_goto_trace.cpp

include $(CBMC)/src/config.inc
include $(CBMC)/src/common

INCLUDES= -I $(CBMC)/src

CLEANFILES = 

all: $(OBJ)

###############################################################################

