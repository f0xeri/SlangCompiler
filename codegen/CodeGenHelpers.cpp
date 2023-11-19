//
// Created by user on 19.11.2023.
//

#include "CodeGenContext.hpp"

namespace Slangc {
    Type* typeOf(const std::string& type, CodeGenContext& context) {
        if (type == "integer")
            return Type::getInt32Ty(*context.llvmContext);
        if (type == "real")
            return Type::getDoubleTy(*context.llvmContext);
        if (type == "float")
            return Type::getFloatTy(*context.llvmContext);
        if (type == "boolean")
            return Type::getInt1Ty(*context.llvmContext);
        if (type == "character")
            return Type::getInt8Ty(*context.llvmContext);
        if (type == "void" || type == "")
            return Type::getVoidTy(*context.llvmContext);
        if (context.allocatedClasses.contains(type))
            return context.allocatedClasses[type];
        return nullptr;
    }

    Type* typeOf(const ExprPtrVariant& expr, CodeGenContext& context) {
        if (auto type = std::get_if<TypeExprPtr>(&expr)) {
            return typeOf(*type, context);
        }
        if (auto type = std::get_if<ArrayExprPtr>(&expr)) {

        }
        if (auto type = std::get_if<FuncPointerStmtPtr>(&expr)) {

        }
        return nullptr;
    }
}
