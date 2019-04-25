/*  =========================================================================
    secw_snmpv1 - Document parsers for snmpv1 document

    Copyright (C) 2019 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef SECW_SNMPV1_H_INCLUDED
#define SECW_SNMPV1_H_INCLUDED

#define SNMPV1_TYPE "Snmpv1"

namespace secw
{
    /**
     * \brief snmpv1 implementation
     */
    class Snmpv1;
    
    using Snmpv1Ptr   = std::shared_ptr<Snmpv1>;

    /**
     * Some key definition for serialization
     * 
     */
    static constexpr const char* DOC_SNMPV1_COMMUNITY_NAME = "secw_snmpv1_community_name";

    class Snmpv1  final : public Document
    {
    public:

        Snmpv1();

        Snmpv1( const std::string & name,
                const std::string & communityName = "",
                const Id & id = "");

        DocumentPtr clone() const override;

        void validate() const override {} //TODO fill this function

        //Public elements
        const std::string & getCommunityName() const { return m_communityName; }

        //Private elements
        //no private

        /**
         * \brief try to cast a document to a Snmpv1 shared ptr
         * 
         * \return shared ptr on snmpv1 or null shared ptr in case of error
         */
        static Snmpv1Ptr tryToCast(DocumentPtr doc);

    private:
        //Public elements
        std::string m_communityName = "";

        //Private elements
        //no private

        void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
        void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

        void UpdatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
        void UpdatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    };

} // namepsace secw

#endif
