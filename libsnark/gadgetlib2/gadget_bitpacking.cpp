/*
    Constraint breakdown:

    (1) packed = sum(unpacked[i] * 2^i)
    (2) (UNPACK only) unpacked[i] is Boolean.
*/

R1P_BitPacking_Gadget::R1P_BitPacking_Gadget(ProtoboardPtr pb,const VariableArray& unpacked,
                                                             const VariableArray& packed,
                                                             PackingMode packingMode)
    : Gadget(pb), CompressionPacking_GadgetBase(pb), R1P_Gadget(pb), packingMode_(packingMode),
      unpacked_(unpacked), packed_(packed) {
    const int n = unpacked.size();
    GADGETLIB_ASSERT(n > 0, "Attempted to pack 0 bits in R1P.")
    GADGETLIB_ASSERT(packed.size() == 1,
                 "Attempted to pack into more than 1 Variable in R1P_BitPacking_Gadget.")
    // TODO add assertion that 'n' bits can fit in the field characteristic
}


void R1P_BitPacking_Gadget::generateConstraints() {
    const int n = unpacked_.size();
    LinearCombination packed;
    FElem two_i(R1P_Elem(1)); // Will hold 2^i
    for (int i = 0; i < n; ++i) {
        packed += unpacked_[i]*two_i;
        two_i += two_i;
        if (packingMode_ == PackingMode::UNPACK)
             {
                pb_->enforceBooleanity(unpacked_[i]);
            }
    }
    addRank1Constraint(packed_[0], 1, packed, "packed[0] = sum(2^i * unpacked[i])");
}

void R1P_BitPacking_Gadget::generateWitness() {
    const int n = unpacked_.size();
    if (packingMode_ == PackingMode::PACK) {
        FElem packedVal = 0;
        FElem two_i(R1P_Elem(1)); // will hold 2^i
        for(int i = 0; i < n; ++i) {
            GADGETLIB_ASSERT(val(unpacked_[i]).asLong() == 0 || val(unpacked_[i]).asLong() == 1,
                         GADGETLIB2_FMT("unpacked[%u]  = %u. Expected a Boolean value.", i,
                             val(unpacked_[i]).asLong()));
            packedVal += two_i * val(unpacked_[i]).asLong();
            two_i += two_i;
        }
        val(packed_[0]) = packedVal;
        return;
    }
    // else (UNPACK)
    GADGETLIB_ASSERT(packingMode_ == PackingMode::UNPACK, "Packing gadget created with unknown packing mode.");
    for(int i = 0; i < n; ++i) {
        val(unpacked_[i]) = val(packed_[0]).getBit(i, R1P);
    }
}