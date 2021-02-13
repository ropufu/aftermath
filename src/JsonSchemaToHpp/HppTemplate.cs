using System;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed record HppTemplate
    {
        [JsonPropertyName("key")]
        public String Key { get; init; }
        [JsonPropertyName("name")]
        public String Name { get; init; }
        [JsonPropertyName("alias")]
        public String Alias { get; init; }

        public HppTemplate TrimmedClone() => new()
        {
            Key = this.Key?.Trim().Nullify(),
            Name = this.Name?.Trim().Nullify(),
            Alias = this.Alias?.Trim().Nullify()
        };
    } // record HppTemplate
} // namespace Ropufu.JsonSchemaToHpp
