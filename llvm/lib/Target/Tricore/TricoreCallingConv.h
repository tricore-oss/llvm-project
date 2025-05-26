
#include "MCTargetDesc/TricoreBaseInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"

namespace llvm {

bool CC_TricoreEABI(unsigned ValNo, MVT ValVT, MVT LocVT,
                    CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                    CCState &State);
}