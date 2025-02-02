//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "otpch.h"

#include "fileloader.h"
#include <cmath>

NodeStruct::NodeStruct()
{
	start = 0;
	propsSize = 0;
	next = 0;
	child = 0;
	type = 0;
}

void NodeStruct::clearNet(NodeStruct* root)
{
	if (root)
	{
		clearChild(root);
	}
}

void NodeStruct::clearNext(NodeStruct* node)
{
	NodeStruct* deleteNode = node;
	NodeStruct* nextNode;

	while (deleteNode)
	{
		if (deleteNode->child)
		{
			clearChild(deleteNode->child);
		}

		nextNode = deleteNode->next;
		delete deleteNode;
		deleteNode = nextNode;
	}
}

void NodeStruct::clearChild(NodeStruct* node)
{
	if (node->child)
	{
		clearChild(node->child);
	}

	if (node->next)
	{
		clearNext(node->next);
	}

	delete node;
}

FileLoader::FileLoader()
{
	m_file = NULL;
	m_root = NULL;
	m_buffer = new unsigned char[1024];
	m_buffer_size = 1024;
	m_lastError = ERROR_NONE;
	//cache
	m_use_cache = false;
	m_cache_size = 0;
	m_cache_index = NO_VALID_CACHE;
	m_cache_offset = NO_VALID_CACHE;
	memset(m_cached_data, 0, sizeof(m_cached_data));
}


FileLoader::~FileLoader()
{
	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}

	NodeStruct::clearNet(m_root);
	delete[] m_buffer;

	for (int i = 0; i < CACHE_BLOCKS; ++i)
	{
		if (m_cached_data[i].data)
		{
			delete[] m_cached_data[i].data;
		}
	}
}

bool FileLoader::openFile(const char* filename, bool write, bool caching /*= false*/)
{
	if (write)
	{
		m_file = fopen(filename, "wb");

		if (m_file)
		{
			uint32_t version = 0;
			writeData(&version, sizeof(version), false);
			return true;
		}
		else
		{
			m_lastError = ERROR_CAN_NOT_CREATE;
			return false;
		}
	}
	else
	{
		m_file = fopen(filename, "rb");

		if (m_file)
		{
			uint32_t version;

			if (fread(&version, sizeof(version), 1, m_file) && version > 0)
			{
				fclose(m_file);
				m_file = NULL;
				m_lastError = ERROR_INVALID_FILE_VERSION;
				return false;
			}
			else
			{
				if (caching)
				{
					m_use_cache = true;
					fseek(m_file, 0, SEEK_END);
					int file_size = ftell(m_file);
					m_cache_size = std::min(32768, std::max(file_size / 20, 8192)) & ~0x1FFF;
				}

				//parse nodes
				if (safeSeek(4))
				{
					delete m_root;
					m_root = new NodeStruct();
					m_root->start = 4;
					int byte;

					if (safeSeek(4) && readByte(byte) && byte == NODE_START)
					{
						bool ret = parseNode(m_root);
						return ret;
					}
					else
					{
						return false;
					}
				}
				else
				{
					m_lastError = ERROR_INVALID_FORMAT;
					return false;
				}
			}
		}
		else
		{
			m_lastError = ERROR_CAN_NOT_OPEN;
			return false;
		}
	}
}

