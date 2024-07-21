# Schema

For the exact schema, see [this question](https://meta.stackexchange.com/q/2677/332043) on meta.SE. All output versions of the data dump follow the exact same schema as the source data dump. It is worth noting that the fields are hard-coded to manage type conversion to more sensible formats; if the data dump schema changes, the code will need to be updated to reflect this change.

That said, the types used to represent the values may deviate slightly between formats. For example, file-based formats are likely to represent the date as a special Date type, while the source XML format and, for example, the transformer's JSON output, use a normal string type.

The exact type changes, if any, will remain consistent within a format. If you're unsure about the exact type, you need to look at one entry, and the type will reliably be defined based on this[^1] -- naturally provided the field is defined, and not null.

## Schema definition within the transformer

The transformer has a `.hpp` file dedicated to defining the schema. This is required as SE doesn't define a namespace with the types for the attributes, which means manually defining the types is required. This is a consequence of XML not supporting any other types.

The definitions operate with four basic types: long, double, string, and date. Note that for most file parsers, dates are treated as strings.

[^1]: This excludes scenarios where SE decides to make single fields have multiple types.

