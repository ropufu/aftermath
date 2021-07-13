using System;
using System.Collections.Generic;

namespace Ropufu.JsonSchemaToHpp
{
    class Program
    {
        private static List<String> Unpack(String[] args)
        {
            try
            {
                var result = new List<String>();
                foreach (var x in args)
                {
                    var attributes = System.IO.File.GetAttributes(x);
                    var files = attributes.HasFlag(System.IO.FileAttributes.Directory)
                        ? System.IO.Directory.GetFiles(x)
                        : new String[] { x };
                    
                    foreach (var y in files) if (System.IO.Path.GetExtension(y).ToLowerInvariant() == ".json") result.Add(y);
                } // foreach (...)
                return result;
            } // try
            catch (Exception ex) when (
                ex is System.ArgumentException ||
                ex is System.IO.IOException ||
                ex is System.NotSupportedException ||
                ex is System.UnauthorizedAccessException)
            {
                System.Console.WriteLine("Unpacking arguments failed.");
                System.Console.WriteLine(ex.Message);
                return null;
            } // catch (...)
        } // Unpack(...)

        private static String[] Read(List<String> filePaths)
        {
            try
            {
                var result = new String[filePaths.Count];
                for (var i = 0; i < result.Length; ++i)
                    result[i] = System.IO.File.ReadAllText(filePaths[i]);
                return result;
            } // try
            catch (Exception ex) when (
                ex is System.ArgumentException ||
                ex is System.IO.IOException ||
                ex is System.NotSupportedException ||
                ex is System.UnauthorizedAccessException ||
                ex is System.Security.SecurityException)
            {
                System.Console.WriteLine("Reading schema text failed.");
                System.Console.WriteLine(ex.Message);
                return null;
            } // catch (...)
        } // Read(...)

        private static Boolean TryWrite(List<String> filePaths, List<String> fileTexts)
        {
            if (filePaths.Count != fileTexts.Count) throw new ApplicationException();

            try
            {
                foreach (var x in filePaths)
                    System.IO.Directory.CreateDirectory(System.IO.Path.GetDirectoryName(x));

                for (var i = 0; i < filePaths.Count; ++i)
                    System.IO.File.WriteAllText(filePaths[i], fileTexts[i]);
                return true;
            } // try
            catch (Exception ex) when (
                ex is System.ArgumentException ||
                ex is System.IO.IOException ||
                ex is System.NotSupportedException ||
                ex is System.UnauthorizedAccessException ||
                ex is System.Security.SecurityException)
            {
                System.Console.WriteLine("Writing hpp text failed.");
                System.Console.WriteLine(ex.Message);
                return false;
            } // catch (...)
        } // TryWrite(...)

        [STAThread]
        static Int32 Main(String[] args)
        {
            if (args is null || args.Length == 0)
            {
                Console.WriteLine("Path to schema or containing folder missing.");
                return 1;
            } // if (...)

#if RELEASE
            try
            {
#endif
                var jsonSchemaPaths = Program.Unpack(args);
                if (jsonSchemaPaths is null) return 2;
                var jsonSchemaTexts = Program.Read(jsonSchemaPaths);
                if (jsonSchemaTexts is null) return 3;
                var hppPaths = new List<String>(jsonSchemaTexts.Length);
                var hppCodes = new List<String>(jsonSchemaTexts.Length);

                for (var i = 0; i < jsonSchemaTexts.Length; ++i)
                {
                    var text = jsonSchemaTexts[i];
                    var jsonSchema = JsonSchema.Parse(text);
                    jsonSchema.SchemaFileName = System.IO.Path.GetFileName(jsonSchemaPaths[i]);

                    if (jsonSchema.HppTargetPath is null)
                    {
                        System.Console.WriteLine($"Skipping {jsonSchema.SchemaFileName}: target path is null.");
                        continue;
                    } // if (...)

                    if (jsonSchema.HasFlag(HppGeneratorOptions.Ignore))
                    {
                        System.Console.WriteLine($"Skipping {jsonSchema.SchemaFileName}: ignore flag encountered.");
                        continue;
                    } // if (...)

                    var cppSchema = jsonSchema.ToCppType();
                    if (!cppSchema.DoesSupportCodeGeneration)
                    {
                        System.Console.WriteLine($"Skipping {jsonSchema.SchemaFileName}: code generation are supported.");
                        continue;
                    } // if (...)

                    System.Console.Write($"Parsing {jsonSchema.SchemaFileName}...");
                    var generator = HppGeneratorFactory.MakeGenerator(cppSchema, out var warnings);
                    var code = generator.ToString();
                    if (code is null) throw new ApplicationException(); // Should never happen.
                    if (warnings.Count == 0) System.Console.WriteLine(" succeded.");
                    else
                    {
                        System.Console.WriteLine(" succeded with the following warnings:");
                        foreach (var x in warnings)
                        {
                            System.Console.Write("-- ");
                            System.Console.WriteLine(x);
                        } // foreach (...)
                    } // if (...)

                    hppPaths.Add(System.IO.Path.Combine(
                        System.IO.Path.GetDirectoryName(jsonSchemaPaths[i]),
                        jsonSchema.HppTargetPath));
                    hppCodes.Add(code);
                } // foreach (...)

                if (!Program.TryWrite(hppPaths, hppCodes)) return 4;
#if RELEASE
            } // try
            catch (Exception ex)
            {
                System.Console.WriteLine(ex);
                return 1729;
            } // catch (...)
#endif
            return 0;
        } // Main(...)
    } // class Program
} // namespace Ropufu.JsonSchemaToHpp
