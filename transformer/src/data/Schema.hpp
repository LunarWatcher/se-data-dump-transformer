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

inline std::map<DataDumpFileType::DataDumpFileType, std::map<std::string, FieldType>> schema = {
    {DataDumpFileType::BADGES, {
        {"Id", LONG},
        {"UserId", LONG},
        {"Name", STRING},
        {"Date", DATE},
        {"Class", LONG},
        {"TagBased", BOOL}       
    }},
    {DataDumpFileType::COMMENTS, {
        {"Id", LONG},
        {"PostId", LONG},
        {"Score", LONG},
        {"Text", STRING},
        {"CreationDate", DATE},
        {"UserDisplayName", STRING},
        {"UserId", LONG},
        {"ContentLicense", STRING}
    }},
    {DataDumpFileType::POST_HISTORY, {
        {"Id", LONG},
        {"PostHistoryTypeId", LONG},
        {"PostId", LONG},
        {"RevisionGUID", STRING},
        {"CreationDate", DATE},
        {"UserId", LONG},
        {"UserDisplayName", STRING},
        {"Comment", STRING},
        {"Text", STRING},
        {"ContentLicense", STRING}
    }},
    {DataDumpFileType::POST_LINKS, {
        {"Id", LONG},
        {"CreationDate", DATE},
        {"PostId", LONG},
        {"RelatedPostId", LONG},
        {"LinkTypeId", LONG}
    }},
    {DataDumpFileType::POSTS, {
        {"Id", LONG},
        {"PostTypeId", LONG},
        {"AcceptedAnswerId", LONG},
        {"ParentId", LONG},
        {"CreationDate", DATE},
        {"Score", LONG},
        {"ViewCount", LONG},
        {"Body", STRING},
        {"OwnerUserId", LONG},
        {"OwnerDisplayName", STRING},
        {"LastEditorUserId", LONG},
        {"LastEditorDisplayName", STRING},
        {"LastEditDate", DATE},
        {"LastActivityDate", DATE},
        {"Title", STRING},
        {"Tags", STRING},
        {"AnswerCount", LONG},
        {"CommentCount", LONG},
        {"FavoriteCount", LONG},
        {"ClosedDate", DATE},
        {"CommunityOwnedDate", DATE},
        {"ContentLicense", STRING}
    }},
    {DataDumpFileType::TAGS, {
        {"Id", LONG},
        {"TagName", STRING},
        {"Count", LONG},
        {"ExcerptPostId", LONG},
        {"WikiPostId", LONG},
        {"IsModeratorOnly", BOOL},
        {"IsRequired", BOOL}
    }},
    {DataDumpFileType::USERS, {
        {"Id", LONG},
        {"Reputation", LONG},
        {"CreationDate", DATE},
        {"DisplayName", STRING},
        {"LastAccessDate", DATE},
        {"WebsiteUrl", STRING},
        {"Location", STRING},
        {"AboutMe", STRING},
        {"Views", LONG},
        {"UpVotes", LONG},
        {"DownVotes", LONG},
        {"ProfileImageUrl", STRING},
        {"AccountId", LONG}
    }},
    {DataDumpFileType::VOTES, {
        {"Id", LONG},
        {"PostId", LONG},
        {"VoteTypeId", LONG},
        {"UserId", LONG},
        {"CreationDate", DATE},
        {"BountyAmount", LONG}
    }},
};

}
