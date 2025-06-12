def CSR_EABI : CalleeSavedRegs<(add)> {
  let OtherPreserved = (add (sequence "D%u", 8, 15), (sequence "A%u", 10, 15));
}

def RetCC_TricoreEABI : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[D2]>>,
  CCIfType<[i64], CCAssignToReg<[E2]>>,
  CCIfType<[f32], CCAssignToReg<[D2]>>,
  CCIfType<[f64], CCAssignToReg<[E2]>>
]>;