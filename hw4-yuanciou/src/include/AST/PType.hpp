#ifndef AST_P_TYPE_H
#define AST_P_TYPE_H

#include <memory>
#include <string>
#include <vector>

class PType;

using PTypeSharedPtr = std::shared_ptr<PType>;

class PType {
  public:
    enum class PrimitiveTypeEnum : uint8_t {
        kVoidType,
        kIntegerType,
        kRealType,
        kBoolType,
        kStringType,
        kErrorType
    };

  private:
    PrimitiveTypeEnum m_type;
    std::vector<uint64_t> m_dimensions;
    mutable std::string m_type_string;
    mutable bool m_type_string_is_valid = false;

  public:
    ~PType() = default;
    PType(const PrimitiveTypeEnum type) : m_type(type) {}

    void setDimensions(std::vector<uint64_t> &p_dims) {
        m_dimensions = std::move(p_dims);
    }

    PrimitiveTypeEnum getPrimitiveType() const { return m_type; }
    const char *getPTypeCString() const;

    //
    bool zeroDimention()
    {
      for(auto &dimension : m_dimensions)
      {
        if(dimension < 1) 
        {
          return true;
        }
      }
      return false;
    }

    int getDimensionsSize() { return m_dimensions.size(); }
    std::vector<uint64_t> getDimensions(){ return m_dimensions; }

    std::string getType(){
      switch (m_type) {
          case PrimitiveTypeEnum::kVoidType:
              return "void";
          case PrimitiveTypeEnum::kIntegerType:
              return "integer";
          case PrimitiveTypeEnum::kRealType:
              return "real";
          case PrimitiveTypeEnum::kBoolType:
              return "boolean";
          case PrimitiveTypeEnum::kStringType:
              return "string";
          case PrimitiveTypeEnum::kErrorType:
              return "error";
      }
    }

};

#endif
