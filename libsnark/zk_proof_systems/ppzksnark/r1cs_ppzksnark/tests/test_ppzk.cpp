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
#include <libsnark/gadgetlib2/integration.hpp>

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
    Another r1_inst showing the use of DualVariable. A DualVariable is a variable which holds both
    a bitwise representation of a word and a packed representation (e.g. both the packed value {42}
    and the unpacked value {1,0,1,0,1,0}). If the word is short enough
    (for r1_inst any integer smaller than the prime characteristic) then the packed representation
    will be stored in 1 field element. 'word' in this context means a set of bits, it is a
    convention which means we expect some semantic ability to decompose the packed value into its
    bits.
    The use of DualVariables is for efficiency reasons. More on this at the end of this r1_inst.
    In this r1_inst we will construct a gadget which receives as input a packed integer value
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
    const size_t difficultyBits_;
    DualWord m_dual_word;
    // This GadgetPtr will be a gadget to unpack m_dual_word from packed representation to bit
    // representation. Recall 'DualWord' holds both values, but only the packed version will be
    // received as input to the constructor.
    GadgetPtr m_bitGadget;

    HashDiffGadget(ProtoboardPtr pb,const MultiPackedWord& hashValue,const size_t difficultyBits)    : Gadget(pb),  difficultyBits_(difficultyBits),m_dual_word(hashValue, UnpackedWord(64, "u"))
{
}
    void init();
    DISALLOW_COPY_AND_ASSIGN(HashDiffGadget);
};


void HashDiffGadget::init() {
    // because we are using a prime field with large characteristic, we can assume a 64 bit value
    // fits in the first element of a multipacked variable.
    GADGETLIB_ASSERT(m_dual_word.multipacked().size() == 1, "multipacked word size too large");
    // A DualWord_Gadget's constraints assert that the unpacked and packed values represent the
    // same integer element. The generateWitness() method has two modes, one for packing (taking the
    // bit representation as input) and one for unpacking (creating the bit representation from
    // the packed representation)
    m_bitGadget = DualWord_Gadget::create(pb_, m_dual_word, PackingMode::UNPACK);
}

GadgetPtr HashDiffGadget::create(ProtoboardPtr pb,const MultiPackedWord& hashValue,
                                                const size_t difficultyBits) {
    GadgetPtr pGadget(new HashDiffGadget(pb, hashValue, difficultyBits));
    pGadget->init();
    return pGadget;
}

void HashDiffGadget::generateConstraints() {
    // enforce that both representations are equal
    m_bitGadget->generateConstraints();
    // add constraints asserting that the first 'difficultyBits' bits of 'hashValue' equal 0. Note
    // endianness, unpacked()[0] is LSB and unpacked()[63] is MSB
    for (size_t i = 0; i < difficultyBits_; ++i) {
        auto a=m_dual_word.unpacked()[63 - i];
        addRank1Constraint(a,1,0, GADGETLIB2_FMT("hash[%u] == 0", 63 - i));
    }
}

void HashDiffGadget::generateWitness() {
    // Take the packed representation and unpack to bits.
    m_bitGadget->generateWitness();
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
    const MultiPackedWord h(64, R1P, "h");
    auto g = HashDiffGadget::create(pb, h, 10);
    g->generateConstraints();

   
 
    pb->val(h[0]) = 42;
    g->generateWitness();
     cout<<pb->annotation()<<endl;
    // First 10 bits of 42 (when represented as a 64 bit number) are '0' so this should work
    cout<<"should true:"<<(pb->isSatisfied(PrintOptions::DBG_PRINT_IF_NOT_SATISFIED))<<endl;
    pb->val(h[0]) = 1000000000000000000;
    // This is a value > 2^54 so we expect constraint system not to be satisfied.
    g->generateWitness(); // This would have failed had we put an assertion
    cout<<"should false:"<<(pb->isSatisfied())<<endl;
}


