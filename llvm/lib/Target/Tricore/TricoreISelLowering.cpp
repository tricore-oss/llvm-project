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
#include "MCTargetDesc/TricoreBaseInfo.h"
#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TricoreCallingConv.h"
#include "TricoreMachineFunctionInfo.h"
#include "TricoreRegisterInfo.h"
#include "TricoreSubtarget.h"
#include "TricoreTargetMachine.h"
#include "TricoreTargetObjectFile.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetCallingConv.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGenTypes/MachineValueType.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <ratio>
using namespace llvm;

#include "TricoreGenCallingConv.inc"

TricoreTargetLowering::TricoreTargetLowering(const TargetMachine &TM,
                                             const TricoreSubtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);

  addRegisterClass(MVT::i32, &Tricore::DGPRRegClass);
  addRegisterClass(MVT::v2i32, &Tricore::EGPRRegClass);
  // addRegisterClass(MVT::i64, &Tricore::EGPRRegClass);
  //  addRegisterClass(MVT::i32, &Tricore::AddrRegsRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());

  /* Use custom lowering for address information */
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i32, Custom);
  setOperationAction(ISD::JumpTable, MVT::i32, Custom);
  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
  setOperationAction(ISD::GlobalTLSAddress, MVT::i64, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i64, Custom);
  setOperationAction(ISD::JumpTable, MVT::i64, Custom);

  // DAG combine
  setTargetDAGCombine({ISD::SRA, ISD::AND, ISD::OR});

  setOperationAction(ISD::ADDE, MVT::i32, Legal);
  setOperationAction(ISD::ADDC, MVT::i32, Legal);
  setOperationAction({ISD::ROTL, ISD::ROTR}, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ, MVT::i32, Expand);
  setOperationAction({ISD::SHL_PARTS, ISD::SRA_PARTS, ISD::SRL_PARTS}, MVT::i32,
                     Expand); // TODO: Optimize

  // BRCC
  // setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  // Tricore does not have i1 sign extending load.
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }

  if (!Subtarget.hasSoftFloat()) {
    addRegisterClass(MVT::f32, &Tricore::DGPRRegClass);

    if (!Subtarget.hasSingleFloat()) {
      addRegisterClass(MVT::f64, &Tricore::EGPRRegClass);
    }
  }
}

bool TricoreTargetLowering::useSoftFloat() const {
  return Subtarget.hasSoftFloat();
}

const char *TricoreTargetLowering::getTargetNodeName(unsigned Opcode) const {
#define NODE_NAME_CASE(NODE)                                                   \
  case TricoreISD::NODE:                                                       \
    return "TricoreISD::" #NODE;
  // clang-format off
  switch ((TricoreISD::NodeType)Opcode) {
  case TricoreISD::FIRST_NUMBER:
    break;
    NODE_NAME_CASE(CALL)
    NODE_NAME_CASE(TAIL)
    default:
    break;
  }
  // clang-format on
  return nullptr;
}

SDValue TricoreTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  const TricoreRegisterInfo &TRI = *static_cast<const TricoreRegisterInfo *>(
      RegInfo.getTargetRegisterInfo());

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
      const TargetRegisterClass *RC = Ins[i].Flags.isPointer()
                                          ? TRI.getPointerRegClass(MF, 0)
                                          : getRegClassFor(LocVT.getSimpleVT());
      Register VReg = RegInfo.createVirtualRegister(RC);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, LocVT);
    } else if (VA.isMemLoc()) {
      int FI =
          MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      if (VA.getValVT() == MVT::i32 || VA.getValVT() == MVT::f32) {
        ArgValue =
            DAG.getLoad(VA.getValVT(), DL, Chain, FIPtr, MachinePointerInfo());
      } else {
        // We shouldn't see any other value types here.
        llvm_unreachable("Unexpected ValVT encountered in frame lowering.");
      }
    }
    InVals.push_back(ArgValue);
  }

  if (IsVarArg) {
    MachineFrameInfo &MFI = MF.getFrameInfo();
    TricoreMachineFunctionInfo *TFI = MF.getInfo<TricoreMachineFunctionInfo>();
    int VaArgOffset = CCInfo.getStackSize();
    int FI = MFI.CreateFixedObject(32, VaArgOffset, true);
    TFI->setVarArgsFrameIndex(FI);
  }

  return Chain;
}

