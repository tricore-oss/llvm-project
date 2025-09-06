add_llvm_component_group(Tricore)

set(LLVM_TARGET_DEFINITIONS Tricore.td)

tablegen(LLVM TricoreGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM TricoreGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TricoreGenCallingConv.inc -gen-callingconv)
tablegen(LLVM TricoreGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TricoreGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM TricoreGenCompressInstEmitter.inc -gen-compress-inst-emitter)
#tablegen(LLVM TricoreGenFastISel.inc -gen-fast-isel)
#tablegen(LLVM TricoreGenGlobalISel.inc -gen-global-isel)
#tablegen(LLVM TricoreGenPostLegalizeGICombiner.inc -gen-global-isel-combiner
#              -combiners="TricorePostLegalizerCombiner")
tablegen(LLVM TricoreGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TricoreGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TricoreGenMCPseudoLowering.inc -gen-pseudo-lowering)
#tablegen(LLVM TricoreGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM TricoreGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TricoreGenSubtargetInfo.inc -gen-subtarget)
#tablegen(LLVM TricoreGenExegesis.inc -gen-exegesis)

add_public_tablegen_target(TricoreCommonTableGen)

add_llvm_target(TricoreCodeGen
  TricoreAsmPrinter.cpp
  TricoreFrameLowering.cpp
  TricoreCallingConv.cpp
  TricoreInstrInfo.cpp
  TricoreISelDAGToDAG.cpp
  TricoreISelLowering.cpp
  TricoreMachineFunctionInfo.cpp
  TricoreRegisterInfo.cpp
  TricoreSubtarget.cpp
  TricoreTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  TricoreDesc
  TricoreInfo
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  Tricore
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
