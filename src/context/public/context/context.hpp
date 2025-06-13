#pragma once

namespace Prism::Context {
    struct Context {
        Context() = default;
        ~Context() = default;

        Context(Context&& other) = delete;
        Context& operator=(Context&&) = delete;
        
        Context(Context& other) = delete;
        Context& operator=(Context&) = delete;

        public:
            void RunEngine();

        private:
    };
};