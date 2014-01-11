/*
 * This file is part of the NMEA0183 library.
 * Copyright (c) 2013, Justin R Cutler <justin.r.cutler@gmail.com>
 * Modified by Jacob Baines <baines.jacob@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "NMEA0183.hpp"

#include <cctype>
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace
{
    char to_hex(uint8_t nibble)
    {
        nibble &= 0x0f;
        return nibble > 9 ? nibble - 10 + 'A' : nibble + '0';
    }

    /*!
     * Converts the stringified lat/long to double
     * \param[in] p_point the lat/long in string format
     * \param[in] p_multiplier N, S, W, or E
     * \return the lat or long in double format
     */
    double to_double(const std::string& p_point, const std::string& p_multiplier)
    {
        std::vector<std::string> entries;
        boost::split(entries, p_point, boost::is_any_of("."));

        if (entries.size() != 2)
        {
            return 0;
        }

        std::string minutes;
        while (minutes.size() != 2 && entries[0].size() > 1)
        {
            minutes.insert(minutes.begin(), entries[0][entries[0].size() - 1]);
            entries[0].pop_back();
        }

        double upper = boost::lexical_cast<double>(entries[0]);
        double minutes_double = boost::lexical_cast<double>(minutes);
        double decimals = boost::lexical_cast<double>(entries[1]);
        int dividor = 10;
        for (std::size_t i = 1; i < entries[1].size(); ++i)
        {
            dividor *= 10;
        }
        decimals = decimals / dividor;
        minutes_double += decimals;
        minutes_double /= 60;

        if (p_multiplier == "S" || p_multiplier == "W")
        {
            return (upper + minutes_double) * -1;
        }

        return upper + minutes_double;
    }
}

NMEA0183::NMEA0183() :
    m_state(NMEA0183_INVALID),
    m_update_checksum(false),
    m_sentence(),
    m_index(0),
    m_fields(0),
    m_checksum(0)
{
}

NMEA0183::~NMEA0183()
{
}

void NMEA0183::reset()
{
    m_state = NMEA0183_INVALID;
    m_update_checksum = false;
    m_index = 0;
    m_fields = 0;
    m_checksum = 0;
}

bool NMEA0183::update(char c)
{
    if (m_state == NMEA0183_ACCEPT)
    {
        // discard previously accepted sentence
        reset();
    }

    if (!(c & 0x80) && (c >= 0x20))
    {
        switch (c)
        {
            case '!':
            case '$':
                // new sentence
                m_index = 0;
                m_fields = 0;
                m_checksum = 0;
                m_state = NMEA0183_ADDRESS;
                break;
            case '*':
                if (m_state == NMEA0183_FIELD_DATA ||
                    m_state == NMEA0183_ADDRESS)
                {
                    m_update_checksum = false;
                    m_state = NMEA0183_CHECKSUM_HI;
                }
                else
                {
                    reset();
                }
                break;
            case ',':
                if (m_state == NMEA0183_FIELD_DATA ||
                    m_state == NMEA0183_ADDRESS)
                {
                    m_state = NMEA0183_FIELD_DATA;
                    ++m_fields;
                }
                else
                {
                    reset();
                }
                break;
            case '\\':
            case '~':
            case 0x7f:
                // reserved for future use
                reset();
                break;
            default:
                switch (m_state)
                {
                    case NMEA0183_ADDRESS:
                        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                        {
                            m_update_checksum = true;
                        }
                        else
                        {
                            reset();
                        }
                        break;
                    case NMEA0183_FIELD_DATA_HEX_HI:
                        if (std::isxdigit(c) != 0)
                        {
                            m_state = NMEA0183_FIELD_DATA_HEX_LO;
                        }
                        else
                        {
                            reset();
                        }
                        break;
                    case NMEA0183_FIELD_DATA_HEX_LO:
                        if (std::isxdigit(c) != 0)
                        {
                           m_state = NMEA0183_FIELD_DATA;
                        }
                        else
                        {
                            reset();
                        }
                        break;
                    case NMEA0183_CHECKSUM_HI:
                        if (c == to_hex(m_checksum >> 4))
                        {
                            // checksum high nibble matches
                            m_state = NMEA0183_CHECKSUM_LO;
                        }
                        else
                        {
                            reset();
                        }
                        break;
                    case NMEA0183_CHECKSUM_LO:
                        if (c == to_hex(m_checksum & 0x0f))
                        {
                            // checksum valid
                            m_state = NMEA0183_EOS;
                        }
                        else
                        {
                            reset();
                        }
                        break;
                    case NMEA0183_INVALID:
                        // do nothing
                        break;
                    default:
                        break;
                }
                break;
        }
    }
    else if (m_state == NMEA0183_EOS && (c == 0x0a || c == 0x0d))
    {
        // NULL terminate
        c = 0;
        m_state = NMEA0183_ACCEPT;
    }
    else
    {
        // invalid
        reset();
    }

    if (m_state != NMEA0183_INVALID)
    {
        m_sentence[m_index] = c;
        ++m_index;
        if (m_update_checksum)
        {
            m_checksum ^= c;
        }
        if ((m_index == sizeof(m_sentence) - 1) && m_state != NMEA0183_ACCEPT)
        {
            reset();
        }
    }

    return m_state == NMEA0183_ACCEPT;
}

bool NMEA0183::get_gprmc_lat_long(const char* p_nmea_sentence, double& p_lat, double& p_long) const
{
    std::vector<std::string> entries;
    boost::split(entries, p_nmea_sentence, boost::is_any_of(","));
    if (entries.empty())
    {
        return false;
    }

    if (entries[0].compare("$GPRMC") != 0 || entries.size() != 13)
    {
        // only care about gprmc...
        return false;
    }

    if (entries[2].compare("A") != 0)
    {
        return false;
    }

    p_lat = to_double(entries[3], entries[4]);
    p_long = to_double(entries[5], entries[6]);
    return true;
}

NMEA0183::ParseState NMEA0183::getState() const
{
    return m_state;
}

const char* NMEA0183::getSentence() const
{
    return m_state == NMEA0183_ACCEPT ? m_sentence : NULL;
}

boost::uint8_t NMEA0183::getFields() const
{
    return m_fields;
}
