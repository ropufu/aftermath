using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppStringSchema : CppSchemaBase
    {
        private Boolean doPassByValue = false;
        private Boolean doesSupportCodeGeneration = false;
        private Boolean? doesRequireValidationOverride = null;
        private List<String> unquotedPermissibleValues = null;

        public override Boolean DoesSupportCodeGeneration => this.doesSupportCodeGeneration;

        protected override Boolean? DoesRequireValidationOverride => this.doesRequireValidationOverride;

        protected override String TrivialDefaultConditionFormat => "{0}.empty()";

        protected override String TrivialTypename => "std::string";

        public override Boolean DoPassByValue => this.doPassByValue;

        public CppStringSchema(JsonSchema jsonSchema)
            : base(jsonSchema, JsonSchemaValueKind.String)
        {
            if (jsonSchema.HppIncludes is not null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppIncludes), "String objects do not support \"{0}\".");
            if (jsonSchema.HppInherits is not null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppInherits), "String objects do not support \"{0}\".");
            if (jsonSchema.HppDoHardcodeEnums)
            {
                if (base.PermissibleValueCodes.Count == 0) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppDoHardcodeEnums), "Permissible values cannot be emtpty when \"{0}\" is true.");
                this.doPassByValue = true;
                this.doesSupportCodeGeneration = true;
                this.doesRequireValidationOverride = false;

                this.unquotedPermissibleValues = new List<String>(base.PermissibleValueCodes.Count);
                foreach (var x in base.PermissibleValueCodes)
                {
                    if (x is null) throw new SchemaException("Permissible values cannot be null.");
                    if (x.Length < 2) throw new SchemaException("Permissible values cannot be empty.");
                    var item = x.Substring(1, x.Length - 2).Trim().Nullify();
                    if (item is null) throw new SchemaException("Permissible values cannot be whitespace.");
                    this.unquotedPermissibleValues.Add(item);
                } // foreach (...)
            } // if (...)
        } // CppStringSchema(...)

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
                JsonValueKind.Null => "\"\"",
                JsonValueKind.String => value.GetRawText(),
                _ => throw new FormatException("JSON type mismatch.")
            };
            
        /// <summary>
        /// Validates JSON schema for code generation.
        /// </summary>
        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="SchemaException">JSON schema did not pass validation.</exception>
        protected override void InvalidateOverride(IList<String> reservedNames, out List<String> warnings)
        {
            base.InvalidateOverride(reservedNames, out warnings);
            if (this.Namespace is null) throw new SchemaException("Namespace cannot be empty.");

            foreach (var x in this.unquotedPermissibleValues)
            {
                if (reservedNames.Contains(x.ToSnakeCase()))
                    throw new SchemaException($"Permissible value \"{x}\" cannot have the same name as a typedef.");
            } // foreach (...)
        } // Invalidate(...)
    } // class CppString
} // namespace Ropufu.JsonSchemaToHpp
