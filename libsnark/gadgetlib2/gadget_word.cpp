
/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                   DualWord_Gadget                      ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/
DualWord_Gadget::DualWord_Gadget(ProtoboardPtr pb,const DualWord& var,PackingMode packingMode)
        : Gadget(pb), m_dual_word(var), packingMode_(packingMode) {}


GadgetPtr DualWord_Gadget::create(ProtoboardPtr pb,const DualWord& var,PackingMode packingMode) {
    GadgetPtr pGadget(new DualWord_Gadget(pb, var, packingMode));
    pGadget->init();
    return pGadget;
}

/*
    Constraint breakdown:

    (1) packed = sum(unpacked[i] * 2^i)
    (2) (UNPACK only) unpacked[i] is Boolean.
*/

void DualWord_Gadget::generateConstraints() {
    auto unpacked = m_dual_word.unpacked();
    LinearCombination packed;
    FElem two_i(R1P_Elem(1)); // Will hold 2^i
    for(auto bit_var : unpacked){
        packed += bit_var*two_i;
        two_i += two_i;
        if (packingMode_ == PackingMode::UNPACK)
             {
                pb_->enforceBooleanity(bit_var);
            }
    }
    auto packed_   = m_dual_word.multipacked();
    addRank1Constraint(packed_[0], 1, packed, "packed[0] = sum(2^i * unpacked[i])");
}

void DualWord_Gadget::generateWitness() {
     auto unpacked_ = m_dual_word.unpacked();
     auto packed_   = m_dual_word.multipacked();
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
    int i=0;
    for(auto bit_var :unpacked_) {
        pb_->val(bit_var) = pb_->val(packed_[0]).getBit(i++, R1P);
    }
}

/*********************************/
/***       END OF Gadget       ***/
/*********************************/

/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                 DualWordArray_Gadget                   ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/
DualWordArray_Gadget::DualWordArray_Gadget(ProtoboardPtr pb,
                                           const DualWordArray& vars,
                                           PackingMode packingMode)
        : Gadget(pb), vars_(vars), packingMode_(packingMode), packingGadgets_() {}

void DualWordArray_Gadget::init() {
    const UnpackedWordArray unpacked = vars_.unpacked();
    const MultiPackedWordArray packed = vars_.multipacked();
    for(size_t i = 0; i < vars_.size(); ++i) {
        const auto curGadget = BitPacking_Gadget::create(pb_, unpacked[i], packed[i],
                                                                 packingMode_);
        packingGadgets_.push_back(curGadget);
    }
}

GadgetPtr DualWordArray_Gadget::create(ProtoboardPtr pb,
                                           const DualWordArray& vars,
                                           PackingMode packingMode) {
    GadgetPtr pGadget(new DualWordArray_Gadget(pb, vars, packingMode));
    pGadget->init();
    return pGadget;
}

void DualWordArray_Gadget::generateConstraints() {
    for(auto& gadget : packingGadgets_) {
        gadget->generateConstraints();
    }
}

void DualWordArray_Gadget::generateWitness() {
    for(auto& gadget : packingGadgets_) {
        gadget->generateWitness();
    }
}

/*********************************/
/***       END OF Gadget       ***/
/*********************************/