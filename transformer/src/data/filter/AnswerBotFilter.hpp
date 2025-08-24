#pragma once

#include "data/filter/Filter.hpp"

namespace sedd {

class AnswerBotFilter : public Filter {
private:
    static constexpr int AIBOT_UID = -2;
public:
    AnswerBotFilter() : Filter("--no-aibot", "Whether or not to strip AI bot answers from the data dump") {}

    bool process(DataDumpFileType_t, pugi::xml_node& row) override;

    std::string getSummary() override;
};

}
