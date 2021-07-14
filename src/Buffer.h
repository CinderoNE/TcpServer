#pragma once
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include<string>
#include<algorithm>
/*



0  ----------------  read_index_ ----------------  write_index_ ---------------- buffer_.size()
	                              ReadableBytes                  WriteableBytes
*/


class Buffer
{
public:

	static constexpr size_t kInitialSize = 4096;

	Buffer():
		buffer_(kInitialSize,0),
		read_index_(0),
		write_index_(0)
	{
	}

	std::string RetrieveAllAsStr() {
		std::string s(ReadBegin(),ReadableBytes());
		ClearAll();
		return s;
	}

	void Retrieve(size_t n) {
		read_index_ += n;
	}

	void RetrieveUntil(const char* end) {
		Retrieve(end - ReadBegin());
	}

	const char* ReadBegin()const  {
		return begin() + read_index_;
	}

	const char* WriteBegin()const {
		return begin() + write_index_;
	}

	char* WriteBegin() {
		return begin() + write_index_;
	}

	size_t ReadableBytes() {
		return write_index_ - read_index_;
	}

	size_t WriteableBytes() {
		return buffer_.size() - write_index_;
	}

	size_t CanReusedBytes() {
		return read_index_;
	}
	
	void Append(const char* data, size_t len) {
		EnsureWritableBytes(len);
		std::copy(data, data + len, WriteBegin());
		write_index_ += len;
	}

	void EnsureWritableBytes(size_t len)
	{
		if (WriteableBytes() < len)
		{
			MakeSpace(len);
		}
		
	}

	void ShrinkToInit() {
		buffer_.resize(kInitialSize);
	}

	void ClearAll() {
		read_index_ = 0;
		write_index_ = 0;
	}

	ssize_t ReadFd(int fd, int& save_errno);

	// 查找回车换行
	const char* FindCRLF() const
	{
		const char* crlf = std::search(ReadBegin(), WriteBegin(), kCRLF, kCRLF + 2);
		return crlf == WriteBegin() ? nullptr : crlf;
	}

private:

	char* begin() {
		return &*buffer_.begin();
	}

	const char* begin()  const {
		return &*buffer_.begin();
	}

	void resize() {
		size_t sz = buffer_.size();
		size_t new_sz = sz + (sz >> 1); // 1.5倍
		buffer_.resize(new_sz);
	}

	void resize(size_t len) {
		buffer_.resize(len);
	}

	void MakeSpace(size_t len) {
		if (CanReusedBytes() + WriteableBytes() < len) {
			resize(write_index_ + len);
		}
		else {
			//移动未读的数据到最前面
			size_t readable = ReadableBytes();
			std::copy(begin() + read_index_,
				begin() + write_index_,
				begin());
			read_index_ = 0;
			write_index_ = read_index_ + readable;
		}
	}

private:
	std::string buffer_;
	size_t read_index_;
	size_t write_index_;

	static constexpr char kCRLF[] = "\r\n";
};

#endif // !_BUFFER_H_




