#include <dmusic/Forms.h>
#include <dmusic/Tracks.h>
#include <dmusic/Exceptions.h>

using namespace DirectMusic;
using namespace DirectMusic::Riff;

TrackForm::TrackForm(const Chunk& c)
    : m_flags(nullptr) {
    if (c.getId() != "RIFF" || c.getListId() != "DMTK")
        throw DirectMusic::InvalidChunkException("RIFF DMTK", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "guid") {
            m_guid = GUID(subchunk.getData().data());
        } else if (id == "vers") {
            m_version = DMUS_IO_VERSION(subchunk.getData().data());
        } else if (id == "trkh") {
            m_header = DMUS_IO_TRACK_HEADER(subchunk.getData().data());
        } else if (id == "trkx") {
            m_flags = std::make_shared<DMUS_IO_TRACK_EXTRAS_HEADER>(subchunk.getData().data());
        } else if (id == "LIST") {
            const std::string& listid = subchunk.getListId();
            if (listid == "UNFO") {
                m_unfo = Unfo(subchunk);
            } else if (listid == "cord") {
                m_data = ChordTrack(subchunk);
            } else if (listid == "pftr") {
                m_data = ChordmapTrack(subchunk);
            } else if (listid == "lyrt") {
                m_data = LyricsTrack(subchunk);
            } else if (listid == "MARK") {
                m_data = MarkerTrack(subchunk);
            } else if (listid == "prmt") {
                m_data = ParameterControlTrack(subchunk);
            } else if (listid == "scrt") {
                m_data = ScriptTrack(subchunk);
            } else if (listid == "segt") {
                m_data = SegmentTriggerTrack(subchunk);
            } else if (listid == "sttr") {
                m_data = StyleTrack(subchunk);
            } else if (listid == "TIMS") {
                m_data = TimeSignatureTrack(subchunk);
            } else if (listid == "wavt") {
                m_data = WaveTrack(subchunk);
            }
        } else if (id == "RIFF") {
            const std::string& listid = subchunk.getListId();
            if (listid == "DMBT") {
                m_data = BandTrack(subchunk);
            } else if (listid == "DMPT") {
                m_data = PatternTrack(subchunk);
            }
        } else if (id == "cmnd") {
            m_data = CommandTrack(subchunk);
        } else if (id == "mute") {
            m_data = MuteTrack(subchunk);
        } else if (id == "sgnp") {
            m_data = SignpostTrack(subchunk);
        } else if (id == "syex") {
            m_data = SysexTrack(subchunk);
        } else if (id == "tetr") {
            m_data = TempoTrack(subchunk);
        }
    }
}

static BandItem parseBandItem(const Chunk& c) {
    DMUS_IO_BAND_ITEM_HEADER2 header;
    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "bdih") {
            DMUS_IO_BAND_ITEM_HEADER h(subchunk.getData().data());
            header.lBandTimeLogical = h.lBandTime;
            header.lBandTimePhysical = h.lBandTime;
        } else if (id == "bd2h") {
            header = DMUS_IO_BAND_ITEM_HEADER2(subchunk.getData().data());
        } else if (id == "RIFF" && subchunk.getListId() == "DMBD") {
            return std::make_tuple(header, BandForm(subchunk));
        }
    }

    throw "Invalid band item";
}

BandTrack::BandTrack(const Chunk& c) {
    if (c.getId() != "RIFF" || c.getListId() != "DMBT")
        throw DirectMusic::InvalidChunkException("RIFF DMBT", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "guid") {
            m_guid = GUID(subchunk.getData().data());
        } else if (id == "vers") {
            m_version = DMUS_IO_VERSION(subchunk.getData().data());
        } else if (id == "bdth") {
            m_header = DMUS_IO_BAND_TRACK_HEADER(subchunk.getData().data());
        } else if (id == "LIST") {
            const std::string& listid = subchunk.getListId();
            if (listid == "UNFO") {
                m_unfo = Unfo(subchunk);
            } else if (listid == "lbdl") {
                for (Chunk band : subchunk.getSubchunks()) {
                    if (band.getId() == "LIST" && band.getListId() == "lbnd") {
                        m_bands.push_back(parseBandItem(band));
                    }
                }
            }
        }
    }
}

