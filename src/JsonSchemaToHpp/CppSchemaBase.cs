using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public abstract class CppSchemaBase : ICppSchema
    {
        private readonly HppGeneratorOptions options = HppGeneratorOptions.None;
        private readonly String trivialValueCode;
        private readonly List<CodeLine> validationFormats = new();
        private readonly List<String> permissibleValueCodes = new();
        private readonly List<HppInclude> includes = null;
        private readonly List<ICppSchema> definitions = null;
        private readonly List<String> extensions = null;

        public Boolean HasFlag(HppGeneratorOptions flag) => (this.options & flag) != HppGeneratorOptions.None;

        public String PropertyName { get; private init; }

        public String MethodName => this.PropertyName?.ToSnakeCase();

        public String FieldName => this.PropertyName?.ToSnakeCase().Prepend("m_");

        public virtual Boolean DoesSupportCodeGeneration => false;

        public Boolean IsPropertyInherited { get; private init; }

        public String Typename { get; private init; }

        public String Namespace { get; private init; }

        public String Description { get; private init; }

        public String DefaultValueCode { get; private init; }

        public Boolean IsDefaultValueTrivial { get; private init; }

        public String DefaultConditionFormat { get; private init; }

        protected virtual String TrivialDefaultConditionFormat => null;

        protected virtual String TrivialTypename => null;

        public Boolean DoesRequireValidation => this.DoesRequireValidationOverride ?? this.validationFormats.Count > 0;

        protected virtual Boolean? DoesRequireValidationOverride => null;

        public abstract Boolean DoPassByValue { get; }

        public IList<CodeLine> ValidationFormats => this.validationFormats.AsReadOnly();

        public IList<String> PermissibleValueCodes => this.permissibleValueCodes.AsReadOnly();

        public IList<HppInclude> Includes => this.includes.AsReadOnly();

        public IList<ICppSchema> Definitions => this.definitions.AsReadOnly();

        public IList<String> Extensions => this.extensions.AsReadOnly();

        protected CppSchemaBase(JsonSchema jsonSchema, JsonSchemaValueKind valueKind)
        {
            if (jsonSchema is null) throw new ArgumentNullException(nameof(jsonSchema));
            if (jsonSchema.SchemaKind != valueKind) throw new SchemaException(jsonSchema, nameof(jsonSchema.SchemaKind), "\"{0}\" mismatch.");
            if (jsonSchema.HasFlag(HppGeneratorOptions.Ignore)) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppOptions), "\"{0}\" is not supposed to be processed.");

            this.options = jsonSchema.Options;
            this.PropertyName = jsonSchema.PropertyName?.Trim().Nullify();
            this.Description = jsonSchema.Description?.Trim().Nullify();
            this.IsPropertyInherited = jsonSchema.HppIsInherited;

            this.Typename = (jsonSchema.HppTypename?.Trim().Nullify()) ?? (this.TrivialTypename?.Trim().Nullify());
            this.Namespace = jsonSchema.HppNamespace?.Trim().Nullify();
            if (this.Typename is null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppTypename), "Schema requires \"{0}\".");

            var isTypenameQualified = this.Typename.Contains("::");
            if (isTypenameQualified && this.Namespace is not null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppNamespace), "\"{0}\" not allowed for already qualified type.");

            // ~~ Permissible values validation ~~

            this.permissibleValueCodes = new List<String>();
            if (jsonSchema.PermissibleValues is not null)
            {
                foreach (var x in jsonSchema.PermissibleValues) permissibleValueCodes.Add(this.Parse(x)); 
                if (permissibleValueCodes.Count == 0) throw new SchemaException(jsonSchema, nameof(jsonSchema.PermissibleValues), "\"{0}\" array should contain at least one value.");
            } // if (...)
            
            // ~~ Includes validation ~~

            if (jsonSchema.HppIncludes is null) this.includes = new(0);
            else
            {
                this.includes = new(jsonSchema.HppIncludes.Count);
                foreach (var x in jsonSchema.HppIncludes)
                {
                    var item = x.TrimmedClone();
                    if (item.Header is null) throw new SchemaException(jsonSchema, nameof(jsonSchema.HppIncludes), "Headers in \"{0}\" cannot be empty.");
                    this.includes.Add(item);
                } // foreach (...)
            } // if (...)
            
            // ~~ Definitions ~~

            if (jsonSchema.Definitions is null) this.definitions = new(0);
            else
            {
                this.definitions = new(jsonSchema.Definitions.Count);
                foreach (var x in jsonSchema.Definitions)
                    if (x.Value?.HppTypename is not null)
                        if (!x.Value.HasFlag(HppGeneratorOptions.Ignore))
                            this.definitions.Add(x.Value.ToCppType());
            } // if (...)
            
            // ~~ Extensions ~~
            
            if (jsonSchema.HppExtensions is null) this.extensions = new(0);
            else
            {
                this.extensions = new(jsonSchema.HppExtensions.Count);
                foreach (var x in jsonSchema.HppExtensions) this.extensions.Add(x?.Trim() ?? "");
            } // if (...)

            // ~~ Specialized validation ~~

            this.ValidateJsonSchema(jsonSchema); 

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
                    this.validationFormats.Add(String.Concat("if (", y, ") return \"", validationMessage, "\";"));
                } // foreach (...)

            if (jsonSchema.HppDoHardcodeEnums && permissibleValueCodes.Count > 0)
            {
                var clauses = new List<String>(permissibleValueCodes.Count);
                foreach (var x in permissibleValueCodes) clauses.Add(String.Concat("({0} == ", x, ")"));

                this.validationFormats.Add(String.Concat(
                    "if (!(",
                    String.Join(" || ", clauses),
                    ")) return \"",
                    validationMessage,
                    "\";"));
            } // if (...)

            this.validationFormats.AddRange(this.MoreValidationFormat(jsonSchema, validationMessage));
        } // CppSchemaBase(...)

        protected virtual void ValidateJsonSchema(JsonSchema schema) { }

        protected virtual List<CodeLine> MoreValidationFormat(JsonSchema schema, String validationMessage) => new List<CodeLine>();

        public String Parse(JsonElement value) => this.ParseOverride(value)?.Trim().Nullify();

        /// <exception cref="NotSupportedException">Schema does not support code generation.</exception>
        /// <exception cref="SchemaException">Schema validation failed.</exception>
        public void Invalidate(IList<String> reservedNames, out List<String> warnings)
        {
            if (!this.DoesSupportCodeGeneration) throw new NotSupportedException("Schema does not support code generation.");
            
            warnings = new();
            foreach (var x in this.definitions)
            {
                x.Invalidate(reservedNames, out var moreWarnings);
                warnings.AddRange(moreWarnings);
            } // foreach (...)

             this.InvalidateOverride(reservedNames, out warnings);
        } // Invalidate(...)

        protected abstract String ParseOverride(JsonElement value);

        protected virtual void InvalidateOverride(IList<String> reservedNames, out List<String> warnings)
        {
            if (reservedNames is null) throw new ArgumentNullException(nameof(reservedNames));
            warnings = new();
        } // InvalidateOverride(...)
    } // class CppSchemaBase
} // Ropufu.JsonSchemaToHpp
