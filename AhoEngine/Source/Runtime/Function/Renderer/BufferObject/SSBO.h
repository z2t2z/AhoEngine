#pragma once
#include <glad/glad.h>

namespace Aho {

    // SSBO: shader storage buffer object in opengl

    class SSBOBase {
    public:
        virtual ~SSBOBase() = default;
        uint32_t GetBufferID() const {
            return m_BufferID;
        }
        uint32_t GetBindingPoint() const {
            return m_BindingPoint;
        }
    protected:
        uint32_t m_BufferID;
        uint32_t m_BindingPoint;
    };

    template <typename T>
    class SSBO : public SSBOBase {
    public:
        SSBO(const SSBO&) = delete;
        SSBO& operator=(const SSBO&) = delete;

        SSBO(uint32_t bindingPoint, size_t maxElements, bool staticDraw = false) {
            m_BindingPoint = bindingPoint;
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
    };


    template<typename T>
    class SSBOScalar : public SSBOBase {
    public:
        ~SSBOScalar() {
            if (m_MappedPtr) {
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
                m_MappedPtr = nullptr;
            }
            if (m_BufferID) {
                glDeleteBuffers(1, &m_BufferID);
            }
        }

        SSBOScalar(uint32_t bindingPoint) {
            m_BindingPoint = bindingPoint;

            glGenBuffers(1, &m_BufferID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);

            // Persistent mapped storage, CPU-readable
            glBufferStorage(
                GL_SHADER_STORAGE_BUFFER,
                sizeof(T),
                nullptr,
                GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_DYNAMIC_STORAGE_BIT
            );

            m_MappedPtr = reinterpret_cast<T*>(glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER,
                0,
                sizeof(T),
                GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
            ));

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_BufferID);
        }

        void Set(const T& value) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferID);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T), &value);
        }

        T Get() const {
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // 确保SSBO写入完成
            glFinish(); // 强制CPU等待GPU完成（仅调试用）
            return *m_MappedPtr;
        }

    private:
        T* m_MappedPtr = nullptr;
    };

}