################################################################################
#	MODULE		:	buildsys.mk
#	AUTHOR 		:	Michael A. Uman
#	DATE		:	April 3, 2013
################################################################################
GCC=g++
CPP_OBJS=$(CPP_SOURCES:%.cpp=$(OBJ_DIR)/%.o)
OBJS=$(CPP_OBJS)

ifdef DEBUG
	EXE_DIR=bin/Debug
	OBJ_DIR=obj/Debug
	CFLAGS+= -D_DEBUG=1 -g3
else
	EXE_DIR=bin/Release
	OBJ_DIR=obj/Release
	CFLAGS+= -DNDEBUG -O2
endif

################################################################################
#	Generate the targets full name
################################################################################
ifeq ($(TARGET_TYPE), exe)
TARGET=$(EXE_DIR)/$(TARGET_EXE)
endif
ifeq ($(TARGET_TYPE), statlib)
TARGET=$(LIBNAME).a
endif

CFLAGS+=$(INCLUDES)
LDFLAGS+=$(EXTERN_LIBS) $(LIBS)


#	Default rule
$(OBJ_DIR)/%.o : %.cpp Makefile
	@echo "Compiling $*.cpp"
	@$(GCC) -c -o $(OBJ_DIR)/$*.o $(CFLAGS) $*.cpp

################################################################################
#	executable target
################################################################################
ifeq ($(TARGET_TYPE), exe)
$(TARGET): objdir exedir $(OBJS) $(EXTERN_LIBS)
	@echo "Linking $(TARGET)"
	@$(GCC) -o $(TARGET) $(OBJS) $(LDFLAGS)
endif

################################################################################
#	static library target
################################################################################
ifeq ($(TARGET_TYPE), statlib)
$(TARGET): objdir $(OBJS)
	@echo "Generating static library $(TARGET)"
	@$(AR) -r -s $(TARGET) $(OBJS)
endif

clean:
	@echo "Removing all objects and targets..."
	@rm -rf $(OBJS) $(TARGET)

objdir: $(OBJ_DIR)

exedir: $(EXE_DIR)

$(OBJ_DIR):
	@if [ ! -d $(OBJ_DIR) ]; then echo "Creating object directory '$(OBJ_DIR)'..." ; mkdir -p $(OBJ_DIR); fi
	
$(EXE_DIR):
	@if [ ! -d $(EXE_DIR) ]; then echo "Creating target directory '$(EXE_DIR)'..." ; mkdir -p $(EXE_DIR); fi

depends:
	makedepend -Y  $(INCLUDES) -p'$$(OBJ_DIR)/' *.cpp

.PHONY:
	@true

