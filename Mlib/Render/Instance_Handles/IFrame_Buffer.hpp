#pragma once

namespace Mlib {

class IFrameBuffer {
public:
	virtual ~IFrameBuffer() = default;
	virtual bool is_configured() const = 0;
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

}