SDValue
TricoreTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                 SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  MachineFunction &MF = DAG.getMachineFunction();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  // Tricore EABI requires Vargs to be passed on stack
  // Only analyze fixed arguments via CC and pass the rest on the stack
  if (IsVarArg) {
    SmallVector<ISD::OutputArg, 16> ArgOuts;
    ArgOuts.resize(CLI.NumFixedArgs);
    std::copy(Outs.begin(), Outs.begin() + CLI.NumFixedArgs, ArgOuts.begin());
    CCInfo.AnalyzeCallOperands(ArgOuts, CC_TricoreEABI);

    unsigned ValNo = CLI.NumFixedArgs;
    for (auto *Arg = Outs.begin() + CLI.NumFixedArgs; Arg != Outs.end();
         Arg++) {
      Align Alignment = Arg->Flags.isByVal() ? Arg->Flags.getNonZeroByValAlign()
                                             : Arg->Flags.getNonZeroMemAlign();
      int Size = Arg->Flags.isByVal() ? Arg->Flags.getByValSize()
                                      : Arg->VT.getStoreSize();
      int64_t Offset = CCInfo.AllocateStack(Size, Alignment);

      CCInfo.addLoc(CCValAssign::getMem(ValNo++, Arg->VT, Offset, Arg->VT,
                                        CCValAssign::Full));
    }
  } else {
    CCInfo.AnalyzeCallOperands(Outs, CC_TricoreEABI);
  }

  // isTailCall = isTailCall && IsEligibleForTailCallOptimization(
  //                                CCInfo, CLI, DAG.getMachineFunction());

  // Get the size of the outgoing arguments stack space requirement.
  unsigned NumBytes = CCInfo.getStackSize();

  // Keep stack frames 8-byte aligned.
  NumBytes = (NumBytes + 7) & ~7;

  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();

  // Create local copies for byval args.
  SmallVector<SDValue, 8> ByValArgs;
  for (unsigned I = 0; I != Outs.size(); ++I) {
    ISD::ArgFlagsTy Flags = Outs[I].Flags;
    if (!Flags.isByVal())
      continue;

    SDValue Arg = OutVals[I];
    unsigned Size = Flags.getByValSize();
    Align Alignment = Flags.getNonZeroByValAlign();

    if (Size > 0U) {
      int FI = MFI.CreateStackObject(Size, Alignment, false);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue SizeNode = DAG.getConstant(Size, DL, MVT::i32);

      Chain = DAG.getMemcpy(Chain, DL, FIPtr, Arg, SizeNode, Alignment,
                            false,        // isVolatile,
                            (Size <= 32), // AlwaysInline if size <= 32,
                            /*CI=*/nullptr, std::nullopt, MachinePointerInfo(),
                            MachinePointerInfo());
      ByValArgs.push_back(FIPtr);
    } else {
      SDValue nullVal;
      ByValArgs.push_back(nullVal);
    }
  }

  if (!IsTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, CLI.DL);

  // Copy argument values to their designated locations.
  SmallVector<std::pair<Register, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;
  SDValue StackPtr;
  for (unsigned I = 0, ValI = 0, OutIdx = 0; I != ArgLocs.size();
       ++I, ++OutIdx) {
    CCValAssign &VA = ArgLocs[I];
    SDValue ArgValue = OutVals[OutIdx];
    ISD::ArgFlagsTy Flags = Outs[OutIdx].Flags;

    // Use local copy if it is a byval arg.
    if (Flags.isByVal())
      ArgValue = ByValArgs[ValI++];

    if (VA.isRegLoc()) {
      // Queue up the argument copies and emit them at the end.
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), ArgValue));
    } else {
      assert(VA.isMemLoc() && "Argument not register or memory");

      // Work out the address of the stack slot.
      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, DL, Tricore::A10, MVT::i32);
      SDValue Address =
          DAG.getNode(ISD::ADD, DL, MVT::i32, StackPtr,
                      DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));

      // Emit the store.
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, ArgValue, Address,
                       MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
    }
  }

  // Join the stores, which are independent of one another.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;

  // Build a sequence of copy-to-reg nodes, chained and glued together.
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, Glue);
    Glue = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress/ExternalSymbol node, turn it into a
  // TargetGlobalAddress/TargetExternalSymbol node so that legalize won't
  // split it and then direct call can be matched by PseudoCALL.
  bool CalleeIsLargeExternalSymbol = false;
  if (getTargetMachine().getCodeModel() == CodeModel::Large) {
    // if (auto *S = dyn_cast<GlobalAddressSDNode>(Callee))
    //   Callee = getLargeGlobalAddress(S, DL, MVT::i32, DAG);
    // else if (auto *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    //   Callee = getLargeExternalSymbol(S, DL, MVT::i32, DAG);
    //   CalleeIsLargeExternalSymbol = true;
    // }
    assert(false);
  } else if (GlobalAddressSDNode *S = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = S->getGlobal();
    Callee =
        DAG.getTargetGlobalAddress(GV, DL, MVT::i32, 0, TricoreII::MO_CALL);
  } else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(S->getSymbol(), MVT::i32,
                                         TricoreII::MO_CALL);
  }

  // The first call operand is the chain and the second is the target address.
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (auto &Reg : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Glue the call to the argument copies, if any.
  if (Glue.getNode())
    Ops.push_back(Glue);

  assert((!CLI.CFIType || CLI.CB->isIndirectCall()) &&
         "Unexpected CFI type for a direct call");

  // Emit the call.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  if (IsTailCall) {
    MF.getFrameInfo().setHasTailCall();
    SDValue Ret = DAG.getNode(TricoreISD::TAIL, DL, NodeTys, Ops);
    if (CLI.CFIType)
      Ret.getNode()->setCFIType(CLI.CFIType->getZExtValue());
    DAG.addNoMergeSiteInfo(Ret.getNode(), CLI.NoMerge);
    return Ret;
  }

  Chain = DAG.getNode(TricoreISD::CALL, DL, NodeTys, Ops);
  if (CLI.CFIType)
    Chain.getNode()->setCFIType(CLI.CFIType->getZExtValue());
  DAG.addNoMergeSiteInfo(Chain.getNode(), CLI.NoMerge);
  Glue = Chain.getValue(1);

  // Mark the end of the call, which is glued to the call itself.
  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_TricoreEABI);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    auto &VA = RVLocs[i];
    // Copy the value out
    SDValue RetValue =
        DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), VA.getLocVT(), Glue);
    // Glue the RetValue to the end of the call sequence
    Chain = RetValue.getValue(1);
    Glue = RetValue.getValue(2);

    InVals.push_back(RetValue);
  }

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

