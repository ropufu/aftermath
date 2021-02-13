using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public partial class JsonSchema
    {
        [JsonPropertyName("type")]
        public JsonSchemaValueKind? SchemaKind { get; set; }
        [JsonPropertyName("additionalProperties")]
        public Boolean AllowAdditionalProperties { get; set; } = true;
        [JsonPropertyName("title")]
        public String Title { get; set; }
        [JsonPropertyName("description")]
        public String Description { get; set; }
        [JsonPropertyName("enum")]
        public List<JsonElement> PermissibleValues { get; set; }
        [JsonPropertyName("required")]
        public List<String> RequiredPropertyNames { get; set; }
        [JsonPropertyName("minLength")]
        public Int32? MinLength { get; set; }
        [JsonPropertyName("maxLength")]
        public Int32? MaxLength { get; set; }
        [JsonPropertyName("minItems")]
        public Int32? MinItems { get; set; }
        [JsonPropertyName("maxItems")]
        public Int32? MaxItems { get; set; }
        [JsonPropertyName("minimum")]
        public Double? Minimum { get; set; }
        [JsonPropertyName("maximum")]
        public Double? Maximum { get; set; }
        [JsonPropertyName("$ref")]
        public String ReferencedSchema { get; set; }
        [JsonPropertyName("default")]
        public JsonElement DefaultValue { get; set; }
        [JsonPropertyName("items")]
        public JsonSchema ItemSchema { get; set; }
        [JsonPropertyName("properties")]
        public Dictionary<String, JsonSchema> Properties { get; set; }
        [JsonPropertyName("definitions")]
        public Dictionary<String, JsonSchema> Definitions { get; set; }
    } // class JsonSchema
} // namespace Ropufu.JsonSchemaToHpp
