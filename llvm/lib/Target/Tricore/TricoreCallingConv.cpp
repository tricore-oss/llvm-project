#include "TricoreCallingConv.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/CodeGenTypes/MachineValueType.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/Alignment.h"

using namespace llvm;

static const MCPhysReg PointerArgList[] = {Tricore::A4, Tricore::A5,
                                           Tricore::A6, Tricore::A7};

static const MCPhysReg DataArgList[] = {Tricore::D4, Tricore::D5, Tricore::D6,
                                        Tricore::D7};

bool llvm::CC_TricoreEABI(unsigned ValNo, MVT ValVT, MVT LocVT,
                          CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  const TargetRegisterInfo *RI =
      State.getMachineFunction().getSubtarget().getRegisterInfo();

  if (!ArgFlags.isByVal() && ArgFlags.isPointer()) {
    if (MCRegister Reg = State.AllocateReg(PointerArgList)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }
  if (ValVT == MVT::i32 || ValVT == MVT::f32 ||
      (ArgFlags.isByVal() && ArgFlags.getByValSize() <= 4)) {
    if (MCRegister Reg = State.AllocateReg(DataArgList)) {
      for (auto SuperReg : RI->superregs(Reg)) {
        State.AllocateReg(SuperReg);
      }
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    } else {
      int64_t Offset = State.AllocateStack(4, Align(4));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
      return false;
    }
  }
  if (ValVT == MVT::i64) {
    if (MCRegister Reg = State.AllocateReg(DataArgList)) {
      for (auto SubReg : RI->subregs(Reg)) {
        State.AllocateReg(SubReg);
      }
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  assert(false);

  return true;
}