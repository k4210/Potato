#pragma once

// llvm stuff
#pragma warning( push )  
#pragma warning( disable : 4996 )  
#pragma warning( disable : 4146 )   
#pragma warning( disable : 4324 ) 
#pragma warning( disable : 4141 ) 
#pragma warning( disable : 4244 ) 
#pragma warning( disable : 4100 )
#pragma warning( disable : 4127 ) 
#pragma warning( disable : 4458 ) 
#pragma warning( disable : 4291 ) 
#pragma warning( disable : 4310 ) 
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#pragma warning( pop ) 

#include <ctype.h>
#include <cstdio>

#include <string>
#include <memory>

#include <map>
#include <vector>
#include <vector>
#include <list>
#include <set>

#include <algorithm>

#include <fstream>