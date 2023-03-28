#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int bufferSize)
	: m_capacity(bufferSize * 2)
	, m_bufferSize(bufferSize)
	, m_readPos(0)
	, m_writePos(0)
	, m_buffer()
{
	m_buffer.resize(m_capacity);
}

bool RecvBuffer::OnRead(int numOfBytes)
{
	if (numOfBytes > GetDataSize())
		return false;

	m_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int numOfBytes)
{
	if (numOfBytes > GetFreeSize())
		return false;

	m_writePos += numOfBytes;
	return true;
}

void RecvBuffer::AdjustPos()
{
	int dataSize = GetDataSize();
	if (m_readPos == m_writePos)
	{
		m_readPos = 0;
		m_writePos = 0;
	}
	else
	{
		if (GetFreeSize() < (m_bufferSize / 2))
		{
			::memcpy(&m_buffer[0], &m_buffer[m_readPos], dataSize);
			m_readPos = 0;
			m_writePos = dataSize;
		}
	}
}
