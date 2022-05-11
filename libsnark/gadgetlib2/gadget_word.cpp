
/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                   DualWord_Gadget                      ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/
DualWord_Gadget::DualWord_Gadget(ProtoboardPtr pb,const DualWord& var,PackingMode packingMode)
        : Gadget(pb), m_dual_word(var), packingMode_(packingMode), packingGadget_() {}

void DualWord_Gadget::init() {
    packingGadget_ = CompressionPacking_Gadget::create(pb_, m_dual_word.unpacked(), m_dual_word.multipacked(),packingMode_);
}

GadgetPtr DualWord_Gadget::create(ProtoboardPtr pb,const DualWord& var,PackingMode packingMode) {
    GadgetPtr pGadget(new DualWord_Gadget(pb, var, packingMode));
    pGadget->init();
    return pGadget;
}

void DualWord_Gadget::generateConstraints() {
    packingGadget_->generateConstraints();
}

void DualWord_Gadget::generateWitness() {
    packingGadget_->generateWitness();
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
        const auto curGadget = CompressionPacking_Gadget::create(pb_, unpacked[i], packed[i],
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