SDValue TricoreTargetLowering::LowerGlobalAddress(SDValue Op,
                                                  SelectionDAG &DAG) const {
  EVT PtrVT = Op.getValueType();
  GlobalAddressSDNode *GSDN = cast<GlobalAddressSDNode>(Op);
  SDLoc DL(GSDN);
  const GlobalValue *GV = GSDN->getGlobal();
  EVT Ty = getPointerTy(DAG.getDataLayout());

  SDValue AddrHi = DAG.getTargetGlobalAddress(GV, DL, Ty, GSDN->getOffset(),
                                              TricoreII::MO_HI);
  SDValue AddrLo = DAG.getTargetGlobalAddress(GV, DL, Ty, GSDN->getOffset(),
                                              TricoreII::MO_LO);
  SDValue MNHi = DAG.getNode(TricoreISD::MOVHA, DL, Ty, AddrHi);
  return DAG.getNode(TricoreISD::LEA, DL, Ty, MNHi, AddrLo);
}

SDValue TricoreTargetLowering::LowerConstantPool(SDValue Op,
                                                 SelectionDAG &DAG) const {
  ConstantPoolSDNode *CPSDN = cast<ConstantPoolSDNode>(Op);
  SDLoc DL(CPSDN);
  const Constant *C = CPSDN->getConstVal();
  EVT Ty = getPointerTy(DAG.getDataLayout());

  SDValue AddrHi = DAG.getTargetConstantPool(
      C, Ty, std::nullopt, CPSDN->getOffset(), TricoreII::MO_HI);
  SDValue AddrLo = DAG.getTargetConstantPool(
      C, Ty, std::nullopt, CPSDN->getOffset(), TricoreII::MO_LO);
  SDValue MNHi = DAG.getNode(TricoreISD::MOVHA, DL, Ty, AddrHi);
  return DAG.getNode(TricoreISD::LEA, DL, Ty, MNHi, AddrLo);
}

