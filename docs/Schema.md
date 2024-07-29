# Schema

For the exact schema, see [this question](https://meta.stackexchange.com/q/2677/332043) on meta.SE. All output versions of the data dump follow the exact same schema as the source data dump. It is worth noting that the fields are hard-coded to manage type conversion to more sensible formats; if the data dump schema changes, the code will need to be updated to reflect this change.

That said, the types used to represent the values may deviate slightly between formats. For example, file-based formats are likely to represent the date as a special Date type, while the source XML format and, for example, the transformer's JSON output, use a normal string type.

The exact type changes, if any, will remain consistent within a format. If you're unsure about the exact type, you need to look at one entry, and the type will reliably be defined based on this[^1] -- naturally provided the field is defined, and not null.

## Schema definition within the transformer

The transformer has a `.hpp` file dedicated to defining the schema. This is required as SE doesn't define a namespace with the types for the attributes, which means manually defining the types is required. This is a consequence of XML not supporting any other types.

The definitions operate with four basic types: long, double, string, and date. Note that for most file parsers, dates are treated as strings.

## Scope

The schema contains three keys; pk (primary key), nullable, and FieldType, which is one of five predefined constants. 

These exist to allow any database system to create tables just based on this schema without having to separately and individually hardcode fields. This system is meant to allow slapping a for loop on some generator code to make stuff work.

Any file-based system that doesn't do a schema, but that can iterate over the columns are not required to look at either the `pk` or the `nullable` fields. The type can be used to assist with conversion, which is necessary due to the previously mentioned lack of explicit typing in XML.

The main thing the schema is missing right now are compatibility flags, notably for fields that are no longer current for newer releases of the data dump. However, historically, the data dump fields have only changed a handful of times, so this is not a major concern for the initial release of the system. There's also a small problem with actually determining which version of the data dump is being parsed, as there are no reliable indicators in the data dump itself, aside maybe parsing the date in the last row of the SO dataset[^2] and guessing based on that.

## Footnotes

[^1]: This excludes scenarios where SE decides to make single fields have multiple types.
[^2]: While any of the XML files would in theory do, it needs to be active. If an inactive site is picked, it's possible that the wrong date is picked just  because the file hasn't been updated. This is the least likely to happen with the SO dump, but this presents issues if SO isn't part of the dataset being converted. 
    
    This is a tricky problem, and it's arguably better left for Later
