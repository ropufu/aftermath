using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public class HppGenerator
    {
        private static readonly HppInclude[] StandardIncludes = new HppInclude[]
        {
            new() { Header = "<nlohmann/json.hpp>" },
            new() { Header = "<ropufu/noexcept_json.hpp>" },
            new() { Header = "<ropufu/number_traits.hpp>" },
            new() { Header = "<cstddef>", Comment = "std::size_t" },
            new() { Header = "<functional>", Comment = "std::hash" },
            new() { Header = "<stdexcept>", Comment = "std::runtime_error" },
            new() { Header = "<string_view>", Comment = "std::string_view" },
            new() { Header = "<string>", Comment = "std::string" }
        };

        private static readonly HppInclude[] ValidationIncludes = new HppInclude[]
        {
            new() { Header = "<optional>", Comment = "std::optional, std::nullopt" }
        };

        private readonly CppObjectSchema validatedSchema;
        private readonly List<CodeBuilder> definitionBuilders;
        private readonly String templateSignature = null;
        private readonly String templatedStructName = String.Empty;
        private readonly String qualifiedStructName = String.Empty;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="schema"></param>
        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="NotSupportedException">Only object-typed JSON schemas are supported.</exception>
        /// <exception cref="NotSupportedException">JSON schema must have at least one property.</exception>
        public HppGenerator(CppObjectSchema validatedSchema, out List<String> warnings)
        {
            if (validatedSchema is null) throw new ArgumentNullException(nameof(validatedSchema));
            validatedSchema.Invalidate(out warnings);
            
            this.definitionBuilders = new List<CodeBuilder>(validatedSchema.Definitions.Count);
            foreach (var x in validatedSchema.Definitions)
            {
                var generator = new HppGenerator(x, out var moreWarnings);
                var littleBuilder = new CodeBuilder();
                littleBuilder.AppendJoin(
                    generator.MakeBody(),
                    generator.MakeNoexceptJson(),
                    generator.MakeHash());
                this.definitionBuilders.Add(littleBuilder);
            } // foreach (...)

            this.validatedSchema = validatedSchema;
            
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
        } // HppGenerator(...)

        private CodeBuilder MakeIncludes()
        {
            var includes = new SortedSet<HppInclude>();

            if (this.validatedSchema.DoesRequireValidation) includes.UnionWith(HppGenerator.ValidationIncludes);
            includes.UnionWith(HppGenerator.StandardIncludes);
            includes.UnionWith(this.validatedSchema.Includes);

            var builder = new CodeBuilder(includes.Count + 1);

            // ~~ First block: not commented includes ~~
            var countNoComment = 0;
            var maxWidth = 0;
            foreach (var x in includes)
            {
                if (!String.IsNullOrWhiteSpace(x.Comment)) maxWidth = Math.Max(maxWidth, x.Header?.Length ?? 0);
                else
                {
                    builder.Append($"#include {x.Header}");
                    ++countNoComment;
                } // if (...)
            } // foreach (...)

            // ~~ Second block: commented includes ~~
            if (maxWidth > 0)
            {
                if (countNoComment > 0) builder.Append();
                foreach (var x in includes)
                {
                    if (String.IsNullOrWhiteSpace(x.Comment)) continue;
                    builder.Append($"#include {x.Header.PadRight(maxWidth, ' ')} // {x.Comment}");
                } // foreach (...)
            } // if (...)

            return builder;
        } // MakeIncludes(...)

        private CodeBuilder MakeDefinitions()
        {
            if (this.definitionBuilders.Count == 0) return new();
            var builder = new CodeBuilder();
            builder.AppendJoin(this.definitionBuilders);
            return builder;
        } // MakeDefinitions(...)

        private CodeBuilder MakeBody()
        {
            var builder = new CodeBuilder();

            builder
                .Append($"namespace {this.validatedSchema.Namespace}")
                .Append($"{{");
            using (builder.NewCodeBlock())
            {
                // ~~ Struct forward declaration ~~
                if (this.validatedSchema.Description is not null)
                    builder.Append($"/** {this.validatedSchema.Description} */");
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"struct {this.validatedSchema.Typename};");

                // ~~ Nlohmann utils forward declaration ~~
                builder.Append();
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"void to_json(nlohmann::json& j, const {templatedStructName}& x) noexcept;");
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"void from_json(const nlohmann::json& j, {templatedStructName}& x);");
                builder.Append();

                // ~~ Struct body ~~
                if (this.templateSignature is not null) builder.Append(this.templateSignature);
                builder.Append($"struct {this.validatedSchema.Typename}");
                this.validatedSchema.Inherits.ForEach((x, isFirst, isLast) =>
                    builder.Append(String.Concat(
                        isFirst ? ": " : "",
                        String.Format(x, templatedStructName),
                        isLast ? "" : ","
                    ), 1));
                builder.Append("{");
                using (builder.NewCodeBlock())
                {
                    builder.Append($"using type = {templatedStructName};");
                    foreach (var x in this.validatedSchema.Templates)
                        if (x.Alias is not null)
                            builder.Append($"using {x.Alias} = {x.Name};");

                    builder
                        .Append()
                        .Append("// ~~ Field typedefs ~~");
                    // We need typedefs even for inherited members for hashing.
                    foreach (var x in this.validatedSchema.Properties)
                        builder.Append($"using {x.MethodName}_type = {x.Typename};");

                    builder
                        .Append()
                        .Append("// ~~ Json keys ~~");
                    foreach (var x in this.validatedSchema.Properties)
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
                    foreach (var x in this.validatedSchema.Properties)
                        if (!x.IsPropertyInherited)
                            builder.Append($"{x.MethodName}_type {x.FieldName} = {x.DefaultValueCode};{x.Description?.Prepend(" // ") ?? ""}");

                    // ~~ Validation ~~
                    if (this.validatedSchema.DoesRequireValidation)
                    {
                        builder
                            .Append()
                            .Append("/** @brief Validates the structure and returns an error message, if any. */")
                            .Append("std::optional<std::string> error_message() const noexcept")
                            .Append("{");
                        using (builder.NewCodeBlock())
                        {
                            builder.Append("const type& self = *this;");

                            foreach (var x in this.validatedSchema.Properties)
                                if (x.DoesRequireValidation)
                                    foreach (var y in x.ValidationFormats)
                                        builder.Append(y.Format($"self.{x.FieldName}"));

                            builder.Append("return std::nullopt;");
                        } // using (...)
                        builder.Append($"}} // error_message(...)");
                    } // if (...)

                    builder
                        .Append()
                        .Append("public:", -1);

                    // ~~ Validation ~~

                    if (!this.validatedSchema.DoesRequireValidation)
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
                    builder.Append(this.validatedSchema.DoesRequireValidation
                        ? $"{this.validatedSchema.Typename}() {{ this->validate(); }} // Will throw if default values do not pass validation."
                        : $"{this.validatedSchema.Typename}() noexcept {{ }}");

                    var constructorArguments = new List<String>(this.validatedSchema.Properties.Count);
                    var constructorInitializers = new List<String>(this.validatedSchema.Properties.Count);
                    foreach (var x in this.validatedSchema.Properties)
                    {
                        if (!x.IsPropertyRequired) continue;

                        constructorArguments.Add($"{x.MethodName}_type {x.MethodName}");
                        constructorInitializers.Add($"{x.FieldName}({x.MethodName})");
                    } // foreach (...)
                    if (constructorArguments.Count > 0)
                    {
                        builder
                            .Append()
                            .Append(String.Concat(
                                (constructorArguments.Count > 1 ? "" : "explicit "),
                                this.validatedSchema.Typename,
                                "(", String.Join(", ", constructorArguments), ")",
                                (this.validatedSchema.DoesRequireValidation ? "" : " noexcept")));

                        constructorInitializers.ForEach((x, isFirst, isLast) =>
                            builder.Append(String.Concat(
                                isFirst ? ": " : "",
                                String.Format(x, templatedStructName),
                                isLast ? "" : ","
                            ), 1));
                        
                        if (!this.validatedSchema.DoesRequireValidation) builder.Append("{ }");
                        else
                            builder
                                .Append($"{{")
                                .Append("this->validate();", 1)
                                .Append($"}} // {this.validatedSchema.Typename}(...)");
                    } // if (...)

                    // ~~ Properties (getters and setters) ~~
                    foreach (var x in this.validatedSchema.Properties)
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

                    builder
                        .Append()
                        .Append("/** Checks if this object is equivalent to \\param other. */")
                        .Append("bool operator ==(const type& other) const noexcept")
                        .Append("{")
                        .Append("return", 1);
                    this.validatedSchema.Properties.ForEach((x, isFirst, isLast) =>
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
                    builder
                        .Append()
                        .Append($"friend void to_json(nlohmann::json& j, const type& x) noexcept")
                        .Append($"{{");
                    using (builder.NewCodeBlock())
                    {
                        if (this.validatedSchema.CountRequiredProperties == 0) builder.Append("j = nlohmann::json{};");
                        else
                        {
                            builder.Append($"j = nlohmann::json{{");
                            // ~~ Requred properties go directly into the initializer ~~
                            using (builder.NewCodeBlock())
                            {
                                var kk = 0;
                                foreach (var x in this.validatedSchema.Properties)
                                    if (x.IsPropertyRequired)
                                        builder.Append((++kk == this.validatedSchema.CountRequiredProperties)
                                            ? $"{{type::jstr_{x.MethodName}, x.{x.FieldName}}}"
                                            : $"{{type::jstr_{x.MethodName}, x.{x.FieldName}}},");
                            } // using (...)
                            builder.Append($"}};");
                        } // if (...)

                        // ~~ Optional properties go after the initializer ~~
                        if (this.validatedSchema.CountOptionalProperties != 0)
                        {
                            builder
                                .Append()
                                .Append("static type default_instance {};")
                                .Append();
                            foreach (var x in this.validatedSchema.Properties)
                                if (!x.IsPropertyRequired)
                                    if (x.DefaultConditionFormat is null)
                                        builder.Append($"if (x.{x.FieldName} != default_instance.{x.FieldName}) j[std::string(type::jstr_{x.MethodName})] = x.{x.FieldName};");
                                    else
                                        builder.Append($"if (!({String.Format(x.DefaultConditionFormat, $"x.{x.FieldName}")})) j[std::string(type::jstr_{x.MethodName})] = x.{x.FieldName};");
                        } // if (...)
                    } // using (...)
                    builder
                        .Append($"}} // to_json(...)")
                        .Append()
                        .Append($"friend void from_json(const nlohmann::json& j, {templatedStructName}& x)")
                        .Append($"{{");
                    using (builder.NewCodeBlock())
                    {
                        builder
                            .Append($"if (!noexcept_json::try_get(j, x))")
                            .Append($"throw std::runtime_error(\"Parsing <{this.validatedSchema.Typename}> failed: \" + j.dump());", 1);
                    } // using (...)
                    builder.Append($"}} // from_json(...)");
                } // using (...)
                builder.Append($"}}; // struct {this.validatedSchema.Typename}");

            } // using (...)
            builder.Append($"}} // namespace {this.validatedSchema.Namespace}");

            return builder;
        } // MakeBody(...)

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
                        foreach (var x in this.validatedSchema.Properties)
                            builder.Append($"if (!noexcept_json::{(x.IsPropertyRequired ? "required" : "optional")}(j, result_type::jstr_{x.MethodName}, x.{x.FieldName})) return false;");

                        builder.Append();
                        if (this.validatedSchema.DoesRequireValidation)
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
                            .Append($"constexpr result_type width = total_width / {this.validatedSchema.Properties.Count};")
                            .Append($"constexpr result_type shift = (width == 0 ? 1 : width);")
                            .Append();

                        // ~~ Hashers ~~
                        foreach (var x in this.validatedSchema.Properties)
                            builder.Append($"std::hash<typename argument_type::{x.MethodName}_type> {x.MethodName}_hasher = {{}};");

                        builder.Append();
                        var kk = 0;
                        foreach (var x in this.validatedSchema.Properties)
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

        public override String ToString()
        {
            var builder = new CodeBuilder();

            var includeGuard = String.Join("_",
                this.validatedSchema.Namespace.ToSnakeCase(false).ToUpperInvariant(),
                this.validatedSchema.Typename.ToSnakeCase(false).ToUpperInvariant(),
                Guid.NewGuid().ToString("N").ToUpperInvariant());

            builder
                .Append()
                .Append($"#ifndef {includeGuard}")
                .Append($"#define {includeGuard}")
                .Append()
                .Append("//=======================================================================")
                .Append("// This file was generated automatically.")
                .Append("// Please do not edit it directly, since any changes may be overwritten.")
                .Append("//=======================================================================")
                .Append($"// UTC Date: {DateTime.UtcNow}")
                .Append("//=======================================================================")
                .Append();

            builder.AppendJoin(
                this.MakeIncludes(),
                this.MakeDefinitions(),
                this.MakeBody(),
                this.MakeNoexceptJson(),
                this.MakeHash());

            builder
                .Append()
                .Append($"#endif // {includeGuard}");

            return builder.ToString();
        } // ToString(...)
    } // class HppGenerator
} // namespace Ropufu.JsonSchemaToHpp
