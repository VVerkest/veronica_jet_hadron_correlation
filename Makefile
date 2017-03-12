os = $(shell uname -s)

#INCFLAGS      = -I/opt/local/libexec/root6/include -I$(FASTJETDIR)/include -I$(STARPICOPATH)
#INCFLAGS      = -I$(ROOTSYS)/include/root -I$(FASTJETDIR)/include -I$(STARPICODIR) -I/opt/local/include
INCFLAGS      = -I$(FASTJETDIR)/include -I$(STARPICODIR) -I/opt/local/include -I$(shell root-config --incdir)
ifeq ($(os),Linux)
CXXFLAGS      = 
else
CXXFLAGS      = -O -fPIC -pipe -Wall -Wno-deprecated-writable-strings -Wno-unused-variable -Wno-unused-private-field -Wno-gnu-static-float-init -std=c++11
## for debugging:
# CXXFLAGS      = -g -O0 -fPIC -pipe -Wall -Wno-deprecated-writable-strings -Wno-unused-variable -Wno-unused-private-field -Wno-gnu-static-float-init
endif

ifeq ($(os),Linux)
LDFLAGS       = -g
LDFLAGSS      = -g --shared 
else
LDFLAGS       = -O -Xlinker -bind_at_load -flat_namespace
LDFLAGSS      = -flat_namespace -undefined suppress
LDFLAGSSS     = -bundle
endif

ifeq ($(os),Linux)
CXX          = g++ 
else
CXX          = clang
endif


ROOTLIBS      = $(shell root-config --libs)

LIBPATH       = -L$(FASTJETDIR)/lib -L$(STARPICODIR) $(shell root-config --libs)
LIBS          = -lfastjet -lfastjettools -lTStarJetPico


# for cleanup
SDIR          = src
ODIR          = src/obj
BDIR          = bin


###############################################################################
################### Remake when these headers are touched #####################
###############################################################################


###############################################################################
# standard rules
$(ODIR)/%.o : $(SDIR)/%.cxx $(INCS)
	@echo 
	@echo COMPILING
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

$(BDIR)/%  : $(ODIR)/%.o 
	@echo 
	@echo LINKING
	$(CXX) $(LDFLAGS) $(LIBPATH) $(LIBS) $^ -o $@

###############################################################################

###############################################################################
############################# Main Targets ####################################
###############################################################################
all : $(BDIR)/star_pp_correlation

$(SDIR)/dict.cxx                : $(SDIR)/ktTrackEff.hh
	cd ${SDIR}; rootcint6 -f dict.cxx -c -I. ./ktTrackEff.hh

$(ODIR)/dict.o                  : $(SDIR)/dict.cxx
$(ODIR)/ktTrackEff.o            : $(SDIR)/ktTrackEff.cxx $(SDIR)/ktTrackEff.hh
$(ODIR)/starFunctions.o		: $(SDIR)/starFunctions.cxx $(SDIR)/starFunctions.hh

$(ODIR)/star_pp_correlation.o	: $(SDIR)/star_pp_correlation.cxx

#data analysis
$(BDIR)/star_pp_correlation			: $(ODIR)/star_pp_correlation.o	$(ODIR)/starFunctions.o $(ODIR)/ktTrackEff.o $(ODIR)/dict.o


###############################################################################
##################################### MISC ####################################
###############################################################################

clean :
	@echo 
	@echo CLEANING
	rm -vf $(ODIR)/*.o
	rm -vf $(BDIR)/*
	rm -vf lib/*
