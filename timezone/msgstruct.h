#pragma once

enum E_NET_CONFIG
{
    E_NET_PACK_SIZE   = 512,
    E_NET_BUFFER_SIZE = 4096,
};


/*
    a lnetpack include 
    lpackheader + net_content
*/

struct LPack{
	char buf[E_NET_PACK_SIZE + 1];
};

struct LPackHeader
{
    int size;
    int version;

	int GetSize() const
	{
		return size;
	}

	const char* GetMPack() const
	{
		return (const char*)(this + 1);
	}

	int GetMPackSize() const
	{
		return size - sizeof(*this);
	}
};
