using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public interface IHppGenerator
    {
        /// <summary>
        /// Blocks of inner code, i.e., code other than includes and header guards.
        /// </summary>
        IEnumerable<CodeBuilder> Make();
    } // interface IHppGenerator
} // namespace Ropufu.JsonSchemaToHpp
