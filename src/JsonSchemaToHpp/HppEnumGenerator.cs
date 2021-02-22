using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public class HppEnumGenerator: HppGenerator<CppStringSchema>
    {
        protected override IList<String> ReservedNames => new String[] {"type"};

        protected override IEnumerable<HppInclude> MoreIncludes => new HppInclude[]
        {
            new() { Header = "<nlohmann/json.hpp>" },
            new() { Header = "<ropufu/noexcept_json.hpp>" },
            new() { Header = "<cstddef>", Comment = "std::size_t, std::nullptr_t" },
            new() { Header = "<functional>", Comment = "std::hash" },
            new() { Header = "<ostream>", Comment = "std::ostream" },
            new() { Header = "<stdexcept>", Comment = "std::runtime_error" },
            new() { Header = "<string_view>", Comment = "std::string_view" },
            new() { Header = "<string>", Comment = "std::string" }
        };

        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="NotSupportedException">Only object-typed JSON schemas are supported.</exception>
        /// <exception cref="NotSupportedException">JSON schema must have at least one property.</exception>
        public HppEnumGenerator(CppStringSchema validatedSchema, out List<String> warnings)
            : base(validatedSchema, out warnings)
        {
        } // HppEnumGenerator(...)

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
                builder.Append($"struct {this.ValidatedSchema.Typename};");

                // ~~ Nlohmann utils forward declaration ~~
                builder
                    .Append()
                    .Append($"void to_json(nlohmann::json& j, const {this.ValidatedSchema.Typename}& x) noexcept;")
                    .Append($"void from_json(const nlohmann::json& j, {this.ValidatedSchema.Typename}& x);")
                    .Append();

                // ~~ Struct body ~~
                builder
                    .Append($"struct {this.ValidatedSchema.Typename}")
                    .Append($"{{");
                using (builder.NewCodeBlock())
                {
                    builder
                        .Append($"using type = {this.ValidatedSchema.Typename};")
                        .Append()
                        .Append($"friend ropufu::noexcept_json_serializer<type>;")
                        .Append($"friend std::hash<type>;")
                        .Append()
                        .Append($"private:", -1)
                        .Append($"char m_value = 0;")
                        .Append()
                        .Append($"constexpr {this.ValidatedSchema.Typename}(std::nullptr_t, char value) noexcept : m_value(value) {{ }}")
                        .Append()
                        .Append($"constexpr void parse(std::string_view value) noexcept")
                        .Append($"{{");
                    this.ValidatedSchema.PermissibleValueCodes.ForEach((x, i) =>
                        builder.Append($"if (value == {x}) this->m_value = {i + 1};", 1));
                    builder
                        .Append($"}} // parse(...)")
                        .Append()
                        .Append($"public:", -1);
                    foreach (var x in this.ValidatedSchema.PermissibleValueCodes)
                        builder.Append($"static const {this.ValidatedSchema.Typename} {x.ToSnakeCase()};");

                    builder
                        .Append()
                        .Append($"constexpr {this.ValidatedSchema.Typename}() noexcept {{ }}")
                        .Append($"constexpr {this.ValidatedSchema.Typename}(std::string_view value) noexcept {{ this->parse(value); }}")
                        .Append($"constexpr {this.ValidatedSchema.Typename}(const char* value) noexcept {{ this->parse(value); }}")
                        .Append($"{this.ValidatedSchema.Typename}(const std::string& value) noexcept {{ this->parse(value); }}")
                        .Append()
                        .Append($"constexpr std::string_view to_string_view() const noexcept")
                        .Append($"{{")
                        .Append($"switch (this->m_value)", 1)
                        .Append($"{{", 1);
                    this.ValidatedSchema.PermissibleValueCodes.ForEach((x, i) =>
                        builder.Append($"case {i + 1}: return {x};", 2));
                    builder
                        .Append($"default: return \"??\";", 2)
                        .Append($"}} // switch (...)", 1)
                        .Append($"}} // to_string_view(...)")
                        .Append()
                        .Append($"/** Checks if this object is equivalent to \\param other. */")
                        .Append($"bool operator ==(const type& other) const noexcept")
                        .Append($"{{")
                        .Append($"return this->m_value == other.m_value;", 1)
                        .Append($"}} // operator ==(...)")
                        .Append()
                        .Append("/** Checks if this object is not equivalent to \\param other. */")
                        .Append("bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }")
                        .Append()
                        .Append($"friend std::ostream& operator <<(std::ostream& os, type self)")
                        .Append($"{{")
                        .Append($"return os << self.to_string_view();", 1)
                        .Append($"}} // operator <<(...)")
                        .Append()
                        .Append($"friend void to_json(nlohmann::json& j, const type& x) noexcept")
                        .Append($"{{")
                        .Append($"j = x.to_string_view();", 1)
                        .Append($"}} // to_json(...)")
                        .Append()
                        .Append($"friend void from_json(const nlohmann::json& j, type& x)")
                        .Append($"{{")
                        .Append($"if (!ropufu::noexcept_json::try_get(j, x))", 1)
                        .Append($"throw std::runtime_error(\"Parsing <{this.ValidatedSchema.Typename}> failed: \" + j.dump());", 2)
                        .Append($"}} // from_json(...)");
                } // using (...)

                builder
                    .Append($"}}; // struct {this.ValidatedSchema.Typename}")
                    .Append();
                this.ValidatedSchema.PermissibleValueCodes.ForEach((x, i) =>
                    builder.Append($"const {this.ValidatedSchema.Typename} {this.ValidatedSchema.Typename}::{x.ToSnakeCase()} = {{nullptr, {i + 1}}};"));
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
                    .Append($"template <>")
                    .Append($"struct noexcept_json_serializer<{this.ValidatedSchema.Namespace}::{this.ValidatedSchema.Typename}>")
                    .Append($"{{");
                using (builder.NewCodeBlock())
                {
                    builder
                        .Append($"using result_type = {this.ValidatedSchema.Namespace}::{this.ValidatedSchema.Typename};")
                        .Append($"static bool try_get(const nlohmann::json& j, result_type& x) noexcept")
                        .Append($"{{")
                        .Append($"std::string value {{}};", 1)
                        .Append($"if (!noexcept_json::try_get(j, value)) return false;", 1)
                        .Append($"x.parse(value);", 1)
                        .Append($"return x.m_value != 0;", 1)
                        .Append($"}} // try_get(...)");
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
                    .Append($"template <>")
                    .Append($"struct hash<{this.ValidatedSchema.Namespace}::{this.ValidatedSchema.Typename}>")
                    .Append($"{{");
                using (builder.NewCodeBlock())
                {
                    builder
                        .Append($"using argument_type = {this.ValidatedSchema.Namespace}::{this.ValidatedSchema.Typename};")
                        .Append($"using result_type = std::size_t;")
                        .Append()
                        .Append($"result_type operator ()(const argument_type& x) const noexcept")
                        .Append($"{{")
                        .Append($"std::hash<char> hasher {{}};", 1)
                        .Append($"return hasher(x.m_value);", 1)
                        .Append($"}} // operator ()(...)");
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
            yield return this.MakeHash();
        } // MakeInner(...)
    } // class HppGenerator
} // namespace Ropufu.JsonSchemaToHpp
