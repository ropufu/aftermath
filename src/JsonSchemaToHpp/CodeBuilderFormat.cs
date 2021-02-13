using System;
using System.Collections.Generic;
using System.Text;

namespace Ropufu.JsonSchemaToHpp
{
    public record CodeBuilderFormat(
        Int32 TabSize = 4,
        Char TabSymbol = ' ',
        String NewLineSequence = "\n",
        Boolean DoCollapseEmptyLines = true);
} // namespace Ropufu.JsonSchemaToHpp
