using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    public enum JsonSchemaValueKind
    {
        Null,
        Boolean,
        Integer,
        Number,
        String,
        Array,
        Object
    } // enum JsonSchemaValueKind
} // namespace Ropufu.JsonSchemaToHpp