ChordTrack::ChordTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "cord")
        throw DirectMusic::InvalidChunkException("LIST cord", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "crdh") {
            m_header = littleEndianRead<std::uint32_t>(subchunk.getData().data());
        } else if (id == "crdb") {
            const std::uint8_t *data = c.getData().data();
            std::uint16_t chordSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            m_chord = DMUS_IO_CHORD(data);
            data += chordSize;
            std::uint16_t subchordNum = littleEndianRead<std::uint16_t>(data);
            data += 2;
            std::uint16_t subchordSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            for (int i = 0; i < subchordNum; i++) {
                m_subchords.push_back(DMUS_IO_SUBCHORD(data));
            }
        }
    }
}

static Chordmap parseChordmap(const Chunk& c) {
    std::uint16_t stamp;
    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "stmp") {
            stamp = littleEndianRead<std::uint16_t>(subchunk.getData().data());
        } else if (id == "LIST" && subchunk.getListId() == "DMRF") {
            return std::make_tuple(stamp, ReferenceList(subchunk));
        }
    }

    throw "Invalid chordmap";
}

ChordmapTrack::ChordmapTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "pftr")
        throw DirectMusic::InvalidChunkException("LIST pftr", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        if (subchunk.getId() == "LIST" && subchunk.getListId() == "pfrf") {
            m_chordmaps.push_back(parseChordmap(subchunk));
        }
    }
}

CommandTrack::CommandTrack(const Chunk& c) {
    if (c.getId() != "cmnd")
        throw DirectMusic::InvalidChunkException("cmnd", c.getId() + " " + c.getListId());
    const std::uint8_t *data = c.getData().data();
    int offset = 0;
    std::uint16_t cmdSize = littleEndianRead<std::uint16_t>(data + offset);
    offset += 2;
    while (offset < c.getData().size()) {
        m_commands.push_back(DMUS_IO_COMMAND(data + offset));
        offset += cmdSize;
    }
}

static LyricsEvent readLyricsEvent(const Chunk& c) {
    DMUS_IO_LYRICSTRACK_EVENTHEADER header;
    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "stmp") {
            header = DMUS_IO_LYRICSTRACK_EVENTHEADER(subchunk.getData().data());
        } else if (id == "lyrn") {
            return std::make_tuple(header, std::wstring((const wchar_t *)subchunk.getData().data()));
        }
    }

    throw "Invalid lyrics event";
}

LyricsTrack::LyricsTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "lyrt")
        throw DirectMusic::InvalidChunkException("LIST lyrt", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        if (subchunk.getId() == "LIST" && subchunk.getListId() == "lyrt") {
            m_lyrics.push_back(readLyricsEvent(subchunk));
        }
    }
}

MarkerTrack::MarkerTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "MARK")
        throw DirectMusic::InvalidChunkException("LIST MARK", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        const std::uint8_t *data = subchunk.getData().data();
        const std::uint8_t *start = data;
        if (id == "vals") {
            std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            while (data - start < subchunk.getData().size()) {
                m_validStarts.push_back(DMUS_IO_VALID_START(data));
                data += structSize;
            }
        } else if (id == "play") {
            std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            while (data - start < subchunk.getData().size()) {
                m_validPlays.push_back(DMUS_IO_PLAY_MARKER(data));
                data += structSize;
            }
        }
    }
}

