#pragma once

#include <memory>

class IPaticleMoifierFactory
{
public:
    virtual ~IPaticleMoifierFactory() = default;

    virtual std::shared_ptr<IPaticleMoifierFactory> CreateModifier() = 0;



};