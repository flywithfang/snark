                       
typedef VariableArray PackedWordArray;

/// Holds variables whose purpose is to be populated with the unpacked form of some word, bit by bit
class UnpackedWord : public VariableArray {
public:
    UnpackedWord() : VariableArray() {}
    UnpackedWord(const size_t numBits, const ::std::string& name) : VariableArray(numBits, name) {}
}; // class UnpackedWord

typedef ::std::vector<UnpackedWord> UnpackedWordArray;

/// Holds variables whose purpose is to be populated with the packed form of some word.
/// word representation can be larger than a single field element in small enough fields
class MultiPackedWord : public VariableArray {
private:
    size_t numBits_;
    FieldType fieldType_;
    size_t getMultipackedSize() const;
public:
    MultiPackedWord(const FieldType& fieldType = AGNOSTIC);
    MultiPackedWord(const size_t numBits, const FieldType& fieldType, const ::std::string& name);
    void resize(const size_t numBits);
    ::std::string name() const {return VariableArray::name();}
}; // class MultiPackedWord

typedef ::std::vector<MultiPackedWord> MultiPackedWordArray;

/// Holds both representations of a word, both multipacked and unpacked
class DualWord {
private:
    MultiPackedWord multipacked_;
    UnpackedWord unpacked_;
public:
    DualWord(const FieldType& fieldType) : multipacked_(fieldType), unpacked_() {}
    DualWord(const size_t numBits, const FieldType& fieldType, const ::std::string& name);
    DualWord(const MultiPackedWord& multipacked, const UnpackedWord& unpacked);
    MultiPackedWord multipacked() const {return multipacked_;}
    UnpackedWord unpacked() const {return unpacked_;}
    FlagVariable bit(size_t i) const {return unpacked_[i];} //syntactic sugar, same as unpacked()[i]
    size_t numBits() const { return unpacked_.size(); }
    void resize(size_t newSize);
}; // class DualWord

class DualWordArray {
private:
    // kept as 2 separate arrays because the more common usecase will be to request one of these,
    // and not dereference a specific DualWord
    std::vector<MultiPackedWord> multipackedContents_;
    std::vector<UnpackedWord>    unpackedContents_;
    size_t numElements_;
public:
    DualWordArray(const FieldType& fieldType);
    DualWordArray(const MultiPackedWordArray& multipackedContents, // TODO delete, for dev
                  const UnpackedWordArray& unpackedContents);
    MultiPackedWordArray multipacked() const;
    UnpackedWordArray unpacked() const;
    PackedWordArray packed() const; //< For cases in which we can assume each unpacked value fits
                                    //< in 1 packed Variable
    void push_back(const DualWord& dualWord);
    DualWord at(size_t i) const;
    size_t size() const;
}; // class DualWordArray