SDValue TricoreTargetLowering::LowerConstant(SDValue Op,
                                             SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();
  SDLoc DL(Op);
  if (VT == MVT::i64) {
    // Expand to a constant pool using the default expansion code.
    return SDValue();
  }
  return Op; // For other types, just return the original node.
}

SDValue TricoreTargetLowering::LowerJumpTable(SDValue Op,
                                              SelectionDAG &DAG) const {
  EVT PtrVT = Op.getValueType();
  JumpTableSDNode *JSDN = cast<JumpTableSDNode>(Op);
  SDLoc DL(JSDN);
  EVT Ty = getPointerTy(DAG.getDataLayout());

  SDValue AddrHi =
      DAG.getTargetJumpTable(JSDN->getIndex(), Ty, TricoreII::MO_HI);
  SDValue AddrLo =
      DAG.getTargetJumpTable(JSDN->getIndex(), Ty, TricoreII::MO_LO);

  SDValue MNHi = DAG.getNode(TricoreISD::MOVHA, DL, Ty, AddrHi);
  return DAG.getNode(TricoreISD::LEA, DL, Ty, MNHi, AddrLo);
}

static SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) {
  MachineFunction &MF = DAG.getMachineFunction();
  TricoreMachineFunctionInfo *FuncInfo =
      MF.getInfo<TricoreMachineFunctionInfo>();

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  SDLoc DL(Op);
  EVT PtrVT = DAG.getTargetLoweringInfo().getPointerTy(DAG.getDataLayout());
  SDValue FR = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(), PtrVT);
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FR, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue TricoreTargetLowering::LowerOperation(SDValue Op,
                                              SelectionDAG &DAG) const {

  switch (Op.getOpcode()) {
  default:
    Op->dump();
    llvm_unreachable("Should not custom lower this!");
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::ConstantPool:
    return LowerConstantPool(Op, DAG);
  case ISD::Constant:
    return LowerConstant(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  }
}

static SDValue performSRACombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const TricoreSubtarget &Subtarget) {
  SDNode *Arg0 = N->getOperand(0).getNode();

  if (Arg0->getOpcode() == ISD::SHL &&
      N->getOperand(1)->getOpcode() == ISD::Constant &&
      Arg0->getOperand(1).getOpcode() == ISD::Constant) {
    int64_t SRAImm = N->getConstantOperandVal(1);
    int64_t SHLImm = Arg0->getConstantOperandVal(1);
    int64_t Width = 32 - SRAImm;
    int64_t Pos = 32 - SHLImm - Width;

    if (Pos >= 0 && Width > 0 && Pos + Width <= 32) {
      return DAG.getNode(TricoreISD::EXTR, SDLoc(N), MVT::i32,
                         Arg0->getOperand(0),
                         DAG.getConstant(Pos, SDLoc(N), MVT::i32),
                         DAG.getConstant(Width, SDLoc(N), MVT::i32));
    }
  }
  return SDValue();
}

static SDValue performANDCombine(SDNode *N, SelectionDAG &DAG,
                                 TargetLowering::DAGCombinerInfo &DCI,
                                 const TricoreSubtarget &Subtarget) {
  SDNode *Arg0 = N->getOperand(0).getNode();

  if (Arg0->getOpcode() == ISD::SRL &&
      N->getOperand(1)->getOpcode() == ISD::Constant &&
      Arg0->getOperand(1).getOpcode() == ISD::Constant) {
    int64_t ANDImm = N->getConstantOperandVal(1);
    int64_t SRLImm = Arg0->getConstantOperandVal(1);
    int64_t Width = __builtin_popcount(ANDImm);
    int64_t Pos = SRLImm;

    if (__builtin_ctz(ANDImm) != 0 || (__builtin_clz(ANDImm) + Width != 32)) {
      return SDValue();
    }

    if (Pos >= 0 && Width > 0 && Pos + Width <= 32) {
      return DAG.getNode(TricoreISD::EXTRU, SDLoc(N), MVT::i32,
                         Arg0->getOperand(0),
                         DAG.getConstant(Pos, SDLoc(N), MVT::i32),
                         DAG.getConstant(Width, SDLoc(N), MVT::i32));
    }
  }
  return SDValue();
}

