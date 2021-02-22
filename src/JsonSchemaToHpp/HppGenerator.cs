using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public abstract class HppGenerator<T> : IHppGenerator
        where T : ICppSchema
    {
        private readonly T validatedSchema;
        private readonly List<CodeBuilder> definitionBuilders;

        protected abstract IList<String> ReservedNames { get; }
        
        /// <summary>
        /// Additional includes not explicitly listed in the JSON schema.
        /// </summary>
        protected abstract IEnumerable<HppInclude> MoreIncludes { get; }

        protected T ValidatedSchema => this.validatedSchema;

        /// <exception cref="ArgumentNullException">Schema cannot be null.</exception>
        /// <exception cref="NotSupportedException">Schema does not support code generation.</exception>
        /// <exception cref="SchemaException">Schema validation failed.</exception>
        public HppGenerator(T cppSchema, out List<String> warnings)
        {
            if (cppSchema is null) throw new ArgumentNullException(nameof(cppSchema));
            cppSchema.Invalidate(this.ReservedNames, out warnings);
            this.validatedSchema = cppSchema;
            
            this.definitionBuilders = new List<CodeBuilder>();
            foreach (var x in cppSchema.Definitions)
            {
                var generator = HppGeneratorFactory.MakeGenerator(x, out var moreWarnings);
                this.definitionBuilders.AddRange(generator.Make());
                warnings.AddRange(moreWarnings);
            } // foreach (...)
        } // HppGenerator(...)

        private CodeBuilder MakeIncludes()
        {
            var includes = new SortedSet<HppInclude>();
            includes.UnionWith(this.validatedSchema.Includes);
            includes.UnionWith(this.MoreIncludes);

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

        protected abstract IEnumerable<CodeBuilder> MakeInner();

        public IEnumerable<CodeBuilder> Make()
        {
            foreach (var x in this.definitionBuilders) yield return x;
            foreach (var x in this.MakeInner()) yield return x;
        } // Make(...)

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
                .Append()
                .Append(this.MakeIncludes())
                .Append();

            builder.AppendJoin(this.Make());

            builder
                .Append()
                .Append($"#endif // {includeGuard}");

            return builder.ToString();
        } // ToString(...)
    } // class HppGenerator
} // namespace Ropufu.JsonSchemaToHpp
