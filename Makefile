git = git -c user.name="Auto" -c user.email="auto@auto.com" 

BUILDDIR = $(PWD)/build
SRCDIR = $(PWD)/src
LLVM_VERSION=4.0.0

all : release

.PHONY : release debug run clean patch test

release : $(BUILDDIR)/buildr/Makefile
	+make -C $(BUILDDIR)/buildr
	cp -f $(BUILDDIR)/buildr/tara tara

debug : $(BUILDDIR)/buildd/Makefile
	+make -C $(BUILDDIR)/buildd
	cp -f $(BUILDDIR)/buildd/tara tara

$(BUILDDIR)/buildr/Makefile: $(BUILDDIR)/z3/buildr/libz3.so
	mkdir -p $(BUILDDIR)/buildr
	cd $(BUILDDIR)/buildr; cmake -DCMAKE_BUILD_TYPE=Release $(SRCDIR)

$(BUILDDIR)/buildd/Makefile: $(BUILDDIR)/z3/buildd/libz3.so $(BUILDDIR)/llvm-$(LLVM_VERSION)/lib/libLLVMCore.a
	mkdir -p $(BUILDDIR)/buildd
	cd $(BUILDDIR)/buildd; cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_VERSION=$(LLVM_VERSION) -DZ3_DEBUG:BOOL=TRUE $(SRCDIR)

clean :
	rm -rf $(BUILDDIR)/buildr
	rm -rf $(BUILDDIR)/buildd
	rm -f tara
	find -name "*~"| xargs rm -rf


#-----------------------------------------------------------------------------
# Z3 fetch and patch generation

patch :
	mkdir -p $(SRCDIR)/z3-patch
	cd $(BUILDDIR)/z3; $(git) diff > $(SRCDIR)/z3-patch/z3.patch

$(BUILDDIR)/z3/README.md : 
	mkdir -p $(BUILDDIR)
	# todo: switch to https; someone may not have ssh access configured
	cd $(BUILDDIR);$(git) clone git@github.com:Z3Prover/z3.git
	cd $(BUILDDIR)/z3;$(git) checkout b8716b333908273ad8e27e325a8bea9be0596be3


NEW_Z3_FILES =  $(SRCDIR)/z3-patch/smt_model_reporter.cpp \
		$(SRCDIR)/z3-patch/special_relations_decl_plugin.cpp \
		$(SRCDIR)/z3-patch/special_relations_decl_plugin.h \
		$(SRCDIR)/z3-patch/theory_special_relations.cpp \
		$(SRCDIR)/z3-patch/theory_special_relations.h \
		$(SRCDIR)/z3-patch/api_special_relations.cpp


$(BUILDDIR)/z3/patched : $(SRCDIR)/z3-patch/z3.patch $(BUILDDIR)/z3/README.md
	cd $(BUILDDIR)/z3; $(git) stash clear && $(git) stash save && $(git) apply --whitespace=fix $(SRCDIR)/z3-patch/z3.patch
	touch $(BUILDDIR)/z3/patched

$(BUILDDIR)/z3/newfiles : $(NEW_Z3_FILES) $(BUILDDIR)/z3/README.md
	cd $(BUILDDIR)/z3/src/smt; ln -sf $(SRCDIR)/z3-patch/smt_model_reporter.cpp smt_model_reporter.cpp; ln -sf $(SRCDIR)/z3-patch/theory_special_relations.cpp theory_special_relations.cpp; ln -sf $(SRCDIR)/z3-patch/theory_special_relations.h theory_special_relations.h
	cd $(BUILDDIR)/z3/src/ast; ln -sf $(SRCDIR)/z3-patch/special_relations_decl_plugin.cpp special_relations_decl_plugin.cpp; ln -sf $(SRCDIR)/z3-patch/special_relations_decl_plugin.h special_relations_decl_plugin.h
	cd $(BUILDDIR)/z3/src/api; ln -sf $(SRCDIR)/z3-patch/api_special_relations.cpp api_special_relations.cpp
	touch $(BUILDDIR)/z3/newfiles

$(BUILDDIR)/z3/buildr/Makefile: $(BUILDDIR)/z3/patched
	rm -rf $(BUILDDIR)/z3/buildr
	cd $(BUILDDIR)/z3; python scripts/mk_make.py --staticlib -b buildr

$(BUILDDIR)/z3/buildr/libz3.so : $(BUILDDIR)/z3/buildr/Makefile $(BUILDDIR)/z3/newfiles
	+make -C $(BUILDDIR)/z3/buildr

$(BUILDDIR)/z3/buildr/libz3.a : $(BUILDDIR)/z3/buildr/libz3.so

$(BUILDDIR)/z3/buildd/Makefile: $(BUILDDIR)/z3/patched
	rm -rf $(BUILDDIR)/z3/buildd
	cd $(BUILDDIR)/z3; python scripts/mk_make.py --staticlib -d -b buildd

$(BUILDDIR)/z3/buildd/libz3.so : $(BUILDDIR)/z3/buildd/Makefile $(BUILDDIR)/z3/newfiles
	+make -C $(BUILDDIR)/z3/buildd


#---------------------------------------------------------------------------
# fetch and install local llvm with debugging enabled

$(BUILDDIR)/llvm-$(LLVM_VERSION).src/README.txt:
	cd $(BUILDDIR);wget http://releases.llvm.org/$(LLVM_VERSION)/llvm-$(LLVM_VERSION).src.tar.xz
	cd $(BUILDDIR);tar -xvJf llvm-$(LLVM_VERSION).src.tar.xz; mkdir -p llvm-$(LLVM_VERSION).src/build; mkdir -p llvm-$(LLVM_VERSION)

$(BUILDDIR)/llvm-$(LLVM_VERSION)/lib/libLLVMCore.a : $(BUILDDIR)/llvm-$(LLVM_VERSION).src/README.txt
	cd $(BUILDDIR)/llvm-$(LLVM_VERSION).src/build;cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=DEBUG -DLLVM_ENABLE_RTTI:BOOL=TRUE -DCMAKE_INSTALL_PREFIX=../../llvm-$(LLVM_VERSION) ../
	+make -C $(BUILDDIR)/llvm-$(LLVM_VERSION).src/build
	+make -C $(BUILDDIR)/llvm-$(LLVM_VERSION).src/build install

#---------------------------------------------------------------------------

runtest:
	make -C examples/tester/

test: release runtest
