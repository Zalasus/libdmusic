#include <dmusic/Riff.h>
#include <dmusic/Exceptions.h>
#include <dmusic/Common.h>
#include <string>

using namespace DirectMusic::Riff;

struct ChunkHeader {
    char id[4];
    std::uint32_t size;

    ChunkHeader(const std::uint8_t *data) {
        READFOURCC(ChunkHeader, id);
        FIELDINIT(ChunkHeader, size, std::uint32_t);
    }
};

Chunk::Chunk(const std::uint8_t* buffer) {
    ChunkHeader header(buffer);
    m_id = std::string(header.id, 4);
    buffer += sizeof(ChunkHeader);
    m_data = std::vector<std::uint8_t>(buffer, buffer + header.size);

    // A RIFF chunk only contains subchunks if its ID is "RIFF" or "LIST"
    if (m_id == "RIFF" || m_id == "LIST") {
        m_listId = std::string((const char *)buffer, 4);
        buffer += 4;
        std::uint32_t count = 0;
        if (m_listId == "INFO") {
            buffer = buffer;
        }
        // len includes the listId FOURCC, we must exclude it in this calculation
        while (count < header.size - 4) {
            Chunk subchunk(buffer);
            m_subchunks.push_back(subchunk);
            int size = subchunk.m_data.size();
            if (size % 2 == 1) size++;
            count += size + 8;
            buffer += size + 8;
        }
    }
}

Info::Info(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "INFO")
        throw DirectMusic::InvalidChunkException("LIST INFO", c.getId() + " " + c.getListId());
    for(Chunk subchunk : c.getSubchunks()) {
        std::vector<std::uint8_t> data = subchunk.getData();
        std::string id = subchunk.getId();
        std::string value = std::string((const char *)data.data());
        if (id == "IARL") m_iarl = value;
        if (id == "IART") m_iart = value;
        if (id == "ICMS") m_icms = value;
        if (id == "ICMT") m_icmt = value;
        if (id == "ICOP") m_icop = value;
        if (id == "ICRD") m_icrd = value;
        if (id == "IENG") m_ieng = value;
        if (id == "IGNR") m_ignr = value;
        if (id == "IKEY") m_ikey = value;
        if (id == "IMED") m_imed = value;
        if (id == "INAM") m_inam = value;
        if (id == "IPRD") m_iprd = value;
        if (id == "ISBJ") m_isbj = value;
        if (id == "ISFT") m_isft = value;
        if (id == "ISRC") m_isrc = value;
        if (id == "ISRF") m_isrf = value;
        if (id == "ITCH") m_itch = value;
    }
}

Unfo::Unfo(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "UNFO")
        throw DirectMusic::InvalidChunkException("LIST UNFO", c.getId() + " " + c.getListId());
    for (Chunk subchunk : c.getSubchunks()) {
        std::vector<std::uint8_t> data = subchunk.getData();
        std::string id = subchunk.getId();
        std::wstring value = std::wstring((const wchar_t *)data.data());
        if (id == "IARL") m_iarl = value;
        if (id == "IART") m_iart = value;
        if (id == "ICMS") m_icms = value;
        if (id == "ICMT") m_icmt = value;
        if (id == "ICOP") m_icop = value;
        if (id == "ICRD") m_icrd = value;
        if (id == "IENG") m_ieng = value;
        if (id == "IGNR") m_ignr = value;
        if (id == "IKEY") m_ikey = value;
        if (id == "IMED") m_imed = value;
        if (id == "INAM") m_inam = value;
        if (id == "IPRD") m_iprd = value;
        if (id == "ISBJ") m_isbj = value;
        if (id == "ISFT") m_isft = value;
        if (id == "ISRC") m_isrc = value;
        if (id == "ISRF") m_isrf = value;
        if (id == "ITCH") m_itch = value;
    }
}
