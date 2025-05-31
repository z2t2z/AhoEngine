#pragma once

#include <glad/glad.h>

namespace Aho {
	// UBO: uniform buffer object in opengl
	class UBOBase {
	public:
		virtual ~UBOBase() = default;
	};

    template <typename T>
    class UBO : public UBOBase {
    public:
        UBO(uint32_t bindingPoint) : m_BindingPoint(bindingPoint) {
            glGenBuffers(1, &m_BufferID);
            glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_BufferID);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        ~UBO() {
            glDeleteBuffers(1, &m_BufferID);
        }

        void SetData(const T& data, size_t offset) {
            glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
            glBufferSubData(GL_UNIFORM_BUFFER, offset * sizeof(T), sizeof(T), &data);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

		//void GetSubData(T& data, size_t offset) {
		//	glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
		//	glGetBufferSubData(GL_UNIFORM_BUFFER, offset * sizeof(T), data.size() * sizeof(T), data.data());

		//}

        uint32_t GetBindingPoint() const { return m_BindingPoint; }

    private:
        uint32_t m_BufferID;
        uint32_t m_BindingPoint;
    };

}
