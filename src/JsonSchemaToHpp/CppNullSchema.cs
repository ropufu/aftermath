using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppNullSchema : CppSchemaBase
    {
        protected override String TrivialTypename => "std::nullptr_t";

        public override Boolean DoPassByValue => true;

        public CppNullSchema(JsonSchema schema)
            : base(schema, JsonSchemaValueKind.Null)
        {
        } // CppNull(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "nullptr",
                JsonValueKind.Null => "nullptr",
                _ => throw new FormatException("JSON type mismatch.")
            };
    } // class CppNull
} // namespace Ropufu.JsonSchemaToHpp