/*
    In this r1_inst we will construct a gadget which builds a circuit for proof (witness) and
    validation (constraints) that a bitcoin transaction's sum of inputs equals the sum of
    outputs + miners fee. Construction of the proof will include finding the miners'
    fee. This fee can be thought of as an output of the circuit.

    This is a field specific gadget, as we will use the '+' operator freely. The addition
    operation works as expected over integers while in prime characteristic fields but not so in
    extension fields. If you are not familiar with extension fields, don't worry about it. Simply
    be aware that + and * behave differently in different fields and don't necessarily give the
    integer values you would expect.

    The library design supports multiple field constructs due to different applied use cases. Some
    cryptographic applications may need extension fields while others may need prime fields,
    but with constraints which are not rank-1, and yet others may need boolean circuits. The library
    was designed so that high level gadgets can be reused by implementing only the low level for
    a new field or constraint structure.

    Later we will supply a recipe for creation of such field specific gadgets with agnostic
    interfaces. We use a few conventions here in order to ease the process by using macros.
*/



// Notice the multiple inheritance. We must specify the interface as well as the field specific
// base gadget. This is what allows the factory class to decide at compile time which field
// specific class to instantiate for every protoboard. See design notes in "gadget.hpp"
// Convention is: class {FieldType}_{GadgetName}_Gadget
class R1P_Tx_Gadget : public Gadget {
public:
    void generateConstraints();
    void generateWitness();

    // We give the factory class friend access in order to instantiate via private constructor.
    friend class VerifyTransactionAmounts_Gadget;
public:
    R1P_Tx_Gadget(ProtoboardPtr pb,const VariableArray& txInputAmounts,
                                        const VariableArray& txOutputAmounts,
                                        const Variable& minersFee);
    void init(){}
private:

    const VariableArray txInputAmounts_;
    const VariableArray txOutputAmounts_;
    const Variable minersFee_;

    DISALLOW_COPY_AND_ASSIGN(R1P_Tx_Gadget);
};

// IMPLEMENTATION


void R1P_Tx_Gadget::generateConstraints() {
    addUnaryConstraint(sum(txInputAmounts_) - sum(txOutputAmounts_) - minersFee_,
                       "sum(txInputAmounts) == sum(txOutputAmounts) + minersFee");
    // It would seem this is enough, but an adversary could cause an overflow of one side of the
    // equation over the field modulus. In fact, for every input/output sum we will always find a
    // miners' fee which will satisfy this constraint!
    // It is left as an exercise for the reader to implement additional constraints (and witness)
    // to check that each of the amounts (inputs, outputs, fee) are between 0 and 21,000,000 * 1E8
    // satoshis. Combine this with a maximum amount of inputs/outputs to disallow field overflow.
    //
    // Hint: use Comparison_Gadget to create a gadget which compares a variable's assigned value
    // to a constant. Use a vector of these new gadgets to check each amount.
    // Don't forget to:
    // (1) Wire these gadgets in init()
    // (2) Invoke the gadgets' constraints in generateConstraints()
    // (3) Invoke the gadgets' witnesses in generateWitness()
}

void R1P_Tx_Gadget::generateWitness() {
    FElem sumInputs = 0;
    FElem sumOutputs = 0;
    for (const auto& inputAmount : txInputAmounts_) {
        sumInputs += val(inputAmount);
    }
    for (const auto& outputAmount : txOutputAmounts_) {
        sumOutputs += val(outputAmount);
    }
    val(minersFee_) = sumInputs - sumOutputs;
}

R1P_Tx_Gadget::R1P_Tx_Gadget(        ProtoboardPtr pb,const VariableArray& txInputAmounts,const VariableArray& txOutputAmounts,const Variable& minersFee)
        // Notice we must initialize 3 base classes (diamond inheritance):
        : Gadget(pb),txInputAmounts_(txInputAmounts), txOutputAmounts_(txOutputAmounts),
        minersFee_(minersFee) {}


