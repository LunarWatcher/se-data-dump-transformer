#include "Filter.hpp"
#include <stdexcept>

namespace sedd {

Filter::Filter(const std::string& flag, const std::string& description) : flag(flag), description(description) {

}

Filter::Filter(bool required) : required(true) {
    if (required) {
        throw std::runtime_error("Using this constructor requires a `true` value; false was supplied");
    }
}

void Filter::init(CLI::App& app) {
    // If required, no flag is allowed. Should only be reserved for filters that are required for the data dump to be
    // processed sanely.
    if (required) { return; }

    app.add_flag(
        flag,
        enabled,
        description
    )
        ->required(false)
        ->default_val(false);
}

}
