using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json.Serialization;

namespace Ropufu.JsonSchemaToHpp
{
    class SchemaException : FormatException
    {
        private static readonly Type JsonSchemaType = typeof(JsonSchema);

        private static String GenerateMessage(JsonSchema schema, String propertyName, String messageFormat)
        {
            if (schema is null) throw new ArgumentNullException(nameof(schema));

            var property = SchemaException.JsonSchemaType.GetProperty(propertyName);
            var attributes = property.GetCustomAttributes(typeof(JsonPropertyNameAttribute), false);
            var attributeNames = new List<String>(attributes.Length);
            foreach (JsonPropertyNameAttribute x in attributes) attributeNames.Add(x.Name);
            var message = String.Format(messageFormat, String.Join(", ", attributeNames));
            return schema.SchemaFileName is null
                ? message
                : String.Concat($"[{schema.SchemaFileName}] ", message);
        } // GenerateMessage(...)

        public SchemaException(String message)
            : base(message)
        {
        } // SchemaException(...)

        public SchemaException(JsonSchema schema, String propertyName, String messageFormat)
            : base(SchemaException.GenerateMessage(schema, propertyName, messageFormat))
        {
        } // SchemaException(...)
    } // class SchemaException
} // namespace Ropufu.JsonSchemaToHpp
