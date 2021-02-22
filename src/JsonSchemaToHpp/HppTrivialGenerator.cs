using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public class HppTrivialGenerator : IHppGenerator
    {
        public IEnumerable<CodeBuilder> Make() => new CodeBuilder[0];
    } // class HppTrivialGenerator
} // namespace Ropufu.JsonSchemaToHpp
