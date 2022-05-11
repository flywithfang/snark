/** @file
 *****************************************************************************
 Test program that exercises the ppzkSNARK (first generator, then
 prover, then verifier) on a synthetic R1CS instance.

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#include <cassert>
#include <cstdio>

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libsnark/relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp>


#include <libsnark/gadgetlib2/gadget.hpp>
#include <libsnark/gadgetlib2/adapters.hpp>

using namespace libsnark;
using namespace libff;
using namespace std;

using namespace gadgetlib2;



class NAND_Gadget : public Gadget {
public:
    // This is a convention we use to always create gadgets as if from a factory class. This will
    // be needed later for gadgets which have different implementations in different fields.
    static GadgetPtr create(ProtoboardPtr pb,
                            const VariableArray& ip,
                            const Variable& output);
    // generateConstraints() is the method which creates all constraints on the protoboard
    void generateConstraints();
    // generateWitness() is the method which generates the witness by assigning a valid value to
    // each wire in the circuit (variable) and putting this on the protoboard
    void generateWitness();
private:
    // constructor is private in order to stick to the convention that gadgets are created using a
    // create() method. This may not make sense now, but when dealing with non-field agnostic
    // gadgets it is very convenient to have a factory class with this convention.
    // Notice the protoboard. This can be thought of as a 'memory manager' which holds the circuit
    // as the constraints are being built, and the 'wire values' as the witness is being built
    NAND_Gadget(ProtoboardPtr pb, const VariableArray& ip, const Variable& output);
    // init() does any non trivial work which we don't want in the constructor. This is where we
    // will 'wire' the sub-gadgets into the circuit. Each sub-gadget can be thought of as a
    // circuit gate with some specific functionality.
    void init();
    // we want every gadget to be explicitly constructed
    DISALLOW_COPY_AND_ASSIGN(NAND_Gadget);

    // This is an internal gadget. Once a gadget is created it can be used as a black box gate. We
    // will initialize this pointer to be an AND_Gadget in the init() method.
    GadgetPtr andGadget_;
    // These are internal variables used by the class. They will always include the variables from
    // the constructor, but can include many more as well. Notice that almost always the variables
    // can be declared 'const', as these are local copies of formal variables, and do not change
    // over the span of the class' lifetime.
    const VariableArray inputs_;
    const Variable output_;
    const Variable andResult_;
};

// IMPLEMENTATION
// Most constructors are trivial and only initialize and assert values.
NAND_Gadget::NAND_Gadget(ProtoboardPtr pb,
                         const VariableArray& ip,
                         const Variable& output)
        : Gadget(pb), inputs_(ip), output_(output), andResult_("andResult") {}

void NAND_Gadget::init() {
    // we 'wire' the AND gate.
    andGadget_ = AND_Gadget::create(pb_, inputs_, andResult_);
}

// The create() method will usually look like this, for field-agnostic gadgets:
GadgetPtr NAND_Gadget::create(ProtoboardPtr pb,const VariableArray& ip,const Variable& output) {
    GadgetPtr pGadget(new NAND_Gadget(pb, ip, output));
    pGadget->init();
    return pGadget;
}

void NAND_Gadget::generateConstraints() {
    // we will invoke the AND gate constraint generator
    andGadget_->generateConstraints();
    // and add our out negation constraint in order to make this a NAND gate
    addRank1Constraint(1, 1 - andResult_, output_, "1 * (1 - andResult) = output");
    // Another way to write the same constraint is:
    // addUnaryConstraint(1 - andResult_ - output_, "1 - andResult == output");
    //
    // At first look, it would seem that this is enough. However, the AND_Gadget expects all of its
    // ip to be boolean, a dishonest prover could put non-boolean ip, so we must check this
    // here. Notice 'Variable' means a variable which we intend to hold only '0' or '1', but
    // this is just a convention (it is a typedef for Variable) and we must enforce it.
    // Look into the internals of the R1P implementation of AND_Gadget and see that
    // {2, 1, 0} as ip with {1} as output would satisfy all constraints, even though this is
    // clearly not our intent!
    for (const auto& input : inputs_) {
        enforceBooleanity(input); // This adds a constraint of the form: input * (1 - input) == 0
    }
}

void NAND_Gadget::generateWitness() {
    // First we can assert that all input values are indeed boolean. The purpose of this assertion
    // is simply to print a clear error message, it is not security critical.
    // Notice the method val() which returns a reference to the current assignment for a variable
    for (const auto& input : inputs_) {
        GADGETLIB_ASSERT(val(input) == 0 || val(input) == 1, "NAND input is not boolean");
    }
    // we will invoke the AND gate witness generator, this will set andResult_ correctly
    andGadget_->generateWitness();
    // and now we set the value of output_
    val(output_) = 1 - val(andResult_);
    // notice the use of 'val()' to tell the protoboard to assign this new value to the
    // variable 'output_'. The variable itself is only a formal variable and never changes.
}

// And now for a test which will exemplify the usage:
void test_nand(){
     ::gadgetlib2::GadgetLibAdapter::resetVariableIndex();
    // create a protoboard for a system of rank 1 constraints over a prime field.
    ProtoboardPtr pb = Protoboard::create(R1P);
    // create 5 variables ip[0]...ip[4]. The string "ip" is used for debug messages
    VariableArray ip(5, "ip");
    Variable output("output");
    cout<<"next-index:"<<GadgetLibAdapter::getNextFreeIndex()<<endl;

    GadgetPtr nandGadget = NAND_Gadget::create(pb, ip, output);
    cout<<"next-index:"<<GadgetLibAdapter::getNextFreeIndex()<<endl;
    // now we can generate a constraint system (or circuit)
    nandGadget->generateConstraints();
    cout<<"next-index:"<<GadgetLibAdapter::getNextFreeIndex()<<endl;
    // so let's assign the input variables for NAND and try again after creating the witness
    for (const auto& input : ip) {
        pb->val(input) = 1;
    }
    nandGadget->generateWitness();
   cout<<"expect true "<<pb->isSatisfied()<<endl;
    cout<<"pb->val(output) == 0:"<<pb->val(output)<<endl;
    pb->val(ip[2]) = 0;
    cout<<"expect false "<<pb->isSatisfied()<<endl;
    // now let's try to cheat. If we hadn't enforced booleanity, this would have worked!
    pb->val(ip[1]) = 2;
    cout<<"expect false "<<pb->isSatisfied()<<endl;
    // now let's reset ip[1] to a valid value
    pb->val(ip[1]) = 1;
    // before, we set both the ip and the output. Notice the output is still set to '0'
    cout<<"pb->val(output) == 0:"<<pb->val(output)<<endl;
    // Now we will let the gadget compute the result using generateWitness() and see what happens
    nandGadget->generateWitness();
    cout<<"pb->val(output) == 1:"<<pb->val(output)<<endl;

    cout<<"expect true "<<pb->isSatisfied()<<endl;
}



/*
    Another example showing the use of DualVariable. A DualVariable is a variable which holds both
    a bitwise representation of a word and a packed representation (e.g. both the packed value {42}
    and the unpacked value {1,0,1,0,1,0}). If the word is short enough
    (for example any integer smaller than the prime characteristic) then the packed representation
    will be stored in 1 field element. 'word' in this context means a set of bits, it is a
    convention which means we expect some semantic ability to decompose the packed value into its
    bits.
    The use of DualVariables is for efficiency reasons. More on this at the end of this example.
    In this example we will construct a gadget which receives as input a packed integer value
    called 'hash', and a 'difficulty' level in bits, and constructs a circuit validating that the
    first 'difficulty' bits of 'hash' are '0'. For simplicity we will assume 'hash' is always 64
    bits long.
*/

