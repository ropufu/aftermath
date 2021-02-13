using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppObjectSchema : CppSchemaBase
    {
        private Boolean doesRequireValidationOverride = false;
        private List<HppTemplate> templates = null;
        private List<HppInclude> includes = null;
        private List<String> inherits = null;
        private List<ICppSchema> properties = null;
        private List<CppObjectSchema> definitions = null;

        public IList<HppTemplate> Templates => this.templates.AsReadOnly();
        public IList<HppInclude> Includes => this.includes.AsReadOnly();
        public IList<String> Inherits => this.inherits.AsReadOnly();
        public IList<ICppSchema> Properties => this.properties.AsReadOnly();
        public IList<CppObjectSchema> Definitions => this.definitions.AsReadOnly();

        protected override Boolean DoesRequireValidationOverride => this.doesRequireValidationOverride;
        
        public String Namespace { get; private set; }

        public Int32 CountRequiredProperties { get; private set; } = 0;

        public Int32 CountOptionalProperties { get; private set; } = 0;

        public override Boolean DoPassByValue => false;

        public CppObjectSchema(JsonSchema jsonSchema)
            : base(jsonSchema, JsonSchemaValueKind.Object)
        {
            this.Namespace = jsonSchema.HppNamespace?.Trim().Nullify();
            
            // ~~ Templates ~~
            if (jsonSchema.HppTemplates is null) this.templates = new(0);
            else
            {
                this.templates = new(jsonSchema.HppTemplates.Count);
                foreach (var x in jsonSchema.HppTemplates) this.templates.Add(x.TrimmedClone());
            } // if (...)
            
            // ~~ Includes ~~
            if (jsonSchema.HppIncludes is null) this.includes = new(0);
            else
            {
                this.includes = new(jsonSchema.HppIncludes.Count);
                foreach (var x in jsonSchema.HppIncludes) this.includes.Add(x.TrimmedClone());
            } // if (...)
            
            // ~~ Inherits ~~
            if (jsonSchema.HppInherits is null) this.inherits = new(0);
            else
            {
                this.inherits = new(jsonSchema.HppInherits.Count);
                foreach (var x in jsonSchema.HppInherits) this.inherits.Add(x?.Trim().Nullify());
            } // if (...)
            
            // ~~ Properties ~~
            if (jsonSchema.Properties is null) this.properties = new(0);
            else
            {
                this.properties = new(jsonSchema.Properties.Count);
                this.doesRequireValidationOverride = false;

                var requiredPropertyNames = jsonSchema.RequiredPropertyNames ?? new(0);
                foreach (var x in jsonSchema.Properties)
                {
                    var item = x.Value?.ToCppType() ?? throw new SchemaException(jsonSchema, nameof(jsonSchema.Properties), $"Property \"{x.Key}\" cannot be null.");
                    if (item.PropertyName is null) throw new SchemaException(jsonSchema, nameof(jsonSchema.Properties), "Property key cannot be empty.");

                    if (requiredPropertyNames.Contains(x.Key)) item.MarkPropertyRequired();

                    // Make sure properties come required-first, optional-second.
                    if (item.IsPropertyRequired)
                    {
                        properties.Insert(0, item);
                        ++this.CountRequiredProperties;
                    } // if (...)
                    else
                    {
                        properties.Add(item);
                        ++this.CountOptionalProperties;
                    } // if (...)

                    this.doesRequireValidationOverride |= item.DoesRequireValidation;
                } // foreach (...)
            } // if (...)
            
            // ~~ Definitions ~~
            if (jsonSchema.Definitions is null) this.definitions = new(0);
            else
            {
                this.definitions = new(jsonSchema.Definitions.Count);
                foreach (var x in jsonSchema.Definitions)
                    if (x.Value?.HppTypename is not null)
                        this.definitions.Add(new(x.Value));
            } // if (...)
        } // CppObject(...)

        protected override String ParseOverride(JsonElement value) =>
            value.ValueKind switch
            {
                JsonValueKind.Undefined => "{}",
                JsonValueKind.Object => throw new NotSupportedException(),
                _ => throw new FormatException("JSON type mismatch.")
            };
            
        /// <summary>
        /// Validates JSON schema for code generation.
        /// </summary>
        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="SchemaException">JSON schema did not pass validation.</exception>
        public void Invalidate(out List<String> warnings)
        {
            if (this.Namespace is null) throw new SchemaException("Typename cannot be empty.");

            warnings = new();
            this.InvalidateTemplates(warnings);
            this.InvalidateIncludes(warnings);
            this.InvalidateInherits(warnings);
            this.InvalidateProperties(warnings);
            this.InvalidateDefinitions(warnings);
        } // Invalidate(...)

        private void InvalidateTemplates(List<String> warnings)
        {
            foreach (var x in this.templates)
            {
                if (x.Key is null) throw new SchemaException("Template keys cannot be empty.");
                if (x.Name is null) throw new SchemaException("Template names cannot be empty.");
            } // foreach (...)
        } // InvalidateTemplates(...)

        private void InvalidateIncludes(List<String> warnings)
        {
            foreach (var x in this.includes)
                if (x.Header is null) throw new SchemaException("Include headers cannot be empty.");
        } // InvalidateIncludes(...)

        private void InvalidateInherits(List<String> warnings)
        {
            foreach (var x in this.inherits)
                if (x is null) throw new SchemaException("Inherits cannot be empty.");
        } // InvalidateInherits(...)

        private void InvalidateProperties(List<String> warnings)
        {
            if (this.properties.Count == 0) throw new SchemaException("JSON schema must have at least one property.");
            foreach (var x in this.properties)
            {
                if (x.IsPropertyInherited)
                {
                    if (!x.IsDefaultValueTrivial) throw new SchemaException($"Inherited property \"{x.PropertyName}\" cannot have non-trivial default value.");
                    if (x.DoesRequireValidation) warnings.Add($"Make sure that custom validation is called from the base class for inherited property \"{x.PropertyName}\".");
                } // if (...)
            } // foreach (...)
        } // InvalidateProperties(...)

        private void InvalidateDefinitions(List<String> warnings)
        {
            foreach (var x in this.definitions)
            {
                x.Invalidate(out var moreWarnings);
                warnings.AddRange(moreWarnings);
            } // foreach (...)
        } // InvalidateDefinitions(...)
    } // class CppObject
} // namespace Ropufu.JsonSchemaToHpp
