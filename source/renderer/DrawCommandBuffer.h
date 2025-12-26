
#pragma once
#include <vector>
#include "DrawCommand.h"
namespace Core {

    class DrawCommandBuffer {
    public:
        void Clear() { _commands.clear(); }
        void reserve(std::size_t n) { _commands.reserve(n); }
        void Push(const DrawCommand& cmd) { _commands.push_back(cmd); }
        const std::vector<DrawCommand>& Data() const { return _commands; }
        std::vector<DrawCommand>& Data() { return _commands; } // if a renderer needs to patch
        std::size_t Size() const { return _commands.size(); }
        bool Empty() const { return _commands.empty(); }

    private:
        std::vector<DrawCommand> _commands;
    };
}
