using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public record CodeLine
    {
        private String code = "";

        public Int32 TabOffset { get; init; } = 0;

        public String Code
        {
            get => this.code;
            set => this.code = (value ?? throw new ArgumentNullException(nameof(CodeLine.Code)));
        } // Code

        public Boolean IsEmpty => String.IsNullOrWhiteSpace(this.code);

        public CodeLine() {}

        public CodeLine(String code, Int32 tabOffset = 0)
        {
            this.code = (code ?? throw new ArgumentNullException(nameof(code)));
            this.TabOffset = tabOffset;
        } // CodeLine(...)

        public CodeLine Format(Object arg0)
            => new(String.Format(this.code, arg0), this.TabOffset);
        public CodeLine Format(Object arg0, Object arg1)
            => new(String.Format(this.code, arg0, arg1), this.TabOffset);
        public CodeLine Format(params Object[] args)
            => new(String.Format(this.code, args), this.TabOffset);

        public static implicit operator CodeLine(String code) => new(code);
    } // struct CodeLine
} // namespace Ropufu.JsonSchemaToHpp
