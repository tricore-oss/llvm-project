add_llvm_component_library(LLVMTricoreDesc
  TricoreAsmBackend.cpp
  TricoreELFObjectWriter.cpp
  TricoreELFStreamer.cpp
  TricoreInstPrinter.cpp
  TricoreMCAsmInfo.cpp
  TricoreMCCodeEmitter.cpp
  TricoreMCExpr.cpp
  TricoreMCTargetDesc.cpp
  TricoreTargetStreamer.cpp

  LINK_COMPONENTS
  CodeGenTypes
  MC
  TricoreInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  Tricore
  )
