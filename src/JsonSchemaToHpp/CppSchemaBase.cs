using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public abstract class CppSchemaBase : ICppSchema
    {
        private readonly String trivialValueCode;
        private readonly List<CodeLine> validationFormat = new List<CodeLine>();

        public String PropertyName { get; private init; }

        public String MethodName => this.PropertyName?.ToSnakeCase();

        public String FieldName => this.PropertyName?.ToSnakeCase().Prepend("m_");

        public Boolean IsPropertyInherited { get; private init; }

        public Boolean IsPropertyRequired { get; private set; }

        public String Typename { get; private init; }

        public String Description { get; private init; }

        public String DefaultValueCode { get; private init; }

        public Boolean IsDefaultValueTrivial { get; private init; }

        public String DefaultConditionFormat { get; private init; }

        protected virtual String TrivialDefaultConditionFormat => null;

        protected virtual String TrivialTypename => null;

        public Boolean DoesRequireValidation => this.DoesRequireValidationOverride || this.validationFormat.Count > 0;

        protected virtual Boolean DoesRequireValidationOverride => false;

        public abstract Boolean DoPassByValue { get; }

        public IEnumerable<CodeLine> ValidationFormats => this.validationFormat.AsReadOnly();

        protected CppSchemaBase(JsonSchema jsonSchema, JsonSchemaValueKind valueKind)
        {
            if (jsonSchema is null) throw new ArgumentNullException(nameof(jsonSchema));
            if (jsonSchema.SchemaKind != valueKind) throw new SchemaException(jsonSchema, nameof(jsonSchema.SchemaKind), "\"{0}\" mismatch.");
            
            this.PropertyName = jsonSchema.PropertyName?.Trim().Nullify();
            this.Description = jsonSchema.Description?.Trim().Nullify();
            this.IsPropertyInherited = jsonSchema.HppIsInherited;

            this.Typename = (jsonSchema.HppTypename?.Trim().Nullify()) ?? (this.TrivialTypename?.Trim().Nullify());
            if (this.Typename is null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppTypename), "Schema requires \"{0}\".");
            this.ValidateJsonSchema(jsonSchema); // Specialized validation.

            // ~~ Default value ~~

            this.trivialValueCode = this.Parse(new JsonElement()) ?? throw new ApplicationException();

            this.DefaultValueCode = (jsonSchema.HppDefaultValueCode?.Trim().Nullify()) ?? this.Parse(jsonSchema.DefaultValue);
            if (this.DefaultValueCode is null) throw new ApplicationException();
            this.IsDefaultValueTrivial = this.DefaultValueCode == this.trivialValueCode;
            this.DefaultConditionFormat = this.IsDefaultValueTrivial ? this.TrivialDefaultConditionFormat : null;

            // ~~ Value validation ~~

            var validationMessage = (jsonSchema.HppValidationMessage?.Trim().Nullify()) ?? "Validation failed.";

            if (jsonSchema.HppInvalidFormats is not null)
                foreach (var x in jsonSchema.HppInvalidFormats)
                {
                    var y = (x?.Trim().Nullify()) ?? throw new SchemaException(jsonSchema, nameof(jsonSchema.HppTypename), "Schema invalid format cannot be empty.");
                    this.validationFormat.Add(String.Concat("if (", y, ") return \"", validationMessage, "\";"));
                } // foreach (...)

            if (jsonSchema.PermissibleValues is not null && jsonSchema.HppDoHardcodeEnums)
            {
                var clauses = new List<String>(jsonSchema.PermissibleValues.Count);
                foreach (var x in jsonSchema.PermissibleValues) clauses.Add(String.Concat("({0} == ", this.Parse(x) ?? throw new ApplicationException(), ")"));
                if (clauses.Count == 0) throw new SchemaException(jsonSchema, nameof(jsonSchema.PermissibleValues), "\"{0}\" array should contain at least one value.");

                this.validationFormat.Add(String.Concat(
                    "if (!(",
                    String.Join(" || ", clauses),
                    ")) return \"",
                    validationMessage,
                    "\";"));
            } // if (...)

            this.validationFormat.AddRange(this.MoreValidationFormat(jsonSchema, validationMessage));
        } // CppSchemaBase(...)

        protected virtual void ValidateJsonSchema(JsonSchema schema) { }

        protected virtual List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage) => new List<CodeLine>();

        public String Parse(JsonElement value) => this.ParseOverride(value)?.Trim().Nullify();

        protected abstract String ParseOverride(JsonElement value);

        public void MarkPropertyRequired() => this.IsPropertyRequired = true;
    } // class CppSchemaBase
} // Ropufu.JsonSchemaToHpp
