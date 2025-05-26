//===-- TricoreISelLowering.h - Tricore DAG Lowering Interface ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Tricore uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCISELLOWERING_H
#define LLVM_LIB_TARGET_SPARC_SPARCISELLOWERING_H

#include "Tricore.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class TricoreSubtarget;

namespace TricoreISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  CALL,     // A call instruction.
  RET_GLUE, // Return with a glue operand.
};
}

class TricoreTargetLowering : public TargetLowering {
  const TricoreSubtarget *Subtarget;

public:
  TricoreTargetLowering(const TargetMachine &TM, const TricoreSubtarget &STI);
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerCall(CallLoweringInfo & /*CLI*/,
                    SmallVectorImpl<SDValue> & /*InVals*/) const override;
  SDValue LowerReturn(SDValue /*Chain*/, CallingConv::ID /*CallConv*/,
                      bool /*isVarArg*/,
                      const SmallVectorImpl<ISD::OutputArg> & /*Outs*/,
                      const SmallVectorImpl<SDValue> & /*OutVals*/,
                      const SDLoc & /*dl*/,
                      SelectionDAG & /*DAG*/) const override;

  bool useSoftFloat() const override;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_SPARC_SPARCISELLOWERING_H
