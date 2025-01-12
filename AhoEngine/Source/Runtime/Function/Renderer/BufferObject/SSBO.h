#pragma once
#include <glad/glad.h>

namespace Aho {

    // SSBO: shader storage buffer object in opengl

    class SSBOBase {
    public:
        virtual ~SSBOBase() = default;
    };


    template <typename T>
    class SSBO : public SSBOBase {
    public:
        SSBO(const SSBO&) = delete;
        SSBO& operator=(const SSBO&) = delete;

        SSBO(uint32_t bindingPoint, size_t maxElements, bool staticDraw = false)
                : m_BindingPoint(bindingPoint), m_BufferID(0) {
            glGenBuffers(1, &m_BufferID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);
            glBufferData(GL_SHADER_STORAGE_BUFFER, maxElements * sizeof(T), nullptr, staticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_BufferID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        ~SSBO() {
            if (m_BufferID) {
                glDeleteBuffers(1, &m_BufferID);
            }
        }

        void SetData(const std::vector<T>& data) {
            size_t dataSize = data.size() * sizeof(T);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);
            glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, data.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_BufferID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        void SetSubData(const std::vector<T>& data, size_t offset) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), data.size() * sizeof(T), data.data());
            //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                AHO_CORE_ERROR("GL Error {}", error);
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }

        void GetSubData(std::vector<T>& data, size_t offset) {

            glFlush();
            glFinish();
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);

            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), data.size() * sizeof(T), data.data());
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                AHO_CORE_ERROR("GL Error {}", error);
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        }

        uint32_t GetBindingPoint() const { return m_BindingPoint; }

    private:
        uint32_t m_BufferID;
        uint32_t m_BindingPoint;
    };
}