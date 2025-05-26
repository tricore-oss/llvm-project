add_llvm_component_library(LLVMTricoreAsmParser
  TricoreAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  TricoreDesc
  TricoreInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  Tricore
  )