/*
    As promised, recipe for creating field specific gadgets with agnostic interfaces:

    (1) Create the Base class using macro:
        CREATE_GADGET_BASE_CLASS({GadgetName}_GadgetBase);
    (2) Create the destructor for the base class:
        {GadgetName_Gadget}Base::~{GadgetName}_GadgetBase() {}
    (3) Create any field specific gadgets with multiple inheritance:
        class {FieldType}_{GadgetName}_Gadget : public {GadgetName}_GadgetBase,
                                                public {FieldType_Gadget}
        Notice all arguments to the constructors must be const& in order to use the factory class
        macro. Constructor arguments must be the same for all field specific implementations.
    (4) Give the factory class {GadgetName}_Gadget public friend access to the field specific
        classes.
    (5) Create the factory class using the macro:
        CREATE_GADGET_FACTORY_CLASS_XX({GadgetName}_Gadget, type1, input1, type2, input2, ... ,
                                                                                  typeXX, inputXX);
*/

void expect_true(bool b){
    cout<<"expect true:"<<b<<endl;
}
void expect_false(bool b){
    cout<<"expect false:"<<b<<endl;
}
void expect_eq(FElem a,FElem b){
    cout<<"expect eq:"<<"a:"<<a<<",b:"<<b<<endl;
}


/**
 * A R1CS r1_inst comprises a R1CS constraint system, R1CS input, and R1CS witness.
 */
template<typename FieldT>
struct r1cs_instance {
    r1cs_constraint_system<FieldT> constraint_system;
    r1cs_primary_input<FieldT> primary_input;
    r1cs_auxiliary_input<FieldT> auxiliary_input;

    r1cs_instance<FieldT>() = default;
    r1cs_instance<FieldT>(const r1cs_instance<FieldT> &other) = default;
    r1cs_instance<FieldT>(const r1cs_constraint_system<FieldT> &constraint_system,
                         const r1cs_primary_input<FieldT> &primary_input,
                         const r1cs_auxiliary_input<FieldT> &auxiliary_input) :
        constraint_system(constraint_system),
        primary_input(primary_input),
        auxiliary_input(auxiliary_input)
    {};
    r1cs_instance<FieldT>(r1cs_constraint_system<FieldT> &&constraint_system,
                         r1cs_primary_input<FieldT> &&primary_input,
                         r1cs_auxiliary_input<FieldT> &&auxiliary_input) :
        constraint_system(std::move(constraint_system)),
        primary_input(std::move(primary_input)),
        auxiliary_input(std::move(auxiliary_input))
    {};
};

/**
 * The code below provides an r1_inst of all stages of running a R1CS ppzkSNARK.
 *
 * Of course, in a real-life scenario, we would have three distinct entities,
 * mangled into one in the demonstration below. The three entities are as follows.
 * (1) The "generator", which runs the ppzkSNARK generator on input a given
 *     constraint system CS to create a proving and a verification key for CS.
 * (2) The "prover", which runs the ppzkSNARK prover on input the proving key,
 *     a primary input for CS, and an auxiliary input for CS.
 * (3) The "verifier", which runs the ppzkSNARK verifier on input the verification key,
 *     a primary input for CS, and a proof.
 */
