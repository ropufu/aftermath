using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public partial class JsonSchema
    {
        [JsonPropertyName("@ropufu-namespace")]
        public String HppNamespace { get; set; }
        [JsonPropertyName("@ropufu-typename")]
        public String HppTypename { get; set; }
        [JsonPropertyName("@ropufu-target-path")]
        public String HppTargetPath { get; set; }
        [JsonPropertyName("@ropufu-templates")]
        public List<HppTemplate> HppTemplates { get; set; }
        [JsonPropertyName("@ropufu-inherits")]
        public List<HppInherit> HppInherits { get; set; }
        [JsonPropertyName("@ropufu-includes")]
        public List<HppInclude> HppIncludes { get; set; }
        [JsonPropertyName("@ropufu-default-value")]
        public String HppDefaultValueCode { get; set; }
        [JsonPropertyName("@ropufu-validation-message")]
        public String HppValidationMessage { get; set; }
        [JsonPropertyName("@ropufu-invalid-formats")]
        public List<String> HppInvalidFormats { get; set; }
        [JsonPropertyName("@ropufu-is-inherited")]
        public Boolean HppIsInherited { get; set; } = false;
        [JsonPropertyName("@ropufu-hard-enums")]
        public Boolean HppDoHardcodeEnums { get; set; } = false;
    } // class JsonSchema
} // namespace Ropufu.JsonSchemaToHpp