class HashDiffGadget : public Gadget {
public:
    static GadgetPtr create(ProtoboardPtr pb,const MultiPackedWord& hashValue,const size_t difficultyBits);
    void generateConstraints();
    void generateWitness();
private:
    const size_t hashSizeInBits_;
    const size_t difficultyBits_;
    DualWord m_dual_word;
    // This GadgetPtr will be a gadget to unpack m_dual_word from packed representation to bit
    // representation. Recall 'DualWord' holds both values, but only the packed version will be
    // received as input to the constructor.
    GadgetPtr hashValueUnpacker_;

    HashDiffGadget(ProtoboardPtr pb,const MultiPackedWord& hashValue,const size_t difficultyBits);
    void init();
    DISALLOW_COPY_AND_ASSIGN(HashDiffGadget);
};

// IMPLEMENTATION
HashDiffGadget::HashDiffGadget(ProtoboardPtr pb,const MultiPackedWord& hashValue,const size_t difficultyBits)
    : Gadget(pb), hashSizeInBits_(64), difficultyBits_(difficultyBits),m_dual_word(hashValue, UnpackedWord(64, "m_dual_wordu"))
{
}

void HashDiffGadget::init() {
    // because we are using a prime field with large characteristic, we can assume a 64 bit value
    // fits in the first element of a multipacked variable.
    GADGETLIB_ASSERT(m_dual_word.multipacked().size() == 1, "multipacked word size too large");
    // A DualWord_Gadget's constraints assert that the unpacked and packed values represent the
    // same integer element. The generateWitness() method has two modes, one for packing (taking the
    // bit representation as input) and one for unpacking (creating the bit representation from
    // the packed representation)
    hashValueUnpacker_ = DualWord_Gadget::create(pb_, m_dual_word, PackingMode::UNPACK);
}

