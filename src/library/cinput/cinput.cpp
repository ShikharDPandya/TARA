/*
 * Copyright 2014, IST Austria
 *
 * This file is part of TARA.
 *
 * TARA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TARA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with TARA.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cinput.h"

using namespace tara;
using namespace tara::cinput;
using namespace tara::helpers;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// pragam'ed to aviod warnings due to llvm included files

#include "llvm/IRReader/IRReader.h"

#include "llvm/IR/InstIterator.h"

#include "llvm/LinkAllPasses.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/IR/DebugInfo.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"

// #include "llvm/IR/InstrTypes.h"
// #include "llvm/IR/BasicBlock.h"

#include "llvm/IRReader/IRReader.h"
// #include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Dwarf.h"

#include "llvm/Analysis/CFGPrinter.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/ADT/StringMap.h"

#pragma GCC diagnostic pop


void c2bc( std::string& filename, std::string& outname ) {
  // make a system call
  std::ostringstream cmd;
  cmd << "clang-3.6 -emit-llvm -O0 -g " << filename << " -o " << outname << " -c";
  if( system( cmd.str().c_str() ) != 0 ) exit(1);
}

//todo: add unique_ptr support
program* tara::cinput::parse_cpp_file( helpers::z3interf& z3_, std::string& cfile ) {
  std::unique_ptr<llvm::Module> module;
  llvm::SMDiagnostic err;
  llvm::LLVMContext& context = llvm::getGlobalContext();

  std::string bc_file = cfile+".bc";

  c2bc( cfile, bc_file);
  //todo: get rid of clang call
  // why are we parsing IR file.. why not directly .cpp??
  module = llvm::parseIRFile( bc_file, err, context);
  if( module.get() == 0 ) {
    // give err msg
  }

  program* p = new program(z3_);
  
  return p;
}