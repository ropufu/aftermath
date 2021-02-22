using System;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed record HppInherit
    {
        [JsonPropertyName("mode")]
        public String InheritanceType { get; init; }
        [JsonPropertyName("name")]
        public String Name { get; init; }
        [JsonPropertyName("constructor arguments")]
        public String ConstructorArgumentsFormat { get; init; }
        [JsonPropertyName("error message call")]
        public String ErrorMessageCall { get; init; }

        public HppInherit TrimmedClone() => new()
        {
            InheritanceType = this.InheritanceType?.Trim().Nullify(),
            Name = this.Name?.Trim().Nullify(),
            ConstructorArgumentsFormat = this.ConstructorArgumentsFormat?.Trim().Nullify(),
            ErrorMessageCall = this.ErrorMessageCall?.Trim().Nullify()
        };
    } // record HppInherit
} // namespace Ropufu.JsonSchemaToHpp
