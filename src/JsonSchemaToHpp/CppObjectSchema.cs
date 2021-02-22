using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CppObjectSchema : CppSchemaBase
    {
        private Boolean? doesRequireValidationOverride = null;
        private List<HppTemplate> templates = null;
        private List<HppInherit> inherits = null;
        private List<ICppSchema> properties = null;
        private ICppSchema[] requiredProperties = null;

        public IList<HppTemplate> Templates => this.templates.AsReadOnly();
        public IList<HppInherit> Inherits => this.inherits.AsReadOnly();
        public IList<ICppSchema> Properties => this.properties.AsReadOnly();
        public IList<ICppSchema> RequiredProperties => Array.AsReadOnly(this.requiredProperties);

        public override Boolean DoesSupportCodeGeneration => true;

        protected override Boolean? DoesRequireValidationOverride => this.doesRequireValidationOverride;

        public Int32 CountRequiredProperties { get; private init; } = 0;

        public Int32 CountOptionalProperties { get; private init; } = 0;

        public override Boolean DoPassByValue => false;

        public CppObjectSchema(JsonSchema jsonSchema)
            : base(jsonSchema, JsonSchemaValueKind.Object)
        {
            // ~~ Templates ~~
            if (jsonSchema.HppTemplates is null) this.templates = new(0);
            else
            {
                this.templates = new(jsonSchema.HppTemplates.Count);
                foreach (var x in jsonSchema.HppTemplates) this.templates.Add(x.TrimmedClone());
            } // if (...)
            
            // ~~ Inherits ~~
            if (jsonSchema.HppInherits is null) this.inherits = new(0);
            else
            {
                this.inherits = new(jsonSchema.HppInherits.Count);
                foreach (var x in jsonSchema.HppInherits)
                {
                    var y = x.TrimmedClone();
                    if (y.ErrorMessageCall is not null) this.doesRequireValidationOverride = true;
                    this.inherits.Add(y);
                } // foreach (...)
            } // if (...)
            
            // ~~ Properties ~~
            this.requiredProperties = new ICppSchema[jsonSchema.RequiredPropertyNames?.Count ?? 0];
            if (jsonSchema.Properties is null) this.properties = new(0);
            else
            {
                this.properties = new(jsonSchema.Properties.Count);
                foreach (var x in jsonSchema.Properties)
                {
                    var item = x.Value?.ToCppType() ?? throw new SchemaException(jsonSchema, nameof(jsonSchema.Properties), $"Property \"{x.Key}\" cannot be null.");
                    if (item.PropertyName is null) throw new SchemaException(jsonSchema, nameof(jsonSchema.Properties), "Property key cannot be empty.");

                    var isRequired = false;
                    for (var i = 0; i < this.requiredProperties.Length; ++i)
                    {
                        if (jsonSchema.RequiredPropertyNames[i] == x.Key)
                        {
                            this.requiredProperties[i] = item;
                            isRequired = true;
                        } // if (...)
                    } // for (...)

                    // Make sure properties come required-first, optional-second.
                    if (isRequired)
                    {
                        this.properties.Insert(this.CountRequiredProperties, item);
                        ++this.CountRequiredProperties;
                    } // if (...)
                    else
                    {
                        this.properties.Add(item);
                        ++this.CountOptionalProperties;
                    } // if (...)

                    if (!item.IsPropertyInherited && item.DoesRequireValidation)
                        this.doesRequireValidationOverride = true;
                } // foreach (...)
            } // if (...)

            for (var i = 0; i < this.requiredProperties.Length; ++i)
                if (this.requiredProperties[i] is null)
                    throw new SchemaException(jsonSchema, nameof(jsonSchema.RequiredPropertyNames), $"Required property \"{jsonSchema.RequiredPropertyNames[i]}\" has not been defined.");
        } // CppObjectSchema(...)

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
        protected override void InvalidateOverride(IList<String> reservedNames, out List<String> warnings)
        {
            base.InvalidateOverride(reservedNames, out warnings);
            if (this.Namespace is null) throw new SchemaException("Namespace cannot be empty.");

            this.InvalidateTemplates(reservedNames, warnings);
            this.InvalidateInherits(warnings);
            this.InvalidateProperties(reservedNames, warnings);
        } // Invalidate(...)

        private void InvalidateTemplates(IList<String> reservedNames, List<String> warnings)
        {
            var aliases = new HashSet<String>(this.templates.Count);
            foreach (var x in this.templates)
            {
                if (x.Key is null) throw new SchemaException("Template keys cannot be empty.");
                if (x.Name is null) throw new SchemaException("Template names cannot be empty.");
                if (x.Alias is not null)
                {
                    if (reservedNames.Contains(x.Alias)) throw new SchemaException($"Template alias cannot be \"{x.Alias}\".");
                    if (!aliases.Add(x.Alias)) throw new SchemaException("Template aliases must be distinct.");
                } // if (...)
            } // foreach (...)
        } // InvalidateTemplates(...)

        private void InvalidateInherits(List<String> warnings)
        {
            foreach (var x in this.inherits)
            {
                if (x.InheritanceType is null) throw new SchemaException("Inheritance mode cannot be empty.");
                if (x.Name is null) throw new SchemaException("Inherited typename cannot be empty.");
            } // foreach (...)
        } // InvalidateInherits(...)

        private void InvalidateProperties(IList<String> reservedNames, List<String> warnings)
        {
            if (this.properties.Count == 0) throw new SchemaException("JSON schema must have at least one property.");
            var namesTaken = new HashSet<String>(this.templates.Count + reservedNames.Count);
            namesTaken.UnionWith(reservedNames);
            foreach (var x in this.templates) if (x.Alias is not null) namesTaken.Add(x.Alias);
            foreach (var x in this.properties)
            {
                if (namesTaken.Contains(x.MethodName)) throw new SchemaException($"Property \"{x.PropertyName}\" cannot have the same name as a typedef.");
                if (x.IsPropertyInherited)
                {
                    if (x.DoesRequireValidation) warnings.Add($"Make sure that validation is properly set up in the base class for inherited property \"{x.PropertyName}\".");
                } // if (...)
            } // foreach (...)
        } // InvalidateProperties(...)
    } // class CppObject
} // namespace Ropufu.JsonSchemaToHpp
