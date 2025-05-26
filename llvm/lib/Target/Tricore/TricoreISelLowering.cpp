//===-- TricoreISelLowering.cpp - Tricore DAG Lowering Implementation
//---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that Tricore uses to lower LLVM code into
// a selection DAG.
//
//===----------------------------------------------------------------------===//

#include "TricoreISelLowering.h"
#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TricoreCallingConv.h"
#include "TricoreMachineFunctionInfo.h"
#include "TricoreRegisterInfo.h"
#include "TricoreSubtarget.h"
#include "TricoreTargetMachine.h"
#include "TricoreTargetObjectFile.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include <cassert>
using namespace llvm;

#include "TricoreGenCallingConv.inc"

TricoreTargetLowering::TricoreTargetLowering(const TargetMachine &TM,
                                             const TricoreSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  addRegisterClass(MVT::i32, &Tricore::DGPRRegClass);
  // addRegisterClass(MVT::i32, &Tricore::AddrRegsRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());
}

bool TricoreTargetLowering::useSoftFloat() const {
  return Subtarget->useSoftFloat();
}

SDValue TricoreTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  const Function &Func = MF.getFunction();
  if (Func.hasFnAttribute("interrupt")) {
    if (!Func.arg_empty())
      report_fatal_error(
          "Functions with the interrupt attribute cannot have arguments!");
  }

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_TricoreEABI);

  for (unsigned i = 0, e = ArgLocs.size(), InsIdx = 0; i != e; ++i, ++InsIdx) {
    CCValAssign &VA = ArgLocs[i];
    SDValue ArgValue;

    if (VA.isRegLoc()) {
      EVT LocVT = VA.getLocVT();
      const TargetRegisterClass *RC = getRegClassFor(LocVT.getSimpleVT());
      Register VReg = RegInfo.createVirtualRegister(RC);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, LocVT);
    } else if (VA.isMemLoc()) {
    }
    InVals.push_back(ArgValue);
  }

  return Chain;
}

SDValue
TricoreTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                 SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();
  bool IsPIC = isPositionIndependent();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  return Chain;
}

SDValue
TricoreTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                   bool IsVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   const SmallVectorImpl<SDValue> &OutVals,
                                   const SDLoc &DL, SelectionDAG &DAG) const {

  MachineFunction &MF = DAG.getMachineFunction();
  const TricoreSubtarget &STI = MF.getSubtarget<TricoreSubtarget>();

  // CCValAssign - represent the assignment of the return value to locations.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_TricoreEABI);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(), OutIdx = 0; i < e; ++i, ++OutIdx) {
    SDValue Val = OutVals[OutIdx];
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    if (VA.needsCustom()) {
      assert(false);
    } else {
      Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Glue);

      if (STI.isRegisterReservedByUser(VA.getLocReg()))
        MF.getFunction().getContext().diagnose(DiagnosticInfoUnsupported{
            MF.getFunction(),
            "Return value register required, but has been reserved."});

      // Guarantee that all emitted copies are stuck together.
      Glue = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    }
  }

  RetOps[0] = Chain;

  // Add the glue if we have it.
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(TricoreISD::RET_GLUE, DL, MVT::Other, RetOps);
}