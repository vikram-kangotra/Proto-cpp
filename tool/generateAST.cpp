#include <any>
#include <vector>
#include <fstream>
#include <sstream>
#include <string_view>
#include <initializer_list>

void defineVisitor(std::ofstream& outfile, std::string_view baseName, std::initializer_list<std::string> types) {
    outfile << "class " << baseName << "Visitor {\n";
    outfile << "    public:\n";

    for (const auto& type : types) {
        auto typeName = type.substr(0, type.find(":"));
        outfile << "        virtual std::any visit" << typeName << baseName << "(class " << typeName << baseName << "*) = 0;\n";
    }

    outfile << "};\n\n";
}

void defineType(std::ofstream& outfile, std::string_view baseName, std::string_view className, std::string fieldName) {
    outfile << "class " << className << baseName << " : public " << baseName << " {\n";
    outfile << "    public:\n";
    outfile << "        " << className << baseName << "(" << fieldName << ")\n";
    outfile << "        : ";

    std::vector<std::string> fields;
    std::stringstream ss(fieldName);

    while (std::getline(ss, fieldName, ',')) {
        fields.push_back(fieldName);
    }

    for (int i = 0; i < fields.size(); i++) {
        auto pos = fields[i].find_last_of(" ");
        auto type = fields[i].substr(0, pos);
        auto left = fields[i].substr(pos + 1);
        auto right = left;
        if (type.find("&&") != std::string::npos) {
            right = "std::move(" + right + ")";
        }
        outfile << left << "{" << right << "}";
        if (i != fields.size() - 1) {
            outfile << ", ";
        }
    }
    outfile << " {}\n\n";

    outfile << "        std::any accept(" << baseName << "Visitor* visitor) override {\n";
    outfile << "            return visitor->visit" << className << baseName << "(this);\n";
    outfile << "        }\n";

    for (const auto& field : fields) {
        auto pos = field.find_last_of(" ");
        auto type = field.substr(0, pos);
        auto variable = field.substr(pos + 1);
        if (type.find("std::unique_ptr<Expr>&&") != std::string::npos) {
            type = "std::unique_ptr<Expr>";
        }
        variable[0] = toupper(variable[0]);
        outfile << "        " << type << "& get" << variable << "() { return ";
        variable[0] = tolower(variable[0]);
        outfile << variable << "; }\n";
    }

    outfile << "    private:\n";
    for (const auto& field : fields) {
        auto pos = field.find_last_of(" ");
        auto type = field.substr(0, pos);
        if (type.find("std::unique_ptr<Expr>&&") != std::string::npos) {
            type = "std::unique_ptr<Expr>";
        }
        auto variable = field.substr(pos + 1);
        outfile << "        " << type << " " << variable << ";\n";
    }

    outfile << "};\n\n";
}

void defineAST(std::ofstream& outfile, std::string_view baseName, std::initializer_list<std::string> fieldList) {
    defineVisitor(outfile, baseName, fieldList);

    outfile << "class " << baseName << " {\n";
    outfile << "    public:\n";
    outfile << "        virtual std::any accept(" << baseName << "Visitor*) = 0;\n";
    outfile << "};\n\n";

    for (auto& field : fieldList) {
        auto typeName = field.substr(0, field.find(":"));
        auto fieldName = field.substr(field.find(":") + 2);
        defineType(outfile, baseName, typeName, fieldName);
    }
}

int main() {

    std::ofstream outfile("../../src/expr.h");

    outfile << "#pragma once\n\n";
    outfile << "#include <memory>\n";
    outfile << "#include <any>\n";
    outfile << "#include \"token.h\"\n\n";

    defineAST(outfile, "Expr", 
        {
            "Literal: std::any value",
            "Unary: Token op, std::unique_ptr<Expr>&& right",
            "Binary: std::unique_ptr<Expr>&& left, Token op, std::unique_ptr<Expr>&& right"
        });

    defineAST(outfile, "Stmt", 
        {
            "Expression: std::unique_ptr<Expr>&& expression",
            "Print: std::unique_ptr<Expr>&& expression"
        });

    return 0;
}
