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
#ifndef NMEA0183_HPP
#define NMEA0183_HPP

#include <boost/cstdint.hpp>

/*!
 * This class builds up NMEA 0183 sentences that we can later extract GPS
 * information from. Data is fed in one char at a time via update().
 */
class NMEA0183
{
public:

    //! The states of the NMEA sentence building
    enum ParseState
    {
        NMEA0183_INVALID = 0,
        NMEA0183_ADDRESS,
        NMEA0183_FIELD_DATA,
        NMEA0183_FIELD_DATA_HEX_HI,
        NMEA0183_FIELD_DATA_HEX_LO,
        NMEA0183_CHECKSUM_HI,
        NMEA0183_CHECKSUM_LO,
        NMEA0183_EOS,
        NMEA0183_ACCEPT
    };

public:

    NMEA0183();
    ~NMEA0183();

    //! Resets the member variables to their default states
    void reset();

    /*!
     * Adds another character to the, in progress, NMEA sentence. Adding
     * characters causes m_state to transition through the ParseStates until
     * the entire sentence is successfully built (NMEA0183_ACCEPT) or failure
     * occurs (NMEA0183_INVALILD)
     *
     * \return true if we have transitioned to NMEA_ACCEPT
     */
    bool update(char c);

    //! return the sentence if we are in the NMEA0183_ACCEPT state
    const char* getSentence() const;

    //! return the number of fields in the current sentence
    boost::uint8_t getFields() const;

    //! \return the current state of the parser
    ParseState getState() const;

    /*!
     * Extract the latitude and longitude from the GPRMC string and convert
     * them to double format.
     *
     * \param[in] p_nmea_sentence a nmea sentence to extract data from
     * \param[in,out] p_lat the extracted latitude
     * \param[in,out] p_long the extracted longitue
     * \return true if and only if lat/long were extracted
     */
    bool get_gprmc_lat_long(const char* p_nmea_sentence, double& p_lat, double& p_long) const;

private:

    //! disable evil things
    NMEA0183(const NMEA0183& p_rhs);
    NMEA0183& operator=(const NMEA0183& p_rhs);

private:

    //! The current state that parsing via update() is in
    ParseState m_state;

    //! Checksum update flag
    bool m_update_checksum;

    /*!
     * NMEA0183 sentence buffer.
     * The maximum number of characters in a sentence shall be 82, consisting
     * of a maximum of 79 characters between the starting delimiter "$" or "!"
     * and the terminating <CR><LF>.
     */
    char m_sentence[82 + 1 /* NUL terminator */];

    //! Current index within @p sentence
    boost::uint8_t m_index;

    //! Number of fields in current sentence
    boost::uint8_t m_fields;

    //! Sentence checksum
    boost::uint8_t m_checksum;
};

#endif // NMEA0183_HPP
