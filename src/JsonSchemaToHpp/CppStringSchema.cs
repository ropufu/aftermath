using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppStringSchema : CppSchemaBase
    {
        protected override String TrivialDefaultConditionFormat => "{0}.empty()";

        protected override String TrivialTypename => "std::string";

        public override Boolean DoPassByValue => false;

        public CppStringSchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.String)
        {
        } // CppString(...)

        protected override List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage)
        {
            var result = base.MoreValidationFormat(schema, validationMessage);

            if (schema.MinLength is not null)
                result.Add(String.Concat("if ({0}.size() < ", schema.MinLength.Value.ToString(), ") return \"", validationMessage, "\";"));

            if (schema.MaxLength is not null)
                result.Add(String.Concat("if ({0}.size() > ", schema.MaxLength.Value.ToString(), ") return \"", validationMessage, "\";"));

            return result;
        } // GetValidationClauses(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "\"\"",
                JsonValueKind.String => value.GetRawText(),
                _ => throw new FormatException("JSON type mismatch.")
            };
    } // class CppString
} // namespace Ropufu.JsonSchemaToHpp