bool FileLoader::parseNode(NODE node)
{
	int byte;
	long pos;
	NODE currentNode = node;

	while (1)
	{
		//read node type
		if (readByte(byte))
		{
			currentNode->type = byte;
			bool setPropsSize = false;

			while (1)
			{
				//search child and next node
				if (readByte(byte))
				{
					if (byte == NODE_START)
					{
						//child node start
						if (safeTell(pos))
						{
							NODE childNode = new NodeStruct();
							childNode->start = pos;
							setPropsSize = true;
							currentNode->propsSize = pos - currentNode->start - 2;
							currentNode->child = childNode;

							if (!parseNode(childNode))
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					}
					else if (byte == NODE_END)
					{
						//current node end
						if (!setPropsSize)
						{
							if (safeTell(pos))
							{
								currentNode->propsSize = pos - currentNode->start - 2;
							}
							else
							{
								return false;
							}
						}

						if (readByte(byte))
						{
							if (byte == NODE_START)
							{
								//starts next node
								if (safeTell(pos))
								{
									NODE nextNode = new NodeStruct();
									nextNode->start = pos;
									currentNode->next = nextNode;
									currentNode = nextNode;
									break;
								}
								else
								{
									return false;
								}
							}
							else if (byte == NODE_END)
							{
								//up 1 level and move 1 position back
								if (safeTell(pos) && safeSeek(pos))
								{
									return true;
								}
								else
								{
									return false;
								}
							}
							else
							{
								//wrong format
								m_lastError = ERROR_INVALID_FORMAT;
								return false;
							}
						}
						else
						{
							//end of file?
							return true;
						}
					}
					else if (byte == ESCAPE_CHAR)
					{
						if (!readByte(byte))
						{
							return false;
						}
					}
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}
}

const unsigned char* FileLoader::getProps(const NODE node, unsigned long& size)
{
	if (node)
	{
		while (node->propsSize >= m_buffer_size)
		{
			delete[] m_buffer;

			while (node->propsSize >= m_buffer_size)
			{
				m_buffer_size *= 2;
			}

			m_buffer = new unsigned char[m_buffer_size];
		}

		//get buffer
		if (readBytes(m_buffer, node->propsSize, node->start + 2))
		{
			//unscape buffer
			unsigned int j = 0;
			bool escaped = false;

			for (unsigned int i = 0; i < node->propsSize; ++i, ++j)
			{
				if (m_buffer[i] == ESCAPE_CHAR)
				{
					//escape char found, skip it and write next
					++i;
					m_buffer[j] = m_buffer[i];
					//is neede a displacement for next bytes
					escaped = true;
				}
				else if (escaped)
				{
					//perform that displacement
					m_buffer[j] = m_buffer[i];
				}
				else
				{
					//the buffer is right as is
				}
			}

			size = j;
			return m_buffer;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


bool FileLoader::getProps(const NODE node, PropStream& props)
{
	unsigned long size;
	const unsigned char* a = getProps(node, size);

	if (!a)
	{
		props.init(NULL, 0);
		return false;
	}
	else
	{
		props.init((char*)a, size);
		return true;
	}
}

int FileLoader::setProps(void* data, unsigned short size)
{
	//data
	if (!writeData(data, size, true))
	{
		return getError();
	}

	return ERROR_NONE;
}

void FileLoader::startNode(unsigned char type)
{
	unsigned char nodeBegin = NODE_START;
	writeData(&nodeBegin, sizeof(nodeBegin), false);
	writeData(&type, sizeof(type), true);
}

void FileLoader::endNode()
{
	unsigned char nodeEnd = NODE_END;
	writeData(&nodeEnd, sizeof(nodeEnd), false);
}

NODE FileLoader::getChildNode(const NODE parent, unsigned long& type)
{
	if (parent)
	{
		NODE child = parent->child;

		if (child)
		{
			type = child->type;
		}

		return child;
	}
	else
	{
		type = m_root->type;
		return m_root;
	}
}

NODE FileLoader::getNextNode(const NODE prev, unsigned long& type)
{
	if (prev)
	{
		NODE next = prev->next;

		if (next)
		{
			type = next->type;
		}

		return next;
	}
	else
	{
		return NO_NODE;
	}
}


inline bool FileLoader::readByte(int& value)
{
	if (m_use_cache)
	{
		if (m_cache_index == NO_VALID_CACHE)
		{
			m_lastError = ERROR_CACHE_ERROR;
			return false;
		}

		if (m_cache_offset >= m_cached_data[m_cache_index].size)
		{
			long pos = m_cache_offset + m_cached_data[m_cache_index].base;
			long tmp = getCacheBlock(pos);

			if (tmp < 0)
			{
				return false;
			}

			m_cache_index = tmp;
			m_cache_offset = pos - m_cached_data[m_cache_index].base;

			if (m_cache_offset >= m_cached_data[m_cache_index].size)
			{
				return false;
			}
		}

		value = m_cached_data[m_cache_index].data[m_cache_offset];
		m_cache_offset++;
		return true;
	}
	else
	{
		value = fgetc(m_file);

		if (value == EOF)
		{
			m_lastError = ERROR_EOF;
			return false;
		}
		else
		{
			return true;
		}
	}
}

inline bool FileLoader::readBytes(unsigned char* buffer, unsigned int size, long pos)
{
	if (m_use_cache)
	{
		//seek at pos
		unsigned long reading, remain = size, bufferPos = 0;

		do
		{
			//prepare cache
			unsigned long i = getCacheBlock(pos);

			if (i == NO_VALID_CACHE)
			{
				return false;
			}

			m_cache_index = i;
			m_cache_offset = pos - m_cached_data[i].base;
			//get maximun read block size and calculate remaining bytes
			reading = std::min(remain, m_cached_data[i].size - m_cache_offset);
			remain = remain - reading;
			//read it
			memcpy(buffer + bufferPos, m_cached_data[m_cache_index].data + m_cache_offset, reading);
			//update variables
			m_cache_offset = m_cache_offset + reading;
			bufferPos = bufferPos + reading;
			pos = pos + reading;
		}
		while (remain > 0);

		return true;
	}
	else
	{
		if (fseek(m_file, pos, SEEK_SET))
		{
			m_lastError = ERROR_SEEK_ERROR;
			return false;
		}

		size_t value = fread(buffer, 1, size, m_file);

		if (value != size)
		{
			m_lastError = ERROR_EOF;
			return false;
		}
		else
		{
			return true;
		}
	}
}


/*
inline bool FileLoader::writeData(void* data, int size, bool unescape)
{
	for(int i = 0; i < size; ++i) {
		unsigned char c = *(((unsigned char*)data) + i);

		if(unescape && (c == NODE_START || c == NODE_END || c == ESCAPE_CHAR)) {
			unsigned char escape = ESCAPE_CHAR;
			int value = fwrite(&escape, 1, 1, m_file);
			if(value != 1) {
				m_lastError = ERROR_COULDNOTWRITE;
				return false;
			}
		}

		int value = fwrite(&c, 1, 1, m_file);
		if(value != 1) {
			m_lastError = ERROR_COULDNOTWRITE;
			return false;
		}
	}

	return true;
}
*/
inline bool FileLoader::checks(const NODE node)
{
	if (!m_file)
	{
		m_lastError = ERROR_NOT_OPEN;
		return false;
	}

	if (!node)
	{
		m_lastError = ERROR_INVALID_NODE;
		return false;
	}

	return true;
}

inline bool FileLoader::safeSeek(unsigned long pos)
{
	if (m_use_cache)
	{
		unsigned long i = getCacheBlock(pos);

		if (i == NO_VALID_CACHE)
		{
			return false;
		}

		m_cache_index = i;
		m_cache_offset = pos - m_cached_data[i].base;
	}
	else
	{
		if (fseek(m_file, pos, SEEK_SET))
		{
			m_lastError = ERROR_SEEK_ERROR;
			return false;
		}
	}

	return true;
}



inline bool FileLoader::safeTell(long& pos)
{
	if (m_use_cache)
	{
		if (m_cache_index == NO_VALID_CACHE)
		{
			m_lastError = ERROR_CACHE_ERROR;
			return false;
		}

		pos = m_cached_data[m_cache_index].base + m_cache_offset - 1;
		return true;
	}
	else
	{
		pos = ftell(m_file);

		if (pos == -1)
		{
			m_lastError = ERROR_TELL_ERROR;
			return false;
		}
		else
		{
			pos = pos - 1;
			return true;
		}
	}
}

bool FileLoader::writeData(const void* data, int size, bool unescape)
{
	for (int i = 0; i < size; ++i)
	{
		unsigned char c = *(((unsigned char*)data) + i);

		if (unescape && (c == NODE_START || c == NODE_END || c == ESCAPE_CHAR))
		{
			unsigned char escape = ESCAPE_CHAR;
			size_t value = fwrite(&escape, 1, 1, m_file);

			if (value != 1)
			{
				m_lastError = ERROR_COULDNOTWRITE;
				return false;
			}
		}

		size_t value = fwrite(&c, 1, 1, m_file);

		if (value != 1)
		{
			m_lastError = ERROR_COULDNOTWRITE;
			return false;
		}
	}

	return true;
}

inline unsigned long FileLoader::getCacheBlock(unsigned long pos)
{
	bool found = false;
	unsigned long i;
	unsigned long base_pos = pos & ~(m_cache_size - 1);

	for (i = 0; i < CACHE_BLOCKS; ++i)
	{
		if (m_cached_data[i].loaded)
		{
			if (m_cached_data[i].base == base_pos)
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		i = loadCacheBlock(pos);
	}

	return i;
}

long FileLoader::loadCacheBlock(unsigned long pos)
{
	long i;
	long loading_cache = -1;
	long base_pos = pos & ~(m_cache_size - 1);

	for (i = 0; i < CACHE_BLOCKS; ++i)
	{
		if (!m_cached_data[i].loaded)
		{
			loading_cache = i;
			break;
		}
	}

	if (loading_cache == -1)
	{
		for (i = 0; i < CACHE_BLOCKS; ++i)
		{
			if ((long)(labs((long)m_cached_data[i].base - base_pos)) > (long)(2 * m_cache_size))
			{
				loading_cache = i;
				break;
			}
		}

		if (loading_cache == -1)
		{
			loading_cache = 0;
		}
	}

	if (!m_cached_data[loading_cache].data)
	{
		m_cached_data[loading_cache].data = new unsigned char[m_cache_size];
	}

	m_cached_data[loading_cache].base = base_pos;

	if (fseek(m_file, m_cached_data[loading_cache].base, SEEK_SET))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return -1;
	}

	size_t size = fread(m_cached_data[loading_cache].data, 1, m_cache_size, m_file);
	m_cached_data[loading_cache].size = size;

	if (size < (pos - m_cached_data[loading_cache].base))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return -1;
	}

	m_cached_data[loading_cache].loaded = 1;
	return loading_cache;
}

PropStream::PropStream()
{
	end = NULL;
	p = NULL;
}

void PropStream::init(const char* a, unsigned long size)
{
	p = a;
	end = a + size;
}

int64_t PropStream::size() const
{
	return end - p;
}

#ifndef __SWAP_ENDIAN__
bool PropStream::GET_UINT32(uint32_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_INT32(int32_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_UINT16(uint16_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_INT16(int16_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_FLOAT(float& ret)
{
	return GET_VALUE(ret);
}

#else

bool PropStream::GET_UINT32(uint32_t& ret)
{
	bool b = GET_VALUE(ret);
	swap_uint32(ret);
	return b;
}

bool PropStream::GET_INT32(int32_t& ret)
{
	bool b = GET_VALUE(ret);
	swap_int32(ret);
	return b;
}

bool PropStream::GET_UINT16(uint16_t& ret)
{
	bool b = GET_VALUE(ret);
	swap_uint16(ret);
	return b;
}

bool PropStream::GET_INT16(int16_t& ret)
{
	bool b = GET_VALUE(ret);
	swap_int16(ret);
	return b;
}

bool PropStream::GET_FLOAT(float& ret)
{
	bool b = GET_VALUE(ret);
	swap_float(ret);
	return b;
}

#endif

bool PropStream::GET_UINT8(uint8_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_INT8(int8_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_CHAR(int8_t& ret)
{
	return GET_VALUE(ret);
}

bool PropStream::GET_STRING(std::string& ret)
{
	char* str;
	uint16_t str_len;

	if (!GET_VALUE(str_len))
	{
		return false;
	}

	if (size() < (int32_t)str_len)
	{
		return false;
	}

	str = new char[str_len + 1];
	memcpy(str, p, str_len);
	str[str_len] = 0;
	ret.assign(str, str_len);
	delete[] str;
	p = p + str_len;
	return true;
}

bool PropStream::GET_LSTRING(std::string& ret
                            )
{
	char* str;
	uint32_t str_len;

	if (!GET_VALUE(str_len))
	{
		return false;
	}

	if (size() < (int32_t)str_len)
	{
		return false;
	}

	str = new char[str_len + 1];
	memcpy(str, p, str_len);
	str[str_len] = 0;
	ret.assign(str, str_len);
	delete[] str;
	p = p + str_len;
	return true;
}

bool PropStream::GET_NSTRING(std::string& ret, unsigned short str_len)
{
	char* str;

	if (size() < (int32_t)str_len)
	{
		return false;
	}

	str = new char[str_len + 1];
	memcpy(str, p, str_len);
	str[str_len] = 0;
	ret.assign(str, str_len); // String can contain 0s
	delete[] str;
	p = p + str_len;
	return true;
}

bool PropStream::GET_RAWSTRING(char* buffer, unsigned short str_len)
{
	if (size() < (int32_t)str_len)
	{
		return false;
	}

	memcpy(&buffer[0], p, str_len);
	p = p + str_len;
	return true;
}

bool PropStream::SKIP_N(int32_t n)
{
	if (size() < n)
	{
		return false;
	}

	p = p + n;
	return true;
}

PropWriteStream::PropWriteStream()
{
	buffer = (char*)malloc(32 * sizeof(char));
	buffer_size = 32;
	size = 0;
	memset(buffer, 0, 32 * sizeof(char));
}

PropWriteStream::~PropWriteStream()
{
	free(buffer);
}

const char* PropWriteStream::getStream(uint32_t& _size) const
{
	_size = size;
	return buffer;
}

#ifndef __SWAP_ENDIAN__

void PropWriteStream::ADD_UINT32(uint32_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_INT32(int32_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_UINT16(uint16_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_INT16(int16_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_FLOAT(float ret)
{
	ADD_VALUE(ret);
}

#else

void PropWriteStream::ADD_UINT32(uint32_t ret)
{
	ADD_VALUE(swap_uint32(ret));
}

void PropWriteStream::ADD_INT32(int32_t ret)
{
	ADD_VALUE(swap_int32(ret));
}

void PropWriteStream::ADD_UINT16(uint16_t ret)
{
	ADD_VALUE(swap_uint16(ret));
}

void PropWriteStream::ADD_INT16(int16_t ret)
{
	ADD_VALUE(swap_int16(ret));
}

void PropWriteStream::ADD_FLOAT(float ret)
{
	ADD_VALUE(swap_float(ret));
}

#endif

void PropWriteStream::ADD_UINT8(uint8_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_INT8(int8_t ret)
{
	ADD_VALUE(ret);
}

void PropWriteStream::ADD_STRING(const std::string& add)
{
	uint16_t str_len = (uint16_t)add.size();
	ADD_VALUE(str_len);

	if ((buffer_size - size) < str_len)
	{
		buffer_size = buffer_size + str_len + 0x1F;
		buffer = (char*)realloc(buffer, buffer_size);
	}

	memcpy(&buffer[size], add.c_str(), str_len);
	size = size + str_len;
}

void PropWriteStream::ADD_LSTRING(const std::string& add)
{
	uint32_t str_len = (uint32_t)add.size();
	ADD_VALUE(str_len);

	if ((buffer_size - size) < str_len)
	{
		buffer_size = buffer_size + str_len + 0x1F;
		buffer = (char*)realloc(buffer, buffer_size);
	}

	memcpy(&buffer[size], add.c_str(), str_len);
	size = size + str_len;
}
