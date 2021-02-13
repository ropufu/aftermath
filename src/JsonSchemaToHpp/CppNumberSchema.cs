using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppNumberSchema : CppSchemaBase
    {
        public override Boolean DoPassByValue => true;

        public CppNumberSchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.Number)
        {
        } // CppNumber(...)

        protected override List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage)
        {
            var result = base.MoreValidationFormat(schema, validationMessage);

            if (schema.Minimum is not null)
                result.Add(String.Concat("if ({0} < ", schema.Minimum.Value.ToString(), ") return \"", validationMessage, "\";"));

            if (schema.Maximum is not null)
                result.Add(String.Concat("if ({0} > ", schema.Maximum.Value.ToString(), ") return \"", validationMessage, "\";"));

            return result;
        } // GetValidationClauses(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "0",
                JsonValueKind.Number => Double.Parse(value.GetRawText()).ToString(),
                _ => throw new FormatException("JSON type mismatch.")
            };
    } // class CppNumber
} // namespace Ropufu.JsonSchemaToHpp
