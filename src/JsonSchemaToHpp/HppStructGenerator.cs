using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public class HppStructGenerator : HppGenerator<CppObjectSchema>
    {
        private static readonly HppInclude[] StandardIncludes = new HppInclude[]
        {
            new() { Header = "<nlohmann/json.hpp>" },
            new() { Header = "<ropufu/noexcept_json.hpp>" },
            new() { Header = "<cstddef>", Comment = "std::size_t" },
            new() { Header = "<functional>", Comment = "std::hash" },
            new() { Header = "<stdexcept>", Comment = "std::runtime_error" },
            new() { Header = "<string_view>", Comment = "std::string_view" },
            new() { Header = "<string>", Comment = "std::string" }
        };

        private static readonly HppInclude[] ValidationIncludes = new HppInclude[]
        {
            new() { Header = "<concepts>", Comment = "std::same_as" },
            new() { Header = "<optional>", Comment = "std::optional, std::nullopt" }
        };

        protected override IList<String> ReservedNames => new String[] {"type"};

        protected override IEnumerable<HppInclude> MoreIncludes
        {
            get
            {
                foreach (var x in HppStructGenerator.StandardIncludes) yield return x;
                if (this.ValidatedSchema.DoesRequireValidation)
                    foreach (var x in HppStructGenerator.ValidationIncludes)
                        yield return x;
            } // get
        } // MoreIncludes

        private readonly String templateSignature = null;
        private readonly String templatedStructName = String.Empty;
        private readonly String qualifiedStructName = String.Empty;

        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="NotSupportedException">Schema does not support code generation.</exception>
        /// <exception cref="SchemaException">Schema validation failed.</exception>
        public HppStructGenerator(CppObjectSchema validatedSchema, out List<String> warnings)
            : base(validatedSchema, out warnings)
        {
            // Cache some commonly used code.
            var templateNames = new List<String>(validatedSchema.Templates.Count);
            var templateDefinitions = new List<String>(validatedSchema.Templates.Count);
            foreach (var x in validatedSchema.Templates)
            {
                templateNames.Add(x.Name);
                templateDefinitions.Add(String.Concat(x.Key, " ", x.Name));
            } // foreach (...)
            this.templateSignature = (templateNames.Count == 0)
                ? null
                : $"template <{String.Join(", ", templateDefinitions)}>";

            this.templatedStructName = this.templateSignature is null
                ? validatedSchema.Typename
                : String.Concat(validatedSchema.Typename, "<", String.Join(", ", templateNames), ">");
            this.qualifiedStructName = String.Concat(validatedSchema.Namespace, "::", this.templatedStructName);
        } // HppStructGenerator(...)

        private CodeBuilder MakeTypedefs()
        {
            var builder = new CodeBuilder();
            // ~~ Self ~~
            builder.Append($"using type = {this.templatedStructName};");
            // ~~ Base types ~~
            this.ValidatedSchema.Inherits.ForEach(
                (x, i) => builder.Append($"using base_type_{i} = {x.Name};")
            );
            // ~~ Aliases ~~
            foreach (var x in this.ValidatedSchema.Templates)
                if (x.Alias is not null)
                    builder.Append($"using {x.Alias} = {x.Name};");
            // ~~ Fields ~~
            builder
                .Append()
                .Append("// ~~ Field typedefs ~~");
            // We need typedefs even for inherited members for hashing.
            foreach (var x in this.ValidatedSchema.Properties)
                builder.Append($"using {x.MethodName}_type = {x.Typename};");
            return builder;
        } // MakeTypedefs(...)

        private CodeBuilder MakeNoexceptValidation()
        {
            if (!this.ValidatedSchema.DoesRequireValidation) return new();

            var builder = new CodeBuilder();
            builder
                .Append()
                .Append("/** @brief Validates the structure and returns an error message, if any. */")
                .Append("std::optional<std::string> error_message() const noexcept")
                .Append("{");
            using (builder.NewCodeBlock())
            {
                builder
                    .Append("const type& self = *this;")
                    .Append();

                this.ValidatedSchema.Inherits.ForEach((x, i) =>
                {
                    if (x.ErrorMessageCall is null) return;

                    var variable_name = String.Concat("message_", i.ToString());
                    var base_type_name = String.Concat("base_type_", i.ToString());
                    builder
                        .Append($"auto {variable_name} = self.{base_type_name}::{x.ErrorMessageCall};")
                        .Append($"static_assert(std::same_as<std::optional<std::string>, decltype({variable_name})>);")
                        .Append($"if ({variable_name}.has_value()) return {variable_name};")
                        .Append();
                });

                foreach (var x in this.ValidatedSchema.Properties)
                    if (x.DoesRequireValidation && !x.IsPropertyInherited)
                        foreach (var y in x.ValidationFormats)
                            builder.Append(y.Format($"self.{x.FieldName}"));

                builder.Append("return std::nullopt;");
            } // using (...)
            builder.Append($"}} // error_message(...)");
            return builder;
        } // MakeNoexceptValidation(...)

        /// <summary>
        /// Creates one or two constructors: one default (always), one with required arguments (if any).
        /// </summary>
        private CodeBuilder MakeConstructors()
        {
            var builder = new CodeBuilder();
            var countRequired = this.ValidatedSchema.RequiredProperties.Count;

            var defaultFormatArgs = new String[countRequired];
            var explicitFormatArgs = new String[countRequired];

            var explicitArguments = new List<String>(this.ValidatedSchema.Properties.Count);
            var explicitInitializers = new List<String>(this.ValidatedSchema.Properties.Count);
            for (var i = 0; i < countRequired; ++i)
            {
                var x = this.ValidatedSchema.RequiredProperties[i];
                defaultFormatArgs[i] = x.DefaultValueCode;
                explicitFormatArgs[i] = x.MethodName;

                explicitArguments.Add(x.DoPassByValue
                    ? $"{x.MethodName}_type {x.MethodName}"
                    : $"const {x.MethodName}_type& {x.MethodName}");
                if (!x.IsPropertyInherited) explicitInitializers.Add($"{x.FieldName}({x.MethodName})");
            } // for (...)

            // ~~ Base constructors ~~
            var baseDefaultConstructors = new List<String>(this.ValidatedSchema.Inherits.Count);
            var baseExplicitConstructors = new List<String>(this.ValidatedSchema.Inherits.Count);
            this.ValidatedSchema.Inherits.ForEach((x, i) =>
            {
                if (x.ConstructorArgumentsFormat is null) return;

                var defaultArgs = String.Format(x.ConstructorArgumentsFormat, defaultFormatArgs);
                var explicitArgs = String.Format(x.ConstructorArgumentsFormat, explicitFormatArgs);

                var baseTypename = String.Concat("base_type_", i.ToString());
                baseDefaultConstructors.Add(String.Concat(baseTypename, "(", defaultArgs, ")"));
                baseExplicitConstructors.Add(String.Concat(baseTypename, "(", explicitArgs, ")"));
            });
            explicitInitializers.InsertRange(0, baseExplicitConstructors);

            // ~~ Default Constructor ~~
            builder.Append(this.ValidatedSchema.DoesRequireValidation
                ? $"{this.ValidatedSchema.Typename}()"
                : $"{this.ValidatedSchema.Typename}() noexcept");

            baseDefaultConstructors.ForEach((x, isFirst, isLast) =>
                builder.Append(String.Concat(
                    isFirst ? ": " : "",
                    x,
                    isLast ? "" : ","
                ), 1));

            if (!this.ValidatedSchema.DoesRequireValidation) builder.Append("{ }");
            else
                builder
                    .Append($"{{")
                    .Append("this->validate();", 1)
                    .Append($"}} // {this.ValidatedSchema.Typename}(...)");

            if (countRequired == 0) return builder;

            // ~~ Explicit Constructor ~~
            builder
                .Append()
                .Append(String.Concat(
                    (countRequired == 1 ? "explicit " : ""),
                    this.ValidatedSchema.Typename,
                    "(", String.Join(", ", explicitArguments), ")",
                    (this.ValidatedSchema.DoesRequireValidation ? "" : " noexcept")));

            explicitInitializers.ForEach((x, isFirst, isLast) =>
                builder.Append(String.Concat(
                    isFirst ? ": " : "",
                    x,
                    isLast ? "" : ","
                ), 1));

            if (!this.ValidatedSchema.DoesRequireValidation) builder.Append("{ }");
            else
                builder
                    .Append($"{{")
                    .Append("this->validate();", 1)
                    .Append($"}} // {this.ValidatedSchema.Typename}(...)");

            return builder;
        } // MakeConstructors(...)

        private CodeBuilder MakePropertyAccessors()
        {
            var builder = new CodeBuilder();
            foreach (var x in this.ValidatedSchema.Properties)
            {
                if (x.IsPropertyInherited) continue; // Inherited fields also inherit getters and setters.

                builder
                    .Append()
                    .Append(x.DoPassByValue
                        ? $"{x.MethodName}_type {x.MethodName}() const noexcept {{ return this->{x.FieldName}; }}"
                        : $"const {x.MethodName}_type& {x.MethodName}() const noexcept {{ return this->{x.FieldName}; }}");

                if (!x.DoesRequireValidation)
                    builder
                        .Append()
                        .Append(x.DoPassByValue
                            ? $"void set_{x.MethodName}({x.MethodName}_type value) noexcept {{ this->{x.FieldName} = value; }}"
                            : $"void set_{x.MethodName}(const {x.MethodName}_type& value) noexcept {{ this->{x.FieldName} = value; }}");
                else
                    builder
                        .Append()
                        .Append(x.DoPassByValue
                            ? $"void set_{x.MethodName}({x.MethodName}_type value)"
                            : $"void set_{x.MethodName}(const {x.MethodName}_type& value)")
                        .Append($"{{")
                        .Append($"this->{x.FieldName} = value;", 1)
                        .Append($"this->validate();", 1)
                        .Append($"}} // set_{x.MethodName}(...)");
            } // foreach (...)
            return builder;
        } // MakePropertyAccessors(...)

        private CodeBuilder MakeNlohmannUtils()
        {
            var builder = new CodeBuilder();
            builder
                .Append()
                .Append($"friend void to_json(nlohmann::json& j, const type& x) noexcept")
                .Append($"{{");
            using (builder.NewCodeBlock())
            {
                if (this.ValidatedSchema.CountRequiredProperties == 0) builder.Append("j = nlohmann::json{};");
                else
                {
                    builder.Append($"j = nlohmann::json{{");
                    // ~~ Requred properties go directly into the initializer ~~
                    using (builder.NewCodeBlock())
                    {
                        this.ValidatedSchema.RequiredProperties.ForEach((x, isFirst, isLast) =>
                            builder.Append(isLast
                                ? $"{{type::jstr_{x.MethodName}, x.{x.FieldName}}}"
                                : $"{{type::jstr_{x.MethodName}, x.{x.FieldName}}},"));
                    } // using (...)
                    builder.Append($"}};");
                } // if (...)

                // ~~ Optional properties go after the initializer ~~
                if (this.ValidatedSchema.CountOptionalProperties != 0)
                {
                    builder
                        .Append()
                        .Append("static type default_instance {};")
                        .Append();
                    this.ValidatedSchema.Properties.ForEach((x, i) =>
                    {
                        if (i < this.ValidatedSchema.CountRequiredProperties) return;
                        builder.Append(x.DefaultConditionFormat is null
                            ? $"if (x.{x.FieldName} != default_instance.{x.FieldName}) j[std::string(type::jstr_{x.MethodName})] = x.{x.FieldName};"
                            : $"if (!({String.Format(x.DefaultConditionFormat, $"x.{x.FieldName}")})) j[std::string(type::jstr_{x.MethodName})] = x.{x.FieldName};");
                    });
                } // if (...)
            } // using (...)
            builder
                .Append($"}} // to_json(...)")
                .Append()
                .Append($"friend void from_json(const nlohmann::json& j, type& x)")
                .Append($"{{")
                .Append($"if (!ropufu::noexcept_json::try_get(j, x))", 1)
                .Append($"throw std::runtime_error(\"Parsing <{this.ValidatedSchema.Typename}> failed: \" + j.dump());", 2)
                .Append($"}} // from_json(...)");
            return builder;
        } // MakeNlohmannUtils(...)

        private CodeBuilder MakeStructBody()
        {
            var builder = new CodeBuilder();

            builder
                .Append($"namespace {this.ValidatedSchema.Namespace}")
                .Append($"{{");
            using (builder.NewCodeBlock())
            {
                // ~~ Struct forward declaration ~~
                if (this.ValidatedSchema.Description is not null)
                    builder.Append($"/** {this.ValidatedSchema.Description} */");
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"struct {this.ValidatedSchema.Typename};");

                // ~~ Nlohmann utils forward declaration ~~
                builder.Append();
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"void to_json(nlohmann::json& j, const {templatedStructName}& x) noexcept;");
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"void from_json(const nlohmann::json& j, {templatedStructName}& x);");
                builder.Append();

                // ~~ Struct body ~~
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"struct {this.ValidatedSchema.Typename}");
                this.ValidatedSchema.Inherits.ForEach((x, isFirst, isLast) =>
                    builder.Append(String.Concat(
                        isFirst ? ": " : "",
                        x.InheritanceType, " ", x.Name,
                        isLast ? "" : ","
                    ), 1));
                builder.Append("{");
                using (builder.NewCodeBlock())
                {
                    builder.Append(this.MakeTypedefs());

                    builder
                        .Append()
                        .Append("// ~~ Json keys ~~");
                    foreach (var x in this.ValidatedSchema.Properties)
                        builder.Append($"static constexpr std::string_view jstr_{x.MethodName} = \"{x.PropertyName}\";");

                    // ~~ Friend declarations ~~
                    builder
                        .Append()
                        .Append($"friend ropufu::noexcept_json_serializer<type>;")
                        .Append($"friend std::hash<type>;");

                    // ~~ Fields ~~
                    builder
                        .Append()
                        .Append("protected:", -1);
                    foreach (var x in this.ValidatedSchema.Properties)
                        if (!x.IsPropertyInherited)
                            builder.Append($"{x.MethodName}_type {x.FieldName} = {x.DefaultValueCode};{x.Description?.Prepend(" // ") ?? ""}");

                    // ~~ Soft validation ~~
                    builder.Append(this.MakeNoexceptValidation());

                    builder
                        .Append()
                        .Append("public:", -1);

                    // ~~ Hard validation ~~
                    if (!this.ValidatedSchema.DoesRequireValidation)
                        builder
                            .Append("constexpr void validate() const noexcept { }")
                            .Append();
                    else
                    {
                        builder
                            .Append("/** @exception std::logic_error Validation failed. */")
                            .Append("void validate() const")
                            .Append("{")
                            .Append("std::optional<std::string> message = this->error_message();", 1)
                            .Append("if (message.has_value()) throw std::logic_error(message.value());", 1)
                            .Append("} // validate(...)")
                            .Append();
                    } // if (...)

                    // ~~ Constructors ~~
                    builder.Append(this.MakeConstructors());

                    // ~~ Properties (getters and setters) ~~
                    builder.Append(this.MakePropertyAccessors());

                    // ~~ Equality operators ~~
                    builder
                        .Append()
                        .Append("/** Checks if this object is equivalent to \\param other. */")
                        .Append("bool operator ==(const type& other) const noexcept")
                        .Append("{")
                        .Append("return", 1);
                    this.ValidatedSchema.Properties.ForEach((x, isFirst, isLast) =>
                        builder.Append(String.Concat(
                            $"this->{x.FieldName} == other.{x.FieldName}",
                            isLast ? ";" : " &&"
                        ), 2));
                    builder.Append("} // operator ==(...)");

                    builder
                        .Append()
                        .Append("/** Checks if this object is not equivalent to \\param other. */")
                        .Append("bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }");

                    // ~~ Nlohmann utils body ~~
                    builder.Append(this.MakeNlohmannUtils());

                    // ~~ Extensions ~~
                    if (this.ValidatedSchema.Extensions.Count > 0)
                    {
                        builder.Append();
                        foreach (var x in this.ValidatedSchema.Extensions) builder.Append(x);
                    } // if (...)
                } // using (...)
                builder.Append($"}}; // struct {this.ValidatedSchema.Typename}");

            } // using (...)
            builder.Append($"}} // namespace {this.ValidatedSchema.Namespace}");

            return builder;
        } // MakeStructBody(...)

        private CodeBuilder MakeNoexceptJson()
        {
            var builder = new CodeBuilder();

            builder
                .Append("namespace ropufu")
                .Append("{");
            using (builder.NewCodeBlock())
            {
                // ~~ Struct forward declaration ~~
                builder
                    .Append(this.templateSignature is null ? "template <>" : this.templateSignature)
                    .Append($"struct noexcept_json_serializer<{this.qualifiedStructName}>")
                    .Append($"{{");
                using (builder.NewCodeBlock())
                {
                    builder
                        .Append($"using result_type = {this.qualifiedStructName};")
                        .Append($"static bool try_get(const nlohmann::json& j, result_type& x) noexcept")
                        .Append($"{{");
                    using (builder.NewCodeBlock())
                    {
                        this.ValidatedSchema.Properties.ForEach((x, i) => builder.Append(
                            i < this.ValidatedSchema.CountRequiredProperties
                            ? $"if (!noexcept_json::required(j, result_type::jstr_{x.MethodName}, x.{x.FieldName})) return false;"
                            : $"if (!noexcept_json::optional(j, result_type::jstr_{x.MethodName}, x.{x.FieldName})) return false;"));

                        builder.Append();
                        if (this.ValidatedSchema.DoesRequireValidation)
                            builder.Append("if (x.error_message().has_value()) return false;");
                        builder.Append("return true;");
                    } // using (...)
                    builder.Append($"}} // try_get(...)");
                } // using (...)
                builder.Append($"}}; // struct noexcept_json_serializer<...>");
            } // using (...)
            builder.Append("} // namespace ropufu");

            return builder;
        } // MakeNoexceptJson(...)

        private CodeBuilder MakeHash()
        {
            var builder = new CodeBuilder();

            builder
                .Append("namespace std")
                .Append("{");
            using (builder.NewCodeBlock())
            {
                builder
                    .Append(this.templateSignature is null ? "template <>" : this.templateSignature)
                    .Append($"struct hash<{this.qualifiedStructName}>")
                    .Append($"{{");
                using (builder.NewCodeBlock())
                {
                    builder
                        .Append($"using argument_type = {this.qualifiedStructName};")
                        .Append($"using result_type = std::size_t;")
                        .Append()
                        .Append($"result_type operator ()(const argument_type& x) const noexcept")
                        .Append($"{{");
                    using (builder.NewCodeBlock())
                    {
                        // TODO: consider shifting in blocks of n = sizeof(result_type).
                        // If the first iteration yields shift = 1, then once the first n
                        // fields are hashed, re-scale to spread the remainder uniformly.
                        builder
                            .Append($"result_type result = 0;")
                            .Append($"constexpr result_type total_width = sizeof(result_type);")
                            .Append($"constexpr result_type width = total_width / {this.ValidatedSchema.Properties.Count};")
                            .Append($"constexpr result_type shift = (width == 0 ? 1 : width);")
                            .Append();

                        // ~~ Hashers ~~
                        foreach (var x in this.ValidatedSchema.Properties)
                            builder.Append($"std::hash<typename argument_type::{x.MethodName}_type> {x.MethodName}_hasher {{}};");

                        builder.Append();
                        var kk = 0;
                        foreach (var x in this.ValidatedSchema.Properties)
                            builder.Append($"result ^= ({x.MethodName}_hasher(x.{x.FieldName}) << ((shift * {kk++}) % total_width));");

                        builder
                            .Append()
                            .Append("return result;");
                    } // using (...)
                    builder.Append($"}} // operator ()(...)");
                } // using (...)
                builder.Append($"}}; // struct hash<...>");
            } // using (...)
            builder.Append("} // namespace std");

            return builder;
        } // MakeHash(...)

        protected override IEnumerable<CodeBuilder> MakeInner()
        {
            yield return this.MakeStructBody();
            yield return this.MakeNoexceptJson();
            if (!this.ValidatedSchema.HasFlag(HppGeneratorOptions.SkipHash)) yield return this.MakeHash();
        } // MakeInner(...)
    } // class HppStructGenerator
} // namespace Ropufu.JsonSchemaToHpp