static SDValue performORCombine(SDNode *N, SelectionDAG &DAG,
                                TargetLowering::DAGCombinerInfo &DCI,
                                const TricoreSubtarget &Subtarget) {
  SDNode *Arg0 = N->getOperand(0).getNode();
  SDNode *Arg1 = N->getOperand(1).getNode();

  bool HasInsertOps = Arg0->getOpcode() == ISD::AND &&
                      Arg0->getOperand(1).getOpcode() == ISD::Constant &&
                      Arg1->getOpcode() == ISD::AND &&
                      Arg1->getOperand(1).getOpcode() == ISD::Constant &&
                      (Arg0->getOperand(0).getOpcode() == ISD::SHL ||
                       Arg1->getOperand(0).getOpcode() == ISD::SHL);

  if (HasInsertOps) {
    uint32_t Arg0Mask = Arg0->getConstantOperandVal(1);
    uint32_t Arg1Mask = Arg1->getConstantOperandVal(1);
    if (Arg1Mask != ~Arg0Mask) {
      return SDValue();
    }

    if (Arg0->getOperand(0).getOpcode() == ISD::SHL &&
        Arg0->getOperand(0).getOperand(1).getOpcode() == ISD::Constant &&
        Arg0->getOperand(0).getConstantOperandVal(1) ==
            __builtin_ctz(Arg0Mask)) {
      return DAG.getNode(
          TricoreISD::INSERT, SDLoc(N), MVT::i32, Arg1->getOperand(0),
          Arg0->getOperand(0).getOperand(0), Arg0->getOperand(0).getOperand(1),
          DAG.getConstant(__builtin_popcount(Arg0Mask), SDLoc(N), MVT::i32));
    }
    if (Arg1->getOperand(0).getOpcode() == ISD::SHL &&
        Arg1->getOperand(0).getOperand(1).getOpcode() == ISD::Constant &&
        Arg1->getOperand(0).getConstantOperandVal(1) ==
            __builtin_ctz(Arg1Mask)) {
      return DAG.getNode(
          TricoreISD::INSERT, SDLoc(N), MVT::i32, Arg0->getOperand(0),
          Arg1->getOperand(0).getOperand(0), Arg1->getOperand(0).getOperand(1),
          DAG.getConstant(__builtin_popcount(Arg1Mask), SDLoc(N), MVT::i32));
    }
  }
  return SDValue();
}

SDValue TricoreTargetLowering::PerformDAGCombine(SDNode *N,
                                                 DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  case ISD::SRA:
    return performSRACombine(N, DCI.DAG, DCI, Subtarget);
  case ISD::AND:
    return performANDCombine(N, DCI.DAG, DCI, Subtarget);
  case ISD::OR:
    return performORCombine(N, DCI.DAG, DCI, Subtarget);
  default:
    return SDValue();
  }
}

/// ReplaceNodeResults - Replace the results of node with an illegal result
/// type with new values built out of custom code.
void TricoreTargetLowering::ReplaceNodeResults(
    SDNode *N, SmallVectorImpl<SDValue> &Results, SelectionDAG &DAG) const {
  SDValue Res;
  switch (N->getOpcode()) {
  default:
    N->dump();
    llvm_unreachable("Don't know how to custom expand this!");
  }
}

std::pair<unsigned, const TargetRegisterClass *>
TricoreTargetLowering::getRegForInlineAsmConstraint(
    const TargetRegisterInfo *TRI, StringRef Constraint, MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      if (VT == MVT::i64)
        return std::make_pair(0U, &Tricore::EGPRRegClass);
      if (VT == MVT::f32)
        return std::make_pair(0U, &Tricore::DGPRRegClass);
      if (VT == MVT::f64)
        return std::make_pair(0U, &Tricore::EGPRRegClass);
      return std::make_pair(0U, &Tricore::DGPRRegClass);
    case 'f':
      if (VT == MVT::f32) {
        return std::make_pair(0U, &Tricore::DGPRRegClass);
      } else if (VT == MVT::f64) {
        return std::make_pair(0U, &Tricore::DGPRRegClass);
      }
      break;
    case 'a':
      return std::make_pair(0U, &Tricore::AGPRRegClass);
    case 'A':
      return std::make_pair(0U, &Tricore::PGPRRegClass);
    default:
      break;
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}