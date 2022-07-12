class R1P_BitPacking_Gadget : public CompressionPacking_GadgetBase, public R1P_Gadget {
private:
    PackingMode packingMode_;
    R1P_BitPacking_Gadget(ProtoboardPtr pb,const VariableArray& unpacked,const VariableArray& packed,
                                  PackingMode packingMode);
    virtual void init(){}
public:
    const VariableArray unpacked_;
    const VariableArray packed_;
    void generateConstraints();
    void generateWitness();
    friend class BitPacking_Gadget;
private:
    DISALLOW_COPY_AND_ASSIGN(R1P_BitPacking_Gadget);
};
