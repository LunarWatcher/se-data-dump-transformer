#pragma once

#include <filesystem>
#include <pugixml.hpp>
#include "ArchiveParser.hpp"
#include "data/ArchiveParser.hpp"

namespace sedd {

class Transformer {
protected:
    bool shouldUnifyInputs;
    std::string extension;
public:
    Transformer(bool shouldUnifyInputs, const std::string& extension = "");

    /**
     * Signals that the currently processed file is done. This is where you should close and clean up existing stuff.
     *
     * WARNING: If you have a sytem that does intermediate commits, such as a database that does in-memory transactions,
     * you do NOT want to only commit here! The biggest (source) files are >100GB uncompressed, and are approaching 200
     * fast. If you only commit here with an in-memory transaction, you can and will run into swap death
     */
    virtual void endFile() = 0;

    /**
     * Signals that a new file has started. This is where you should open connections, files, or whatever else.
     */
    virtual void beginFile(const ParserContext& ctx) = 0;

    /**
     * Signals that a new archive is being started. This function can be used by particularly file writers to better
     * manage an ArchiveWriter instance.
     */
    virtual void beginArchive(const ParserContext& ctx) = 0;

    /**
     * Signals that an archive is being ended
     */
    virtual void endArchive(const ParserContext& ctx) = 0;

    virtual void parseLine(const pugi::xml_node& row, const ParserContext& ctx) = 0;

    /**
     * Returns the full path to the output archive. 
     */
    virtual std::filesystem::path getOutputArchivePath(const ParserContext& ctx);

    virtual std::string getDescription() {
        return "No special notes were specified about this format";
    }
};

}
