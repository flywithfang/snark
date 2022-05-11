
/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                   DualWord_Gadget                      ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/
//TODO add test

class DualWord_Gadget : public Gadget {

private:
    const DualWord m_dual_word;
    const PackingMode packingMode_;

    GadgetPtr packingGadget_;

    DualWord_Gadget(ProtoboardPtr pb, const DualWord& var, PackingMode packingMode);
    virtual void init();
    DISALLOW_COPY_AND_ASSIGN(DualWord_Gadget);
public:
    static GadgetPtr create(ProtoboardPtr pb, const DualWord& var, PackingMode packingMode);
    void generateConstraints();
    void generateWitness();
};

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
//TODO add test

class DualWordArray_Gadget : public Gadget {

private:
    const DualWordArray vars_;
    const PackingMode packingMode_;

    ::std::vector<GadgetPtr> packingGadgets_;

    DualWordArray_Gadget(ProtoboardPtr pb,
                             const DualWordArray& vars,
                             PackingMode packingMode);
    virtual void init();
    DISALLOW_COPY_AND_ASSIGN(DualWordArray_Gadget);
public:
    static GadgetPtr create(ProtoboardPtr pb,
                            const DualWordArray& vars,
                            PackingMode packingMode);
    void generateConstraints();
    void generateWitness();
};