GadgetPtr HashDiffGadget::create(ProtoboardPtr pb,
                                                const MultiPackedWord& hashValue,
                                                const size_t difficultyBits) {
    GadgetPtr pGadget(new HashDiffGadget(pb, hashValue, difficultyBits));
    pGadget->init();
    return pGadget;
}

void HashDiffGadget::generateConstraints() {
    // enforce that both representations are equal
    hashValueUnpacker_->generateConstraints();
    // add constraints asserting that the first 'difficultyBits' bits of 'hashValue' equal 0. Note
    // endianness, unpacked()[0] is LSB and unpacked()[63] is MSB
    for (size_t i = 0; i < difficultyBits_; ++i) {
        addUnaryConstraint(m_dual_word.unpacked()[63 - i], GADGETLIB2_FMT("hashValue[%u] == 0", 63 - i));
    }
}

void HashDiffGadget::generateWitness() {
    // Take the packed representation and unpack to bits.
    hashValueUnpacker_->generateWitness();
    // In a real setting we would add an assertion that the value will indeed satisfy the
    // difficulty constraint, and notify the user with an error otherwise. As this is a tutorial,
    // we'll let invalid values pass through so that we can see how isSatisfied() returns false.
}

// Remember we pointed out that DualVariables are used for efficiency reasons. Now is the time to
// elaborate on this. As you've seen, we needed a bit representation in order to check the first
// bits of hashValue. But hashValue may be used in many other places, for instance we may want to
// check equality with another value. Checking equality on a packed representation will 'cost' us
// 1 constraint, while checking equality on the unpacked value will 'cost' us 64 constraints. This
// translates heavily into proof construction time and memory in the ppzkSNARK proof system.

void test_hash( ) {
    auto pb = Protoboard::create(R1P);
    const MultiPackedWord hashValue(64, R1P, "hashValue");
    auto g = HashDiffGadget::create(pb, hashValue, 10);
    g->generateConstraints();
 
    pb->val(hashValue[0]) = 42;
    g->generateWitness();
    // First 10 bits of 42 (when represented as a 64 bit number) are '0' so this should work
    cout<<"should true:"<<(pb->isSatisfied(PrintOptions::DBG_PRINT_IF_NOT_SATISFIED));
    pb->val(hashValue[0]) = 1000000000000000000;
    // This is a value > 2^54 so we expect constraint system not to be satisfied.
    g->generateWitness(); // This would have failed had we put an assertion
    cout<<"should false:"<<(pb->isSatisfied());
}


int main()
{
     libff::alt_bn128_pp::init_public_params();

     test_hash();
   //  g();

    /*
   
    libff::start_profiling();

    libff::print_header("(enter) Test R1CS ppzkSNARK");

    r1cs_example<libff::Fr<alt_bn128_pp> > example = g_input<libff::Fr<alt_bn128_pp> >(100, 10);
    const bool bit = run_r1cs_ppzksnark<alt_bn128_pp>(example, true);
    assert(bit);

    libff::print_header("(leave) Test R1CS ppzkSNARK");*/
}
