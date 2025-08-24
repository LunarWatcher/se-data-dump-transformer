# Known watermarks

As of the 2025-06-30 data dump, Stack Exchange has added bogus data to the data dumps. As of this release, they require additional filtering to be usable. This document describes what these methods are. For convenience, the SE data dump transformer includes filters for each of the watermark types, if they're possible to filter without damaging real data. These are passed to the `sedd-transformer` executable when converting a directory. Example use: `./sedd-transformer --filter-fabricated-v1 -i ${HOME}/Documents/2025-06-30-rev2 -o $(pwd)/2025-06-30-rev2-clean --no-nesting -t xml`

For the sake of information distribution, this document refers to the specific watermarks by a version. If anything major changes that affects or invalidates previous watermarking identification methods, a new version will be added. 

All dates shown in the headers are inclusive. Using a fictional example, "1999-12-30 to 2000-03-30" means the first affected data dump was 1999-12-30, and the last affected data dump was 2000-03-30. The first version not affected would then be 2000-06-30. 

## v1 watermarks: 2025-06-30 to present

Transformer filter flag: `--filter-fabricated-v1`

https://meta.stackexchange.com/questions/412018/fabricated-data-in-posts-xml-for-multiple-all-data-dumps

Some of the data dumps have two bogus rows. These are identified by the post IDs `1000000001` and `1000000010`. Full format:
```
  <row Id="1000000001" PostTypeId="1" CreationDate="2025-06-01T01:00:00.100" Score="1" ViewCount="100" Body="<random content, occasionally blatantly fake, occasionally blatantly aimed at being a trap>" OwnerUserId="-1" LastEditorUserId="-1" LastEditDate="2025-06-01T01:00:00.100" LastActivityDate="2025-06-01T01:00:00.100" Title="<random content, occasionally blatantly fake, occasionally blatantly aimed at being a trap>" Tags="<varies>" AnswerCount="1" CommentCount="0" ContentLicense="CC BY-SA 4.0" />
  <row Id="1000000010" PostTypeId="2" CreationDate="2025-06-01T01:03:15.100" Score="1" Body="<as question>" OwnerUserId="-1" LastEditorUserId="-1" LastEditDate="2025-06-01T01:03:15.100" LastActivityDate="2025-06-01T01:03:15.100" CommentCount="0" ContentLicense="CC BY-SA 4.0" />
```

Some notes:

* The IDs are always fixed
* The posts are present in `Posts.xml`, but they're not present in `PostHistory.xml`
* The data is fabricated, but it's unclear how it's fabricated. Some reference tools and versions that don't exist yet, some seem to be aimed at creating bogus instructions for AI agents. Some of them have typos, but it's unclear how this relates to the generation process. 
* Until the 2025-09-30 data dump, it's not possible to tell what relation the dates have to the watermarking.

## No watermarks: all data dumps until 2025-03-30

2025-03-30 was the last known version that didn't contain watermarks.
