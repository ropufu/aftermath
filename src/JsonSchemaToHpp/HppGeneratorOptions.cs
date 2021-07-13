using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Ropufu.JsonSchemaToHpp
{
    [Flags]
    public enum HppGeneratorOptions
    {
        None = 0,
        [JsonPropertyName("ignore")]
        Ignore, // Property/item should be ignored altogether.
        [JsonPropertyName("skip hash")]
        SkipHash // Hashing is to be skipped.
    } // enum HppGeneratorOptions
} // namespace Ropufu.JsonSchemaToHpp
