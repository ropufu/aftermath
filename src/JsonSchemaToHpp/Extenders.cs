using System;
using System.Collections.Generic;
using System.Text;
using System.Text.Json;

namespace Ropufu.JsonSchemaToHpp
{
    static class Extenders
    {
        public static String Nullify(this String that)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            return that.Length == 0 ? null : that;
        } // Nullify(...)

        public static String ToFormat(this String that)
            => that?.Replace("{", "{{").Replace("}", "}}") ?? throw new ArgumentNullException(nameof(that));

        public static String ToSnakeCase(this String that, Boolean allowMultipleUnderscores = false)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));

            var builder = new StringBuilder(that.Length * 2);
            var wasUnderscore = false;
            foreach (var c in that)
            {
                var isUnderscore = false;
                if (Char.IsUpper(c)) builder.Append('_').Append(Char.ToLowerInvariant(c));
                else if (Char.IsLower(c)) builder.Append(c);
                else
                {
                    isUnderscore = true;
                    if (allowMultipleUnderscores || !wasUnderscore) builder.Append('_');
                } // if (...)
                wasUnderscore = isUnderscore;
            } // foreach (...)
            return builder.ToString().Trim('_');
        } // ToSnakeCase(...)

        public static String CapitalizeFirstLetter(this String that)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));

            if (that.Length == 0) return that;
            var builder = new StringBuilder(that.Length);
            builder.Append(Char.ToUpperInvariant(that[0]));
            builder.Append(that.AsSpan(1));
            return builder.ToString();
        } // CapitalizeFirstLetter(...)

        public static String Prepend(this String that, String prefix)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            if (prefix is null) throw new ArgumentNullException(nameof(prefix));
            return String.Concat(prefix, that);
        } // Prepend(...)

        public static String Append(this String that, String suffix)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            if (suffix is null) throw new ArgumentNullException(nameof(suffix));
            return String.Concat(that, suffix);
        } // Prepend(...)

        public static StringBuilder AppendUnixLine(this StringBuilder that, String value = "")
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            that.Append(value);
            return that.Append('\n');
        } // AppendUnixLine(...)

        public static Boolean DoesAgree(this JsonValueKind that, JsonSchemaValueKind? other)
        {
            return that switch
            {
                JsonValueKind.Undefined => other is null,
                JsonValueKind.Null => other == JsonSchemaValueKind.Null,
                JsonValueKind.True => other == JsonSchemaValueKind.Boolean,
                JsonValueKind.False => other == JsonSchemaValueKind.Boolean,
                JsonValueKind.Number => (other == JsonSchemaValueKind.Number) || (other == JsonSchemaValueKind.Integer),
                JsonValueKind.String => other == JsonSchemaValueKind.String,
                JsonValueKind.Array => other == JsonSchemaValueKind.Array,
                JsonValueKind.Object => other == JsonSchemaValueKind.Object,
                _ => throw new NotImplementedException("JSON value kind not recognized.")
            };
        } // DoValueKindsAgree(...)

        public static void ForEach<T>(this IList<T> that, Action<T, Boolean, Boolean> action)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            var k = 0;
            foreach (var x in that)
            {
                action(x, k == 0, k == that.Count - 1);
                ++k;
            } // foreach (...)
        } // ForEach(...)

        public static void ForEach<T>(this IList<T> that, Action<T, Int32> action)
        {
            if (that is null) throw new ArgumentNullException(nameof(that));
            var k = 0;
            foreach (var x in that)
            {
                action(x, k);
                ++k;
            } // foreach (...)
        } // ForEach(...)
    } // class Extenders
} // namespace Ropufu.JsonSchemaToHpp
