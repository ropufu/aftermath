using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppArraySchema : CppSchemaBase
    {
        private JsonSchemaValueKind itemsKind;

        protected override String TrivialDefaultConditionFormat => "{0}.empty()";

        public override Boolean DoPassByValue => false;

        public CppArraySchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.Array)
        {
        } // CppArray(...)

        protected override void ValidateJsonSchema(JsonSchema schema)
        {
            if (schema.ItemSchema is null) throw new SchemaException(schema, nameof(schema.ItemSchema), "Array schema requires \"{0}\".");
            if (schema.ItemSchema.SchemaKind is null) throw new SchemaException(schema, nameof(schema.ItemSchema.SchemaKind), "Array items schema requires \"{0}\".");
            this.itemsKind = schema.ItemSchema.SchemaKind.Value;
        } // ValidateJsonSchema(...)

        protected override List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage)
        {
            var result = base.MoreValidationFormat(schema, validationMessage);

            if (schema.MinItems is not null)
                result.Add(String.Concat("if ({0}.size() < ", schema.MinItems.Value.ToString(), ") return \"", validationMessage, "\";"));

            if (schema.MaxItems is not null)
                result.Add(String.Concat("if ({0}.size() > ", schema.MaxItems.Value.ToString(), ") return \"", validationMessage, "\";"));

            if (!schema.ItemSchema.HasFlag(HppGeneratorOptions.Ignore))
            {
                var itemsSchema = schema.ItemSchema.ToCppType();
                if (itemsSchema.DoesRequireValidation)
                {
                    result.Add("for (const auto& x : {0})");
                    result.Add("{{");
                    foreach (var x in itemsSchema.ValidationFormats)
                        result.Add(new(String.Format(x.Code, "x"), x.TabOffset + 1));
                    result.Add("}} // for (...)");
                } // if (...)
            } // if (...)

            return result;
        } // GetValidationClauses(...)

        protected override String ParseOverride(JsonElement value)
        {
            switch (value.ValueKind)
            {
                case JsonValueKind.Undefined: return "{}";
                case JsonValueKind.Array:
                    // Ensure consistency.
                    foreach (var x in value.EnumerateArray())
                        if (!x.ValueKind.DoesAgree(this.itemsKind))
                            throw new FormatException("JSON type mismatch.");
                    Func<JsonElement, String> parser = this.itemsKind switch
                    {
                        JsonSchemaValueKind.Null => (x) => "nullptr",
                        JsonSchemaValueKind.Boolean => (x) => Boolean.Parse(x.GetRawText()).ToString().ToLowerInvariant(),
                        JsonSchemaValueKind.Integer => (x) => Int64.Parse(x.GetRawText()).ToString(),
                        JsonSchemaValueKind.Number => (x) => Double.Parse(x.GetRawText()).ToString(),
                        JsonSchemaValueKind.String => (x) => x.GetRawText(),
                        _ => throw new NotSupportedException("Primitive-typed items expected."),
                    };
                    // Parse values.
                    var items = new List<String>(value.GetArrayLength());
                    foreach (var x in value.EnumerateArray()) items.Add(parser(x));
                    return String.Concat("{", String.Join(", ", items), "}");
                default: throw new FormatException("JSON type mismatch.");
            } // switch (...)
        } // Parse(...)
    } // class CppArray
} // namespace Ropufu.JsonSchemaToHpp
