add_llvm_component_library(LLVMTricoreDisassembler
  TricoreDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  TricoreInfo
  Support

  ADD_TO_COMPONENT
  Tricore
  )
