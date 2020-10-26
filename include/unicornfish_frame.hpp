#ifndef UNICORNFISH_FRAME_HPP
#define UNICORNFISH_FRAME_HPP

#include <unicornfish_base.hpp>

namespace Uf {
	class Frame {
		friend class Message;

	public:
		Frame();
		Frame(Frame& frameToMove) { Move(frameToMove); }
		// Frame(const Frame & frameToCopy) { Copy(frameToCopy); }
		Frame(const zframe_t* frameToCopy) { Copy(frameToCopy); }
		Frame(zframe_t** frameToMove) { Move(frameToMove); }
		~Frame() { Delete(); }

		operator bool() const { return frame != nullptr; }
		//const Frame &operator=(const Frame &frameToCopy);
		const Frame& operator=(Frame& frameToMove);

		void Move(zframe_t** frameToMove);
		void Move(Frame& frameToMove);
		void Copy(const zframe_t* frameToCopy);
		void Copy(const Frame& frameToCopy);
		zframe_t* CopyZFrame() const;
		void Delete();
		size_t SizeInBytes() const;
		bool strncmp(const char* str, size_t n) const;

		void ReplaceData(const std::string& data);
		const std::string& GetStrData() const;
		const std::string& GetHexData() const;
		const void* GetData() const;
		const char* c_str() const { return GetStrData().c_str(); }
		const zframe_t* GetZFrame() const { return frame; }
		zframe_t* MoveZFrame();

	private:
		zframe_t* frame = nullptr;
		mutable std::string data;
		mutable std::string hex;
	};
} // namespace Uf

#endif