template<typename ppT>
static bool run_r1cs_ppzksnark(const r1cs_instance<libff::Fr<ppT> > &r1_inst,const bool test_serialization)
{
    libff::enter_block("Call to run_r1cs_ppzksnark");

    r1cs_ppzksnark_keypair<ppT> keypair = r1cs_ppzksnark_generator<ppT>(r1_inst.constraint_system);
    printf("\n"); libff::print_indent(); libff::print_mem("after generator");

    libff::print_header("Preprocess verification key");
    r1cs_ppzksnark_processed_verification_key<ppT> pvk = r1cs_ppzksnark_verifier_process_vk<ppT>(keypair.vk);

    if (test_serialization)
    {
        libff::enter_block("Test serialization of keys");
        keypair.pk = libff::reserialize<r1cs_ppzksnark_proving_key<ppT> >(keypair.pk);
        keypair.vk = libff::reserialize<r1cs_ppzksnark_verification_key<ppT> >(keypair.vk);
        pvk = libff::reserialize<r1cs_ppzksnark_processed_verification_key<ppT> >(pvk);
        libff::leave_block("Test serialization of keys");
    }

    libff::print_header("R1CS ppzkSNARK Prover");
    r1cs_ppzksnark_proof<ppT> proof = r1cs_ppzksnark_prover<ppT>(keypair.pk, r1_inst.primary_input, r1_inst.auxiliary_input);
    printf("\n"); libff::print_indent(); libff::print_mem("after prover");

    if (test_serialization)
    {
        libff::enter_block("Test serialization of proof");
        proof = libff::reserialize<r1cs_ppzksnark_proof<ppT> >(proof);
        libff::leave_block("Test serialization of proof");
    }

    libff::print_header("R1CS ppzkSNARK Verifier");
    const bool ans = r1cs_ppzksnark_verifier_strong_IC<ppT>(keypair.vk, r1_inst.primary_input, proof);
    printf("\n"); libff::print_indent(); libff::print_mem("after verifier");
    printf("* The verification result is: %s\n", (ans ? "PASS" : "FAIL"));

    libff::print_header("R1CS ppzkSNARK Online Verifier");
    const bool ans2 = r1cs_ppzksnark_online_verifier_strong_IC<ppT>(pvk, r1_inst.primary_input, proof);
    assert(ans == ans2);

    test_affine_verifier<ppT>(keypair.vk, r1_inst.primary_input, proof, ans);

    libff::leave_block("Call to run_r1cs_ppzksnark");

    return (ans && ans2);
}

void test_tx_gadget() {
    ::gadgetlib2::GadgetLibAdapter::resetVariableIndex();
    auto pb = Protoboard::create(R1P);
    const VariableArray in(2, "in");
    const VariableArray out(3, "ou");
    const Variable minersFee("fee");
    R1P_Tx_Gadget g(pb, in, out,minersFee);
    g.generateConstraints();
    pb->val(in[0]) = pb->val(in[1]) = 2;
    pb->val(out[0]) = pb->val(out[1]) = pb->val(out[2]) = 1;
    g.generateWitness();
    
    cout<<pb->annotation()<<endl;

    expect_true(pb->isSatisfied());
    expect_eq(pb->val(minersFee), 1);
  
    typedef  alt_bn128_pp::Fp_type FP;
      // translate constraint system to libsnark format.
    r1cs_constraint_system<FP> cs = get_constraint_system_from_gadgetlib2(*pb);
    // translate full variable assignment to libsnark format
    const r1cs_variable_assignment<FP> full_assignment = get_variable_assignment_from_gadgetlib2(*pb);
    // extract primary and auxiliary input
    const r1cs_primary_input<FP> primary_input(full_assignment.begin(), full_assignment.begin() + cs.num_inputs());
    const r1cs_auxiliary_input<FP> auxiliary_input(full_assignment.begin() + cs.num_inputs(), full_assignment.end());

    assert(cs.is_valid());
    assert(cs.is_satisfied(primary_input, auxiliary_input));

    r1cs_instance<FP> x(cs, primary_input, auxiliary_input);

     // Run ppzksnark. Jump into function for breakdown
    const bool bit = run_r1cs_ppzksnark<libff::alt_bn128_pp>(x, false);
    expect_true(bit);
}

int main()
{
     libff::alt_bn128_pp::init_public_params();

    test_hash();
   //  g();

    /*
   
    libff::start_profiling();

    libff::print_header("(enter) Test R1CS ppzkSNARK");

    r1cs_instance<libff::Fr<alt_bn128_pp> > r1_inst = g_input<libff::Fr<alt_bn128_pp> >(100, 10);
    const bool bit = run_r1cs_ppzksnark<alt_bn128_pp>(r1_inst, true);
    assert(bit);

    libff::print_header("(leave) Test R1CS ppzkSNARK");*/
}
