#pragma once

#include "data/ArchiveParser.hpp"
#include <map>
#include <string>

namespace sedd::Schema {

enum FieldType {
    LONG,
    DOUBLE,
    STRING,
    DATE,
    BOOL
};

struct Field {
    bool pk;
    bool nullable;
    FieldType type;
};

// TODO: Verify the entire schema against https://data.stackexchange.com/stackoverflow/query/472607/information-schema-for-a-table?table=tags
// Some of these are only based on the schema post, and not the export from SEDE, which likely is the closest match
// to the XML files.
inline std::map<DataDumpFileType::DataDumpFileType, std::map<std::string, Field>> schema = {
    {DataDumpFileType::BADGES, 
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"UserId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"Name", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = STRING,
                }},
            {"Date", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"Class", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"TagBased", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = BOOL,
                }},
        }
    },
    {DataDumpFileType::COMMENTS,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"PostId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"Score", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"Text", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = STRING,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"UserDisplayName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"UserId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ContentLicense", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }}
        }
    },
    {DataDumpFileType::POST_HISTORY,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"PostHistoryTypeId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"PostId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"RevisionGUID", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = STRING,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"UserId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"UserDisplayName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Comment", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Text", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"ContentLicense", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }}
        }
    },
    {DataDumpFileType::POST_LINKS,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"PostId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"RelatedPostId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"LinkTypeId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }}
        }
    },
    {DataDumpFileType::POSTS,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"PostTypeId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"AcceptedAnswerId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ParentId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"Score", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ViewCount", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"Body", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = STRING,
                }},
            {"OwnerUserId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"OwnerDisplayName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"LastEditorUserId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"LastEditorDisplayName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"LastEditDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"LastActivityDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"Title", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Tags", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"AnswerCount", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"CommentCount", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"FavoriteCount", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ClosedDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"CommunityOwnedDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"ContentLicense", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }}
        }
    },
    {DataDumpFileType::TAGS,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"TagName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Count", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ExcerptPostId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"WikiPostId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"IsModeratorOnly", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = BOOL,
                }},
            {"IsRequired", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = BOOL,
                }}
        }
    },
    {DataDumpFileType::USERS,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"Reputation", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = DATE,
                }},
            {"DisplayName", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"LastAccessDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"WebsiteUrl", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Location", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"AboutMe", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"Views", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"UpVotes", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"DownVotes", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"ProfileImageUrl", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = STRING,
                }},
            {"AccountId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }}
        }
    },
    {DataDumpFileType::VOTES,
        {
            {"Id", 
                Field {
                    .pk = true,
                    .nullable = false,
                    .type = LONG,
                }},
            {"PostId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"VoteTypeId", 
                Field {
                    .pk = false,
                    .nullable = false,
                    .type = LONG,
                }},
            {"UserId", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }},
            {"CreationDate", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = DATE,
                }},
            {"BountyAmount", 
                Field {
                    .pk = false,
                    .nullable = true,
                    .type = LONG,
                }}
        }
    },
};

}
