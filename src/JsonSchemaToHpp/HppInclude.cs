using System;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed record HppInclude : IComparable<HppInclude>
    {
        [JsonPropertyName("header")]
        public String Header { get; init; }
        [JsonPropertyName("comment")]
        public String Comment { get; init; }

        public HppInclude TrimmedClone() => new()
        {
            Header = this.Header?.Trim().Nullify(),
            Comment = this.Comment?.Trim().Nullify()
        };

        public Int32 CompareTo(HppInclude other)
        {
            if (other is null) return 1;
            return String.CompareOrdinal(this.Header, other.Header);
        } // CompareTo(...)

        public Boolean Equals(HppInclude other)
        {
            if (other is null) return false;
            return String.Equals(this.Header, other.Header);
        } // Equals(...)

        public override Int32 GetHashCode() => (this.Header ?? "").GetHashCode();
    } // record HppInclude
} // namespace Ropufu.JsonSchemaToHpp
