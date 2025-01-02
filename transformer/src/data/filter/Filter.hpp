#pragma once

#include "CLI/CLI.hpp"
#include <string>
#include <pugixml.hpp>

namespace sedd {

class Filter {
protected:
    std::string flag;
    std::string description;

    /**
     * Whether or not this filter is required. 
     * This variable should only be set to true for filters that are actually required for objective reasons. All other
     * filters should set this to false
     */
    bool required = false;
    bool enabled;
public:
    Filter(const std::string& flag, const std::string& description);
    /**
     * Utility constructor for required filters that are on by default.
     *
     * Must be invoked with required = true
     */
    Filter(bool required);

    virtual ~Filter() = default;

    /**
     * 
     *
     * \returns whether or not to discard the line. Line is discarded if `true` is returned
     */
    virtual bool process(pugi::xml_node& row) = 0;

    /**
     * Invoked when setting up flags for the CLI app
     */
    virtual void init(CLI::App& app);
    virtual bool isEnabled() { return required || enabled; }

    virtual std::string getSummary() = 0;
};

}
