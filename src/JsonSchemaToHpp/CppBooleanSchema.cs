using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppBooleanSchema : CppSchemaBase
    {
        protected override String TrivialTypename => "bool";

        public override Boolean DoPassByValue => true;

        public CppBooleanSchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.Boolean)
        {
        } // CppBoolean(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "false",
                JsonValueKind.False => "false",
                JsonValueKind.True => "true",
                _ => throw new FormatException("JSON type mismatch.")
            };
    } // class CppBoolean
} // namespace Ropufu.JsonSchemaToHpp
