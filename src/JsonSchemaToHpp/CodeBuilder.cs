using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public sealed class CodeBuilder
    {
        class BuilderBlock : IDisposable
        {
            private Boolean disposedValue;
            private CodeBuilder builder;

            public BuilderBlock(CodeBuilder builder)
            {
                // if (builder is null) throw new ArgumentNullException(nameof(builder));
                this.builder = builder;
                ++this.builder.TabLevel;
            } // BuilderBlock(...)

            private void Dispose(Boolean disposing)
            {
                if (!this.disposedValue)
                {
                    if (disposing)
                    {
                        --this.builder.TabLevel;
                    } // if (...)

                    this.builder = null;
                    this.disposedValue = true;
                } // if (...)
            } // Dispose(...)

            public void Dispose()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                this.Dispose(disposing: true);
                GC.SuppressFinalize(this);
            } // Dispose(...)
        } // class BuilderBlock

        private readonly List<CodeLine> lines = new();

        public Int32 TabLevel { get; private set; }

        public Boolean IsEmpty => this.lines.Count == 0;

        public CodeBuilder()
        {
        } // CodeBuilder(...)

        public CodeBuilder(Int32 capacity) => this.lines = new(capacity);

        /// <summary>
        /// All lines within a code block will have extra indentation.
        /// </summary>
        public IDisposable NewCodeBlock() => new BuilderBlock(this);

        /// <summary>
        /// Adds an empty line to the collection.
        /// </summary>
        public CodeBuilder Append()
        {
            this.lines.Add(new("", this.TabLevel));
            return this;
        } // Append(...)

        /// <summary>
        /// Adds a line to the collection.
        /// </summary>
        public CodeBuilder Append(String value, Int32 tabOffset = 0)
        {
            if (value is null) throw new ArgumentNullException(nameof(value));

            this.lines.Add(new(value, this.TabLevel + tabOffset));
            return this;
        } // Append(...)

        public CodeBuilder Append(CodeLine value)
        {
            if (value is null) throw new ArgumentNullException(nameof(value));

            this.lines.Add(value with {TabOffset = this.TabLevel + value.TabOffset});
            return this;
        } // Append(...)

        public CodeBuilder Append(IEnumerable<CodeLine> values)
        {
            if (values is null) throw new ArgumentNullException(nameof(values));

            foreach (var x in values) this.Append(x);
            return this;
        } // Append(...)

        public CodeBuilder Append(CodeBuilder value)
        {
            if (value is null) throw new ArgumentNullException(nameof(value));

            foreach (var x in value.lines) this.Append(x);
            return this;
        } // Append(...)

        /// <summary>
        /// Joins all non-empty pieces of code separated by an extra line break.
        /// </summary>
        public CodeBuilder AppendJoin(params CodeBuilder[] values) =>
            this.AppendJoin(values as IEnumerable<CodeBuilder>);

        /// <summary>
        /// Joins all non-empty pieces of code separated by an extra line break.
        /// </summary>
        public CodeBuilder AppendJoin(IEnumerable<CodeBuilder> values)
        {
            if (values is null) throw new ArgumentNullException(nameof(values));

            var nonEmpty = new List<CodeBuilder>();
            foreach (var x in values) if (!x.IsEmpty) nonEmpty.Add(x);

            var isFirst = true;
            foreach (var x in nonEmpty)
            {
                if (!isFirst) this.Append();
                this.Append(x);
                isFirst = false;
            } // foreach (...)

            return this;
        } // AppendJoin(...)

        public override String ToString() => this.ToString(new());

        public String ToString(CodeBuilderFormat format)
        {
            if (format is null) throw new ArgumentNullException(nameof(format));

            var builder = new StringBuilder();
            foreach (var x in this.lines)
            {
                var offset = format.TabSize * (x.TabOffset < 0 ? 0 : x.TabOffset);
                if (!(format.DoCollapseEmptyLines && x.IsEmpty))
                    builder
                        .Append(format.TabSymbol, repeatCount: offset)
                        .Append(x.Code);
                builder.Append(format.NewLineSequence);
            } // foreach (...)
            return builder.ToString();
        } // ToString(...)
    } // class CodeBuilder
} // namespace Ropufu.JsonSchemaToHpp
