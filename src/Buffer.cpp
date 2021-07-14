#include "Buffer.h"

#include<cstring>
#include <sys/socket.h>
#include<unistd.h>


constexpr char Buffer::kCRLF[];

ssize_t Buffer::ReadFd(int fd,int &save_errno)
{
	ssize_t read_sum = 0;
	while (true) {
		size_t writeable = WriteableBytes();
		ssize_t n = read(fd, begin() + write_index_, writeable);
		if (n < 0) {
			if (errno == EAGAIN) {
				return read_sum;
			}
			else {
				save_errno = errno;
				return -1;
			}	
		}
		else if (n > 0) {
			write_index_ += n;
			read_sum += n;
			//没有读满，就是读完了
			if (static_cast<size_t>(n) < writeable) {
				return read_sum;
			}
			//可能还没读完
			else {
				//buffer_扩容
				resize();
				continue;
			}
		}
		else {
			//对端关闭连接
			return 0;
		}
	}
}
