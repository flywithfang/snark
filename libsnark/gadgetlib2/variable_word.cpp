
/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                 Custom Variable classes                    ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/

MultiPackedWord::MultiPackedWord(const FieldType& fieldType)
        : VariableArray(), numBits_(0), fieldType_(fieldType) {}

MultiPackedWord::MultiPackedWord(const size_t numBits,
                                 const FieldType& fieldType,
                                 const ::std::string& name)
        : VariableArray(), numBits_(numBits), fieldType_(fieldType) {
    size_t packedSize = getMultipackedSize();
    VariableArray varArray(packedSize, name);
    VariableArray::swap(varArray);
}

void MultiPackedWord::resize(const size_t numBits) {
    numBits_ = numBits;
    size_t packedSize = getMultipackedSize();
    VariableArray::resize(packedSize);
}

size_t MultiPackedWord::getMultipackedSize() const {
    size_t packedSize = 0;
    if (fieldType_ == R1P) {
        packedSize = 1; // TODO add assertion that numBits can fit in the field characteristic
    } else {
        GADGETLIB_FATAL("Unknown field type for packed variable.");
    }
    return packedSize;
}

DualWord::DualWord(const size_t numBits,
                   const FieldType& fieldType,
                   const ::std::string& name)
        : multipacked_(numBits, fieldType, name + "_p"),
          unpacked_(numBits, name + "_u") {}

DualWord::DualWord(const MultiPackedWord& multipacked, const UnpackedWord& unpacked)
        : multipacked_(multipacked), unpacked_(unpacked) {}

void DualWord::resize(size_t newSize) {
    multipacked_.resize(newSize);
    unpacked_.resize(newSize);
}




DualWordArray::DualWordArray(const FieldType& fieldType)
        : multipackedContents_(0, MultiPackedWord(fieldType)), unpackedContents_(0),
          numElements_(0) {}

DualWordArray::DualWordArray(const MultiPackedWordArray& multipackedContents, // TODO delete, for dev
                             const UnpackedWordArray& unpackedContents)
        : multipackedContents_(multipackedContents), unpackedContents_(unpackedContents),
            numElements_(multipackedContents_.size()) {
    GADGETLIB_ASSERT(multipackedContents_.size() == numElements_,
                    "Dual Variable multipacked contents size mismatch");
    GADGETLIB_ASSERT(unpackedContents_.size() == numElements_,
                    "Dual Variable packed contents size mismatch");
}

MultiPackedWordArray DualWordArray::multipacked() const {return multipackedContents_;}
UnpackedWordArray DualWordArray::unpacked() const {return unpackedContents_;}
PackedWordArray DualWordArray::packed() const {
    GADGETLIB_ASSERT(numElements_ == multipackedContents_.size(), "multipacked contents size mismatch")
    PackedWordArray retval(numElements_);
    for(size_t i = 0; i < numElements_; ++i) {
        const auto element = multipackedContents_[i];
        GADGETLIB_ASSERT(element.size() == 1, "Cannot convert from multipacked to packed");
        retval[i] = element[0];
    }
    return retval;
}

void DualWordArray::push_back(const DualWord& dualWord) {
    multipackedContents_.push_back(dualWord.multipacked());
    unpackedContents_.push_back(dualWord.unpacked());
    ++numElements_;
}

DualWord DualWordArray::at(size_t i) const {
    //const MultiPackedWord multipackedRep = multipacked()[i];
    //const UnpackedWord unpackedRep = unpacked()[i];
    //const DualWord retval(multipackedRep, unpackedRep);
    //return retval;
    return DualWord(multipacked()[i], unpacked()[i]);
}

size_t DualWordArray::size() const {
    GADGETLIB_ASSERT(multipackedContents_.size() == numElements_,
                    "Dual Variable multipacked contents size mismatch");
    GADGETLIB_ASSERT(unpackedContents_.size() == numElements_,
                    "Dual Variable packed contents size mismatch");
    return numElements_;
}