MuteTrack::MuteTrack(const Chunk& c) {
    if (c.getId() != "mute")
        throw DirectMusic::InvalidChunkException("mute", c.getId() + " " + c.getListId());

    const std::uint8_t *data = c.getData().data();
    const std::uint8_t *start = data;
    std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
    data += 2;
    while (data - start < c.getData().size()) {
        m_mutes.push_back(DMUS_IO_MUTE(data));
        data += structSize;
    }
}

PatternTrack::PatternTrack(const Chunk& c)
    : m_pattern(nullptr)
{
    if(c.getId() != "RIFF" || c.getListId() != "DMPT")
        throw DirectMusic::InvalidChunkException("RIFF DMPT", c.getId() + " " + c.getListId());
    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "styh") {
            const std::uint8_t *data = subchunk.getData().data();
            m_style = DMUS_IO_STYLE(data);
        } else if (id == "LIST") {
            if (subchunk.getListId() == "pttn") {
                m_pattern = std::make_shared<Pattern>(subchunk);
            }
        }
    }
}

SequenceTrack::SequenceTrack(const Chunk& c) {
    if (c.getId() != "seqt")
        throw DirectMusic::InvalidChunkException("seqt", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        const std::uint8_t *data = subchunk.getData().data();
        const std::uint8_t *start = data;
        if (id == "evtl") {
            std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            while (data - start < subchunk.getData().size()) {
                m_seqItems.push_back(DMUS_IO_SEQ_ITEM(data));
                data += structSize;
            }
        } else if (id == "curl") {
            std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            while (data - start < subchunk.getData().size()) {
                m_curveItems.push_back(DMUS_IO_CURVE_ITEM(data));
                data += structSize;
            }
        }
    }
}

SignpostTrack::SignpostTrack(const Chunk& c) {
    if (c.getId() != "sgnp")
        throw DirectMusic::InvalidChunkException("sgnp", c.getId() + " " + c.getListId());
    const std::uint8_t *data = c.getData().data();
    const std::uint8_t *start = data;
    std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
    data += 2;
    while (data - start < c.getData().size()) {
        m_signposts.push_back(DMUS_IO_SIGNPOST(data));
        data += structSize;
    }
}

static StyleReference readStyleReference(const Chunk& c) {
    std::uint16_t stmp;
    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "stmp") {
            stmp = littleEndianRead<std::uint16_t>(subchunk.getData().data());
        } else if (id == "LIST" && subchunk.getListId() == "DMRF") {
            return std::make_tuple(stmp, ReferenceList(subchunk));
        }
    }

    throw "Invalid style reference";
}

StyleTrack::StyleTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "sttr")
        throw DirectMusic::InvalidChunkException("LIST sttr", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "LIST" && subchunk.getListId() == "strf") {
            m_styles.push_back(readStyleReference(subchunk));
        }
    }
}

TempoTrack::TempoTrack(const Chunk& c) {
    if (c.getId() != "tetr")
        throw DirectMusic::InvalidChunkException("tetr", c.getId() + " " + c.getListId());
    const std::uint8_t *data = c.getData().data();
    const std::uint8_t *start = data;
    std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
    data += 2;
    while (data - start < c.getData().size()) {
        m_items.push_back(DMUS_IO_TEMPO_ITEM(data));
        data += structSize;
    }
}

TimeSignatureTrack::TimeSignatureTrack(const Chunk& c) {
    if (c.getId() != "LIST" || c.getListId() != "TIMS")
        throw DirectMusic::InvalidChunkException("LIST TIMS", c.getId() + " " + c.getListId());

    for (const Chunk& subchunk : c.getSubchunks()) {
        const std::string& id = subchunk.getId();
        if (id == "tims") {
            const std::uint8_t *data = subchunk.getData().data();
            const std::uint8_t *start = data;
            std::uint16_t structSize = littleEndianRead<std::uint16_t>(data);
            data += 2;
            while (data - start < subchunk.getData().size()) {
                m_items.push_back(DMUS_IO_TIMESIGNATURE_ITEM(data));
                data += structSize;
            }
        }
    }
}