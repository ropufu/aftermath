using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public partial class JsonSchema
    {
        private String schemaFileName = null;

        [JsonIgnore]
        public String SchemaFileName
        {
            get => this.schemaFileName;
            set
            {
                this.schemaFileName = value;
                this.Populate();
            } // set
        } // SchemaFileName

        /// <summary>
        /// If this schema describes a property in another schema, name of the property.
        /// If this schema is a member of a definitions block in another schema, name of the definition.
        /// </summary>
        [JsonIgnore]
        public String PropertyName { get; private set; }

        public JsonSchema()
        {
        } // JsonSchema(...)

        public CppSchemaBase ToCppType() =>
            this.SchemaKind switch
            {
                JsonSchemaValueKind.Null => new CppNullSchema(this),
                JsonSchemaValueKind.Boolean => new CppBooleanSchema(this),
                JsonSchemaValueKind.Integer => new CppIntegerSchema(this),
                JsonSchemaValueKind.Number => new CppNumberSchema(this),
                JsonSchemaValueKind.String => new CppStringSchema(this),
                JsonSchemaValueKind.Array => new CppArraySchema(this),
                JsonSchemaValueKind.Object => new CppObjectSchema(this),
                _ => throw new NotImplementedException("JSON schema value kind not recognized.")
            };

        private void Populate()
        {
            if (this.ItemSchema is not null)
            {
                this.ItemSchema.SchemaFileName = this.SchemaFileName;
                this.ItemSchema.Populate();
            } // if (...)

            if (this.Definitions is not null)
                foreach (var x in this.Definitions)
                {
                    x.Value.SchemaFileName = this.SchemaFileName;
                    x.Value.PropertyName = x.Key;
                    x.Value.Populate();
                } // foreach (...)

            if (this.Properties is not null)
                foreach (var x in this.Properties)
                {
                    x.Value.SchemaFileName = this.SchemaFileName;
                    x.Value.PropertyName = x.Key;
                    x.Value.Populate();
                } // foreach (...)
        } // Populate(...)

        /// <summary>
        /// 
        /// </summary>
        /// <param name="text"></param>
        /// <returns></returns>
        /// <exception cref="System.ArgumentNullException"></exception>
        /// <exception cref="System.Text.Json.JsonException"></exception>
        public static JsonSchema Parse(String text)
        {
            if (text is null) throw new ArgumentNullException(nameof(text));

            var options = new JsonSerializerOptions
            {
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
                Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase, false) }
            };

            var schema = JsonSerializer.Deserialize<JsonSchema>(text, options);
            schema.Populate();

            return schema;
        } // Parse(...)
    } // class JsonSchema
} // namespace Ropufu.JsonSchemaToHpp
