using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    public interface ICppSchema
    {
        /// <summary>
        /// If this schema describes a property in another schema, indicates that the property is inherited from a base class.
        /// </summary>
        Boolean IsPropertyInherited { get; }

        /// <summary>
        /// If this schema describes a property in another schema, indicates that an explicit value of the property is required.
        /// </summary>
        Boolean IsPropertyRequired { get; }

        /// <summary>
        /// If this schema describes a property in another schema, name of the property.
        /// </summary>
        String PropertyName { get; }

        /// <summary>
        /// If this schema describes a property in another schema, name of the associated method.
        /// </summary>
        String MethodName { get; }

        /// <summary>
        /// If this schema describes a property in another schema, name of the associated field.
        /// </summary>
        String FieldName { get; }

        /// <summary>
        /// Code for object type.
        /// </summary>
        /// <example>"std::size_t"</example>
        String Typename { get; }

        /// <summary>
        /// Description of the object.
        /// </summary>
        String Description { get; }

        /// <summary>
        /// Indicates that the default values has not been customized.
        /// </summary>
        Boolean IsDefaultValueTrivial { get; }

        /// <summary>
        /// Code for default value of the object.
        /// </summary>
        /// <example>"{}"</example>
        String DefaultValueCode { get; }

        /// <summary>
        /// One-parameter code format to check for default value.
        /// </summary>
        /// <example>"{0} == 1729"</example>
        String DefaultConditionFormat { get; }

        /// <summary>
        /// Indicates if there are restrictions on the value the object can assume.
        /// </summary>
        Boolean DoesRequireValidation { get; }

        /// <summary>
        /// Indicates that the value should be passed by-value rather than by-reference.
        /// </summary>
        Boolean DoPassByValue { get; }

        /// <summary>
        /// Collection of one-parameter validation code formats.
        /// </summary>
        /// <example>
        ///   [
        ///     "if ({0}.empty()) return \"Array must have at least one element.\";",
        ///     "foreach (const auto& x : {0}) if (x == 0) return \"Zero elements not allowed.\";",
        ///   ]
        /// </example>
        IEnumerable<CodeLine> ValidationFormats { get; }

        String Parse(JsonElement value);
    } // interface ICppType
} // namespace Ropufu.JsonSchemaToHpp
