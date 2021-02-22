using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public static class HppGeneratorFactory
    {
        public static IHppGenerator MakeGenerator(ICppSchema cppSchema, out List<String> warnings)
        {
            if (cppSchema is null) throw new ArgumentNullException(nameof(cppSchema));
            warnings = new();

            if (!cppSchema.DoesSupportCodeGeneration) return new HppTrivialGenerator();

            switch (cppSchema)
            {
                case CppObjectSchema o: return new HppStructGenerator(o, out warnings);
                case CppStringSchema s: return new HppEnumGenerator(s, out warnings);
                default: throw new NotImplementedException("C++ header generator for this JSON type not implemented.");
            } // switch (...)
        } // MakeGenerator(...)
    } // class HppGeneratorFactory
} // namespace Ropufu.JsonSchemaToHpp
