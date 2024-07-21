# JSON

## Using the data

The JSON output, similarly to the source XML data from the data dump, is partly pretty-printed. Each line of the resulting JSON object, except for the first and last lines, represent one single JSON entry. For example, the JSON output of a data dump can look like:
```json
[
  {"field1": "value1", "field2": "value2", "...": "..." },
  {"field1": "value1", "field2": "value2", "...": "..." },
  {"field1": "value1", "field2": "value2", "...": "..." },
  {"field1": "value1", "field2": "value2", "...": "..." }
]
```

This is to work around many JSON parsers not supporting incremental parsing; i.e. where you feed the parser the file, and you read it one line at a time, parsing as you go.

If your JSON parser doesn't support incremental parsing, you can read a line, strip the trailing comma if it exists (and your JSON parser doesn't support JSON 5, where trailing commas are allowed), and put the resulting JSON into a parser. This will give you one single entry form the data dump, that you can discard when you're done, before moving onto the next line.

This is, essentially, meant to allow incremental parsing without a JSON parser that does incremental parsing.

As with any file-based format, **it's a bad idea to read the entire file at once**. If you're *absolutely sure* that the specific file you're reading is small, carry on - but if you have a system set up to read from any site, you can and will run into problems on the bigger sites - especially Stack Overflow, which is almost 200GB in the source XML. If you need to read anything big, use some form of incremental parsing - either with support from your JSON parser, or by reading the file one line at a time.

