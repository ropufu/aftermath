using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppIntegerSchema : CppSchemaBase
    {
        public override Boolean DoPassByValue => true;

        public CppIntegerSchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.Integer)
        {
        } // CppInteger(...)

        protected override List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage)
        {
            var result = base.MoreValidationFormat(schema, validationMessage);

            if (schema.Minimum is not null)
            {
                if (!Double.IsFinite(schema.Minimum.Value)) throw new FormatException();
                var minimum = checked((Int64)Math.Ceiling(schema.Minimum.Value));
                result.Add(String.Concat("if ({0} < ", minimum.ToString(), ") return \"", validationMessage, "\";"));
            } // if (...)

            if (schema.Maximum is not null)
            {
                if (!Double.IsFinite(schema.Maximum.Value)) throw new FormatException();
                var maximum = checked((Int64)Math.Floor(schema.Minimum.Value));
                result.Add(String.Concat("if ({0} > ", maximum.ToString(), ") return \"", validationMessage, "\";"));
            } // if (...)

            return result;
        } // GetValidationClauses(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "0",
                JsonValueKind.Number => Int64.Parse(value.GetRawText()).ToString(),
                _ => throw new FormatException("JSON type mismatch."),
            };
    } // class CppInteger
} // namespace Ropufu.JsonSchemaToHpp
